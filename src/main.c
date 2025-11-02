#include "raylib.h"
#include "projectile.h"
#include "enemy.h"
#include "game.h"
#include "starfield.h"

void InitGame(int* score, int* lives);

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "raylib - Tailgunner");

    GameState gameState = STATE_START;
    Camera camera = { 0 };
    camera.position = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, -1.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    int score = 0;
    int lives = 0;

    InitAudioDevice();

    // NOTE: Make sure you have shoot.wav and explosion.wav in the resources directory
    Sound shootSound = LoadSound("resources/shoot.wav");
    Sound explosionSound = LoadSound("resources/explosion.wav");

    while (!WindowShouldClose())
    {
        switch (gameState)
        {
            case STATE_START:
            {
                if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                {
                    InitGame(&score, &lives);
                    gameState = STATE_PLAYING;
                    HideCursor();
                }
            } break;
            case STATE_PLAYING:
            {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                {
                    Ray ray = GetMouseRay(GetMousePosition(), camera);
                    ShootProjectile(ray);
                    PlaySound(shootSound);
                }

                UpdateProjectiles();
                UpdateStarfield();
                score += UpdateEnemies(explosionSound, &lives);

                if (lives <= 0)
                {
                    gameState = STATE_GAME_OVER;
                    ShowCursor();
                }

            } break;
            case STATE_GAME_OVER:
            {
                if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                {
                    gameState = STATE_START;
                }
            } break;
        }

        BeginDrawing();
        ClearBackground(BLACK);

        if (gameState == STATE_START)
        {
            DrawText("TAILGUNNER", GetScreenWidth() / 2 - MeasureText("TAILGUNNER", 40) / 2, GetScreenHeight() / 2 - 40, 40, LIGHTGRAY);
            DrawText("Press ENTER or CLICK to Start", GetScreenWidth() / 2 - MeasureText("Press ENTER or CLICK to Start", 20) / 2, GetScreenHeight() / 2 + 20, 20, GRAY);
        }
        else if (gameState == STATE_PLAYING)
        {
            BeginMode3D(camera);
            DrawStarfield();
            DrawProjectiles();
            DrawEnemies();
            EndMode3D();

            DrawCircleLines(GetMouseX(), GetMouseY(), 10, MAROON);
            DrawLine(GetMouseX() - 20, GetMouseY(), GetMouseX() + 20, GetMouseY(), RED);
            DrawLine(GetMouseX(), GetMouseY() - 20, GetMouseX(), GetMouseY() + 20, RED);

            DrawText(TextFormat("Score: %i", score), 10, 10, 20, GREEN);
            DrawText(TextFormat("Lives: %i", lives), GetScreenWidth() - 100, 10, 20, RED);
        }
        else if (gameState == STATE_GAME_OVER)
        {
            DrawText("GAME OVER", GetScreenWidth() / 2 - MeasureText("GAME OVER", 40) / 2, GetScreenHeight() / 2 - 40, 40, RED);
            DrawText(TextFormat("Final Score: %i", score), GetScreenWidth() / 2 - MeasureText(TextFormat("Final Score: %i", score), 20) / 2, GetScreenHeight() / 2 + 20, 20, GREEN);
            DrawText("Press ENTER or CLICK to Return to Start", GetScreenWidth() / 2 - MeasureText("Press ENTER or CLICK to Return to Start", 20) / 2, GetScreenHeight() / 2 + 60, 20, GRAY);
        }

        EndDrawing();
    }

    UnloadSound(shootSound);
    UnloadSound(explosionSound);
    CloseAudioDevice();

    CloseWindow();

    return 0;
}

void InitGame(int* score, int* lives)
{
    *score = 0;
    *lives = 3;
    InitProjectiles();
    InitEnemies();
    InitStarfield();
}