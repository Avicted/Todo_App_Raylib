#include "includes.h"

// Rendering -------------------------------------------------------------------
Camera2D ScreenSpaceCamera = {0};
const unsigned int SCREEN_WIDTH = 854;
const unsigned int SCREEN_HEIGHT = 480;
int ScreenWidth = 854;
int ScreenHeight = 480;

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
    TODO_STATE TODO_STATE_NOT_STARTED;
} TodoItem;

// -----------------------------------------------------------------------------

static void
HandleWindowResize(void)
{
    // Handle F11 fullscreen
    if (IsKeyPressed(KEY_F11))
    {
        // Set the window size to max screen size
        if (!IsWindowFullscreen())
        {
            SetWindowSize(GetMonitorWidth(0), GetMonitorHeight(0));
            printf("GetMonitorWidth(0): %i\n", GetMonitorWidth(0));
            printf("GetMonitorHeight(0): %i\n", GetMonitorHeight(0));
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
Update(float DeltaTime)
{
    HandleWindowResize();

    if (IsKeyPressed(KEY_ESCAPE))
    {
        isRunning = false;
    }
}

static void
Render(float DeltaTime)
{
    BeginDrawing();
    BeginTextureMode(MainRenderer->MainRenderTexture);

    ClearBackground(MAGENTA);

    DrawRectangleGradientV(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){250, 38, 100, 255}, (Color){255, 112, 255, 255});

    BeginMode2D(ScreenSpaceCamera);

    DrawTextEx(MainFont, "Hello, Sailor!", (Vector2){(SCREEN_WIDTH / 2) - (MeasureText("Hello, Sailor!", 20) / 2), 100}, 20, 0, BLACK);
    DrawTextEx(MainFont, "Hello, Sailor!", (Vector2){(SCREEN_WIDTH / 2) - (MeasureText("Hello, Sailor!", 20) / 2), 99}, 20, 0, WHITE);

    // Draw a button Center Screen
    Rectangle ButtonRec = (Rectangle){
        (SCREEN_WIDTH / 2) - (200 / 2),
        (SCREEN_HEIGHT / 2) - (50 / 2),
        200,
        50};

    if (GuiButton(ButtonRec, "Click Me!"))
    {
        printf("Button Clicked!\n");
        ButtonCount++;
    }

    DrawTextEx(MainFont, TextFormat("Button Clicked: %i", ButtonCount), (Vector2){(SCREEN_WIDTH / 2) - (MeasureText("Button Clicked: 00", 20) / 2), 281}, 20, 0, BLACK);

    DrawTextEx(MainFont, TextFormat("Button Clicked: %i", ButtonCount), (Vector2){(SCREEN_WIDTH / 2) - (MeasureText("Button Clicked: 00", 20) / 2), 280}, 20, 0, WHITE);

    EndMode2D();
    EndTextureMode();

    // Draw the render texture in the window
    BeginDrawing();
    ClearBackground(BLACK);
    BeginMode2D(ScreenSpaceCamera);

    SetTextureFilter(MainRenderer->MainRenderTexture.texture, TEXTURE_FILTER_POINT);

    DrawTexturePro(MainRenderer->MainRenderTexture.texture,
                   MainRenderer->MainRenderTextureSourceRec,
                   MainRenderer->MainRenderTextureDestRec,
                   MainRenderer->MainRenderTextureOrigin,
                   0.0f,
                   WHITE);

    EndMode2D();

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
    CloseWindow();

    return 0;
}
