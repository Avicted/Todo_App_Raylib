#include "includes.h"

bool isRunning = true;

// Rendering
Camera2D ScreenSpaceCamera = {0};
const unsigned int SCREEN_WIDTH = 800;
const unsigned int SCREEN_HEIGHT = 450;
int ScreenWidth = 800;
int ScreenHeight = 450;

Font MainFont;

Renderer *MainRenderer;
Vector2 virtualMouse = {0};

// Game Specific Functions and Variables
unsigned int ButtonCount = 0;

static void
HandleWindowResize(void)
{
    // Get the current window size
    ScreenWidth = GetScreenWidth();
    ScreenHeight = GetScreenHeight();

    // Calculate aspect ratios
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

    // Update the mouse position
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
    BeginMode2D(ScreenSpaceCamera);

    DrawText("Hello, Sailor!", (SCREEN_WIDTH / 2) - (MeasureText("Hello, Sailor!", 20) / 2), 100, 20, BLACK);
    DrawText("Hello, Sailor!", (SCREEN_WIDTH / 2) - (MeasureText("Hello, Sailor!", 20) / 2), 99, 20, WHITE);

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

    DrawText(TextFormat("Button Clicked: %i", ButtonCount), (SCREEN_WIDTH / 2) - (MeasureText("Button Clicked: 00", 20) / 2), 279, 20, BLACK);

    DrawText(TextFormat("Button Clicked: %i", ButtonCount), (SCREEN_WIDTH / 2) - (MeasureText("Button Clicked: 00", 20) / 2), 280, 20, WHITE);

    EndMode2D();
    EndTextureMode();

    // Draw the render texture in the window
    BeginDrawing();
    ClearBackground(BLACK);
    BeginMode2D(ScreenSpaceCamera);

    DrawTexturePro(MainRenderer->MainRenderTexture.texture,
                   MainRenderer->MainRenderTextureSourceRec,
                   MainRenderer->MainRenderTextureDestRec,
                   MainRenderer->MainRenderTextureOrigin,
                   0.0f,
                   WHITE);

    EndMode2D();

    const int FPS = GetFPS();
    DrawText(TextFormat("FPS: %02i", FPS), 10, 10, 20, BLACK);
    DrawText(TextFormat("FPS: %02i", FPS), 9, 9, 20, WHITE);

    // Shadow text
    DrawText(TextFormat("Virtual Mouse: %.0f, %.0f", virtualMouse.x, virtualMouse.y), 11, 31, 20, BLACK);

    DrawText(TextFormat("Virtual Mouse: %.0f, %.0f", virtualMouse.x, virtualMouse.y), 10, 30, 20, WHITE);

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

int main(int argc, char **argv)
{
    HandleInputArgs(argc, argv);

    printf("Hello, Sailor!\n");

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Todo App");

    SetTargetFPS(60);

    ScreenSpaceCamera = (Camera2D){
        .offset = {0.0f, 0.0f},
        .target = {0.0f, 0.0f},
        .rotation = 0.0f,
        .zoom = 1.0f,
    };

    MainFont = LoadFont("resources/fonts/SuperMarioBros2.ttf");

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
