#include "includes.h"

// Rendering -------------------------------------------------------------------
Camera2D ScreenSpaceCamera = {0};
const float lerpFactor = 0.1f; // How quickly the camera follows the target
const unsigned int SCREEN_WIDTH = 854 * 1;
const unsigned int SCREEN_HEIGHT = 480 * 1;
int ScreenWidth = 854 * 1;
int ScreenHeight = 480 * 1;

Font MainFont;

Renderer *MainRenderer;
Vector2 virtualMouse = {0};

// Game Specific Functions and Variables ---------------------------------------
bool isRunning = true;

unsigned int ButtonCount = 0;

typedef enum TODO_STATE
{
    TODO_STATE_NOT_STARTED,
    TODO_STATE_IN_PROGRESS,
    TODO_STATE_COMPLETED,
} TODO_STATE;

typedef struct TodoItem
{
    char *Title;
    char *Description;
    TODO_STATE State;
} TodoItem;

TodoItem *TodoItems;
int TodoItemCount = 0;
int SelectedTodoItem = 0;
bool ShowTodoItemDetails = false;

float offsetY = 0.0f;           // Vertical offset for scrolling
const float scrollSpeed = 0.5f; // Speed of scrolling
const int TodoItemHeight = 64;
const int TodoItemWidth = 854 * 0.75;

// -----------------------------------------------------------------------------

static void
AddTodoItem(const char *title, const char *description, TODO_STATE state)
{
    // Increase the size of the array
    TodoItems = (TodoItem *)realloc(TodoItems, (TodoItemCount + 1) * sizeof(TodoItem));
    if (TodoItems == NULL)
    {
        // Handle memory allocation failure
        fprintf(stderr, "Failed to allocate memory for TodoItems\n");
        exit(EXIT_FAILURE);
    }

    // Initialize the new TodoItem
    TodoItems[TodoItemCount].Title = strdup(title);             // Duplicate string for title
    TodoItems[TodoItemCount].Description = strdup(description); // Duplicate string for description
    TodoItems[TodoItemCount].State = state;                     // Update the state

    TodoItemCount++; // Increment the item count
}

static void
HandleWindowResize(void)
{
    if (IsKeyPressed(KEY_F11))
    {
        if (!IsWindowFullscreen())
        {
            SetWindowSize(GetMonitorWidth(0), GetMonitorHeight(0));
        }

        ToggleFullscreen();
    }

    ScreenWidth = GetScreenWidth();
    ScreenHeight = GetScreenHeight();

    float aspectRatio = (float)ScreenWidth / (float)ScreenHeight;
    float targetAspectRatio = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;

    // Determine letterbox dimensions
    if (aspectRatio > targetAspectRatio)
    {
        // Window is wider than 16:9, letterbox on top and bottom
        int letterboxHeight = ScreenHeight;
        int letterboxWidth = (int)(ScreenHeight * targetAspectRatio);
        int offsetX = (ScreenWidth - letterboxWidth) / 2;
        MainRenderer->MainRenderTextureDestRec = (Rectangle){offsetX, 0, letterboxWidth, letterboxHeight};
    }
    else
    {
        // Window is taller than 16:9, letterbox on left and right
        int letterboxWidth = ScreenWidth;
        int letterboxHeight = (int)(ScreenWidth / targetAspectRatio);
        int offsetY = (ScreenHeight - letterboxHeight) / 2;
        MainRenderer->MainRenderTextureDestRec = (Rectangle){0, offsetY, letterboxWidth, letterboxHeight};
    }

    Vector2 mouse = GetMousePosition();
    float scaleX = (float)GetScreenWidth() / SCREEN_WIDTH;
    float scaleY = (float)GetScreenHeight() / SCREEN_HEIGHT;
    float scale = fminf(scaleX, scaleY);

    // Adjust for window offsets
    float offsetX = (GetScreenWidth() - (SCREEN_WIDTH * scale)) * 0.5f;
    float offsetY = (GetScreenHeight() - (SCREEN_HEIGHT * scale)) * 0.5f;

    // Calculate the virtual mouse position
    virtualMouse.x = (mouse.x - offsetX) / scale;
    virtualMouse.y = (mouse.y - offsetY) / scale;
    virtualMouse = Vector2Clamp(virtualMouse, (Vector2){0, 0}, (Vector2){(float)SCREEN_WIDTH, (float)SCREEN_HEIGHT});

    // Apply the same transformation as the virtual mouse to the real mouse (to work with raygui)
    SetMouseOffset(-offsetX, -offsetY);
    SetMouseScale(1 / scale, 1 / scale);
}

static void
CustomUpdateCamera(void)
{
    // Calculate the target position based on the selected item
    float targetY = SelectedTodoItem * TodoItemHeight;

    // Smoothly interpolate the ScreenSpaceCamera position towards the target position
    ScreenSpaceCamera.target.y += (targetY - ScreenSpaceCamera.target.y) * lerpFactor;

    // Keep the camera within the bounds of the content
    if (ScreenSpaceCamera.target.y < ScreenSpaceCamera.offset.y)
    {
        ScreenSpaceCamera.target.y = ScreenSpaceCamera.offset.y;
    }
    if (ScreenSpaceCamera.target.y > (TodoItemCount * TodoItemHeight) - (SCREEN_HEIGHT / 2))
    {
        ScreenSpaceCamera.target.y = (TodoItemCount * TodoItemHeight) - (SCREEN_HEIGHT / 2);
    }

    ScreenSpaceCamera.offset = (Vector2){0, GetScreenHeight() / 2}; // Set the camera offset
}

static void
Update(float DeltaTime)
{
    HandleWindowResize();
    CustomUpdateCamera();

    if (IsKeyPressed(KEY_ESCAPE))
    {
        isRunning = false;
    }

    float Wheel = GetMouseWheelMove();

    if (Wheel != 0)
    {
        Vector2 MouseWorldPos = GetScreenToWorld2D(GetMousePosition(), ScreenSpaceCamera);

        ScreenSpaceCamera.offset = GetMousePosition();

        ScreenSpaceCamera.target = MouseWorldPos;

        ScreenSpaceCamera.zoom += Wheel * 0.0125f;
        if (ScreenSpaceCamera.zoom < 0.125f)
        {
            ScreenSpaceCamera.zoom = 0.125f;
        }
    }

    // translate based on right click
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
    {
        Vector2 Delta = GetMouseDelta();
        ScreenSpaceCamera.target.x -= Delta.x * 0.5;
        ScreenSpaceCamera.target.y -= Delta.y * 0.5;
    }

    if (IsKeyPressed(KEY_UP))
    {
        if (SelectedTodoItem <= 0)
        {
            SelectedTodoItem = 0;
        }
        else
        {
            SelectedTodoItem--;
            offsetY -= scrollSpeed; // Scroll up
        }
    }
    if (IsKeyPressed(KEY_DOWN))
    {
        if (SelectedTodoItem >= TodoItemCount - 1)
        {
            SelectedTodoItem = TodoItemCount - 1;
        }
        else
        {
            SelectedTodoItem++;
            offsetY += scrollSpeed; // Scroll down
        }
    }

    // Clamp the offset to ensure you don't scroll too far
    if (offsetY < 0)
    {
        offsetY = 0;
    }
    if (offsetY > (TodoItemCount * TodoItemHeight) - SCREEN_HEIGHT)
    {
        offsetY = (TodoItemCount * TodoItemHeight) - SCREEN_HEIGHT;
    }

    if (IsKeyPressed(KEY_ENTER))
    {
        printf("Selected Todo Item: %i\n", SelectedTodoItem);
        ShowTodoItemDetails = !ShowTodoItemDetails;
    }
}

static void
Render(float DeltaTime)
{
    BeginDrawing();
    // BeginTextureMode(MainRenderer->MainRenderTexture);

    ClearBackground(MAGENTA);
    BeginMode2D(ScreenSpaceCamera);

    // Calculate total height of all Todo items
    const int totalHeight = (TodoItemCount * TodoItemHeight) + 256; // Adding some padding

    DrawRectangleGradientV(0, 0, GetScreenWidth(), GetScreenHeight() + 256,
                           (Color){250, 38, 100, 255}, (Color){255, 112, 255, 255});

    const int TodoItemXPos = (GetScreenWidth() / 2) - (TodoItemWidth / 2);

    // Draw the Todo items
    const int topMargin = 32;

    for (int i = 0; i < TodoItemCount; i++)
    {
        float itemYPos = i * TodoItemHeight - offsetY + topMargin;

        // Draw the rectangle for the Todo item
        Color itemColor = (i == SelectedTodoItem) ? WHITE : LIGHTGRAY;
        DrawRectangle(TodoItemXPos, itemYPos, TodoItemWidth, TodoItemHeight, itemColor);
        DrawRectangleLines(TodoItemXPos, itemYPos, TodoItemWidth, TodoItemHeight, BLACK);

        const int XLeftPadding = 10;
        DrawTextEx(MainFont, TodoItems[i].Title,
                   (Vector2){TodoItemXPos + XLeftPadding, itemYPos + 5}, 16, 0, BLACK);
        DrawTextEx(MainFont, TodoItems[i].Description,
                   (Vector2){TodoItemXPos + XLeftPadding, itemYPos + 30}, 12, 0, BLACK);
    }

    // Draw a Green Rectangle around the selected Todo Item
    if (SelectedTodoItem >= 0 && SelectedTodoItem < TodoItemCount)
    {
        float selectedYPos = SelectedTodoItem * TodoItemHeight - offsetY + topMargin;
        DrawRectangleLines(TodoItemXPos, selectedYPos, TodoItemWidth, TodoItemHeight, GREEN);
    }

    // Draw Todo Item Details modal if ShowTodoItemDetails is true
    if (ShowTodoItemDetails)
    {
        const int PosY = (GetScreenHeight() / 2) - (GetScreenHeight() / 10);
        const int Width = GetScreenWidth() / 3;
        const int Height = GetScreenHeight() / 5;
        Rectangle ModalRec = (Rectangle){
            (GetScreenWidth() / 2) - (Width / 2),
            PosY,
            Width,
            Height};

        GuiWindowBox(ModalRec, "Todo Item Details");

        // Set GuiLabel style
        GuiSetFont(MainFont);
        GuiSetStyle(LABEL, TEXT_SIZE, 16);
        GuiSetStyle(LABEL, TEXT_PADDING, 10);

        // Item Title
        GuiLabel((Rectangle){ModalRec.x + 10, ModalRec.y + 30, 0, 0}, "Item Title");
        GuiLabel((Rectangle){ModalRec.x + 10, ModalRec.y + 50, 0, 0}, TodoItems[SelectedTodoItem].Title);

        // Item Description
        GuiLabel((Rectangle){ModalRec.x + 10, ModalRec.y + 80, 0, 0}, "Item Description");
        GuiLabel((Rectangle){ModalRec.x + 10, ModalRec.y + 100, 0, 0}, TodoItems[SelectedTodoItem].Description);

        // Item State
        const char *StateText = "Not Started";
        if (TodoItems[SelectedTodoItem].State == TODO_STATE_IN_PROGRESS)
        {
            StateText = "In Progress";
        }
        else if (TodoItems[SelectedTodoItem].State == TODO_STATE_COMPLETED)
        {
            StateText = "Completed";
        }

        GuiLabel((Rectangle){ModalRec.x + 10, ModalRec.y + 130, 0, 0}, "Item State");
        GuiLabel((Rectangle){ModalRec.x + 10, ModalRec.y + 150, 0, 0}, StateText);
    }

    EndMode2D();
    // EndTextureMode();

    // Draw the render texture in the window
    /*BeginDrawing();
    ClearBackground(BLACK);
    BeginMode2D(ScreenSpaceCamera);

    SetTextureFilter(MainRenderer->MainRenderTexture.texture, TEXTURE_FILTER_POINT);

    // Calculate the source rectangle for drawing the render texture
    Rectangle sourceRec = {
        0,
        ScreenSpaceCamera.target.y,
        MainRenderer->MainRenderTexture.texture.width,
        -MainRenderer->MainRenderTexture.texture.height};

    DrawTexturePro(MainRenderer->MainRenderTexture.texture,
                   sourceRec,
                   MainRenderer->MainRenderTextureDestRec,
                   MainRenderer->MainRenderTextureOrigin,
                   0.0f,
                   WHITE);*/

    DrawRectangleLinesEx((Rectangle){MainRenderer->MainRenderTextureDestRec.x, MainRenderer->MainRenderTextureDestRec.y, MainRenderer->MainRenderTextureDestRec.width, MainRenderer->MainRenderTextureDestRec.height}, 4, BLUE);

    EndMode2D();

    // UI -----------------------------------------------------------------------
    const int FPS = GetFPS();
    DrawTextEx(MainFont, TextFormat("FPS: %02i", FPS), (Vector2){10, 10}, 20, 0, BLACK);
    DrawTextEx(MainFont, TextFormat("FPS: %02i", FPS), (Vector2){9, 9}, 20, 0, WHITE);

    // Virtual Mouse Position
    DrawTextEx(MainFont, TextFormat("Virtual Mouse: %i x %i", (int)virtualMouse.x, (int)virtualMouse.y), (Vector2){10, 30}, 20, 0, BLACK);
    DrawTextEx(MainFont, TextFormat("Virtual Mouse: %i x %i", (int)virtualMouse.x, (int)virtualMouse.y), (Vector2){9, 29}, 20, 0, WHITE);

    // Render texture position
    DrawTextEx(MainFont, TextFormat("RenderTexture: %i x %i", SCREEN_WIDTH, SCREEN_HEIGHT), (Vector2){10, 60}, 20, 0, BLACK);
    DrawTextEx(MainFont, TextFormat("RenderTexture: %i x %i", SCREEN_WIDTH, SCREEN_HEIGHT), (Vector2){9, 59}, 20, 0, WHITE);

    DrawTextEx(MainFont, TextFormat("Window: %i x %i", ScreenWidth, ScreenHeight), (Vector2){10, 90}, 20, 0, BLACK);
    DrawTextEx(MainFont, TextFormat("Window: %i x %i", ScreenWidth, ScreenHeight), (Vector2){9, 89}, 20, 0, WHITE);

    // SelectedTodoItem
    DrawTextEx(MainFont, TextFormat("Selected Todo Item: %i", SelectedTodoItem), (Vector2){10, 120}, 20, 0, BLACK);
    DrawTextEx(MainFont, TextFormat("Selected Todo Item: %i", SelectedTodoItem), (Vector2){9, 119}, 20, 0, WHITE);

    // ScreenSpaceCamera.offset.y
    DrawTextEx(MainFont, TextFormat("Camera Offset Y: %f", ScreenSpaceCamera.offset.y), (Vector2){10, 180}, 20, 0, BLACK);
    DrawTextEx(MainFont, TextFormat("Camera Offset Y: %f", ScreenSpaceCamera.offset.y), (Vector2){9, 179}, 20, 0, WHITE);

    // ScreenSpaceCamera.target.y
    DrawTextEx(MainFont, TextFormat("Camera Target Y: %f", ScreenSpaceCamera.target.y), (Vector2){10, 210}, 20, 0, BLACK);
    DrawTextEx(MainFont, TextFormat("Camera Target Y: %f", ScreenSpaceCamera.target.y), (Vector2){9, 209}, 20, 0, WHITE);

    // ShowTodoItemDetails
    DrawTextEx(MainFont, TextFormat("Show Todo Item Details: %s", ShowTodoItemDetails ? "true" : "false"), (Vector2){10, 150}, 20, 0, BLACK);
    DrawTextEx(MainFont, TextFormat("Show Todo Item Details: %s", ShowTodoItemDetails ? "true" : "false"), (Vector2){9, 149}, 20, 0, WHITE);

    EndDrawing();
}

static void
HandleInputArgs(int argc, char **argv)
{
    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
        {
            printf("Usage: %s [OPTION]\n", argv[0]);
            printf("Options:\n");
            printf("\t-h, --help\t\tDisplay this information\n");
            exit(0);
        }
    }
}

static void
InitGame(void)
{
    // Place the window in the center of the current screen
    SetWindowPosition((GetMonitorWidth(0) / 2) - (SCREEN_WIDTH / 2) + 800, GetMonitorHeight(0) / (2 - SCREEN_HEIGHT / 2) + 800);

    SetTargetFPS(60);

    ScreenSpaceCamera = (Camera2D){
        .offset = {0.0f, 0.0f},
        .target = {0.0f, 0.0f},
        .rotation = 0.0f,
        .zoom = 1.0f,
    };

    MainFont = LoadFontEx("resources/fonts/SuperMarioBros2.ttf", 1024, 0, 250);

    MainRenderer = (Renderer *)calloc(1, sizeof(Renderer));

    MainRenderer->MainRenderTexture = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
    MainRenderer->virtualRatio = (double)GetScreenWidth() / (double)SCREEN_WIDTH;
    MainRenderer->MainRenderTextureOrigin = (Vector2){0.0f, 0.0f};

    // The target's height is flipped (in the source Rectangle), due to OpenGL reasons
    MainRenderer->MainRenderTextureSourceRec = (Rectangle){
        0.0f,
        0.0f,
        (float)(MainRenderer->MainRenderTexture.texture.width),
        (float)(-MainRenderer->MainRenderTexture.texture.height),
    };

    MainRenderer->MainRenderTextureDestRec = (Rectangle){
        (float)(-MainRenderer->virtualRatio),
        (float)(MainRenderer->virtualRatio),
        (float)(GetScreenWidth() + (MainRenderer->virtualRatio)),
        (float)(GetScreenHeight() + (MainRenderer->virtualRatio)),
    };

    // Start with no items
    TodoItems = NULL;
    TodoItemCount = 0;

    // Add initial Todo Items
    AddTodoItem("Test Todo Item", "This is a test todo item.", TODO_STATE_NOT_STARTED);
    AddTodoItem("Test Todo Item 2", "This is a test todo item 2.", TODO_STATE_NOT_STARTED);
    AddTodoItem("Test Todo Item 3", "This is a test todo item 3.", TODO_STATE_IN_PROGRESS);
    AddTodoItem("Test Todo Item 4", "This is a test todo item 4.", TODO_STATE_COMPLETED);
    AddTodoItem("Test Todo Item 5", "This is a test todo item 5.", TODO_STATE_NOT_STARTED);
    AddTodoItem("Test Todo Item 6", "This is a test todo item 6.", TODO_STATE_IN_PROGRESS);
    AddTodoItem("Test Todo Item 7", "This is a test todo item 7.", TODO_STATE_COMPLETED);
    AddTodoItem("Test Todo Item 8", "This is a test todo item 8.", TODO_STATE_NOT_STARTED);
    AddTodoItem("Test Todo Item 9", "This is a test todo item 9.", TODO_STATE_IN_PROGRESS);
    AddTodoItem("Test Todo Item 10", "This is a test todo item 10. With a longer text", TODO_STATE_COMPLETED);
    AddTodoItem("Test Todo Item 11", "This is a test todo item 11.", TODO_STATE_NOT_STARTED);
    AddTodoItem("Test Todo Item 12", "This is a test todo item 12.", TODO_STATE_IN_PROGRESS);
    AddTodoItem("Test Todo Item 13", "This is a test todo item 13.", TODO_STATE_COMPLETED);
    AddTodoItem("Test Todo Item 14", "This is a test todo item 14.", TODO_STATE_NOT_STARTED);
    AddTodoItem("Test Todo Item 15", "This is a test todo item 15.", TODO_STATE_IN_PROGRESS);
    AddTodoItem("Test Todo Item 16", "This is a test todo item 16.", TODO_STATE_COMPLETED);
    AddTodoItem("Test Todo Item 17", "This is a test todo item 17.", TODO_STATE_NOT_STARTED);
    AddTodoItem("Test Todo Item 18 123 Longer Title", "This is a test todo item 18.", TODO_STATE_IN_PROGRESS);
}

int main(int argc, char **argv)
{
    HandleInputArgs(argc, argv);

    printf("Hello, Sailor!\n");

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Todo App");

    InitGame();

    // Main Game Loop
    while (!WindowShouldClose())
    {
        const float DeltaTime = GetFrameTime();
        Update(DeltaTime);
        Render(DeltaTime);
    }

    // Cleanup
    UnloadRenderTexture(MainRenderer->MainRenderTexture);

    free(MainRenderer);
    free(TodoItems);

    CloseWindow();

    return 0;
}
