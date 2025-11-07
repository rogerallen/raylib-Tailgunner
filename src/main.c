#include "raylib.h"
#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif
#include "enemy.h"
#include "game.h"
#include "starfield.h"
#include "laser.h"
#include "forcefield.h"
#include <stdio.h>

//----------------------------------------------------------------------------------
// main.c - Game entry and orchestration
//
// See module-level notes in repository README for high-level behavior. This file
// initializes subsystems, runs the main loop, and tears down resources on exit.
//
// Implementation notes:
// - Loads audio assets at startup and unloads at exit
// - Keeps main loop minimal: input -> update -> render
// - Delegates gameplay logic to subsystem modules
//----------------------------------------------------------------------------------

void InitGame(int* score, int* lives, int* wave, struct LaserManager* lmgr, struct EnemyManager* emgr, struct ForceFieldManager* ffmgr);

//----------------------------------------------------------------------------------
// main - Implementation Notes:
// - Initializes window, audio and resources
// - Handles simple state machine for START/PLAYING/GAME_OVER
// - Updates and renders subsystems each frame
//----------------------------------------------------------------------------------
int main(void)
{
    int screenWidth = 1600;
    int screenHeight = 900;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "raylib - Tailgunner");

    SetTargetFPS(60);

    GameState gameState = STATE_START;
    Camera camera = { 0 };
    camera.position = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, -1.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    int score = 0;
    int lives = 0;
    int wave = 0;

    // Encapsulated managers (avoid globals)
    LaserManager laserMgr = { 0 };
    EnemyManager enemyMgr = { 0 };
    ForceFieldManager ffMgr = { 0 };

    InitAudioDevice();

    Sound shootSound = LoadSound("resources/shoot.wav");
    Sound explosionSound = LoadSound("resources/explosion.wav");
    Sound forceFieldSound = LoadSound("resources/forcefield.wav");
    Sound forceFailSound = LoadSound("resources/forcefail.wav");
    Sound forceFieldHitSound = LoadSound("resources/bounce.wav");

    int frameCount = 0;
    int touch_count_last_frame = 0;
    double previousTime = GetTime();

    while (!WindowShouldClose())
    {
        switch (gameState)
        {
            case STATE_START:
            {
                if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                {
                    InitGame(&score, &lives, &wave, &laserMgr, &enemyMgr, &ffMgr);
                    gameState = STATE_PLAYING;
                    HideCursor();
                }
            } break;
            case STATE_PLAYING:
            {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                {
                    Ray ray = GetMouseRay(GetMousePosition(), camera);
                    int hits = FireLasers(&laserMgr, &enemyMgr, ray, camera, explosionSound);
                    score += hits;
                    if (hits > 0) PlaySound(explosionSound);
                    PlaySound(shootSound);
                }

                if (IsKeyPressed(KEY_SPACE))
                {
                    bool activated = ActivateForceField(&ffMgr);
                    if (activated) PlaySound(forceFieldSound); else PlaySound(forceFailSound);
                }

                int touch_count = GetTouchPointCount();
                if (touch_count == 2 && touch_count_last_frame != 2)
                {
                    bool activated = ActivateForceField(&ffMgr);
                    if (activated) PlaySound(forceFieldSound); else PlaySound(forceFailSound);
                }
                touch_count_last_frame = touch_count;

                UpdateLasers(&laserMgr);
                UpdateStarfield();
                UpdateEnemies(&enemyMgr, &lives, &wave);
                bool ffHit = UpdateForceField(&ffMgr, &enemyMgr);
                if (ffHit) PlaySound(forceFieldHitSound);

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
            DrawEnemies(&enemyMgr);
            DrawLasers(&laserMgr);
            EndMode3D();
            DrawForceField2D(&ffMgr);

            DrawCircleLines(GetMouseX(), GetMouseY(), 10, MAROON);
            DrawLine(GetMouseX() - 20, GetMouseY(), GetMouseX() + 20, GetMouseY(), RED);
            DrawLine(GetMouseX(), GetMouseY() - 20, GetMouseX(), GetMouseY() + 20, RED);

            DrawText(TextFormat("Score: %i", score), 10, 10, 20, GREEN);
            DrawText(TextFormat("Lives: %i", lives), GetScreenWidth() - 100, 10, 20, RED);
            DrawText(TextFormat("Wave: %i", wave), GetScreenWidth() / 2 - 20, 10, 20, YELLOW);
            DrawForceFieldUI(&ffMgr);
        }
        else if (gameState == STATE_GAME_OVER)
        {
            DrawText("GAME OVER", GetScreenWidth() / 2 - MeasureText("GAME OVER", 40) / 2, GetScreenHeight() / 2 - 40, 40, RED);
            DrawText(TextFormat("Final Score: %i", score), GetScreenWidth() / 2 - MeasureText(TextFormat("Final Score: %i", score), 20) / 2, GetScreenHeight() / 2 + 20, 20, GREEN);
            DrawText("Press ENTER or CLICK to Return to Start", GetScreenWidth() / 2 - MeasureText("Press ENTER or CLICK to Return to Start", 20) / 2, GetScreenHeight() / 2 + 60, 20, GRAY);
        }

        EndDrawing();

        frameCount++;
        double currentTime = GetTime();
        double elapsedTime = currentTime - previousTime;
        if (elapsedTime >= 1.0)
        {
#if defined(PLATFORM_WEB)
            emscripten_log(EM_LOG_CONSOLE, "Average FPS: %.2f", frameCount / elapsedTime);
#else
            printf("Average FPS: %.2f\n", frameCount / elapsedTime);
#endif
            frameCount = 0;
            previousTime = currentTime;
        }
    }

    UnloadSound(shootSound);
    UnloadSound(explosionSound);
    UnloadSound(forceFieldSound);
    UnloadSound(forceFailSound);
    UnloadSound(forceFieldHitSound);
    CloseAudioDevice();

    CloseWindow();

    return 0;
}

//----------------------------------------------------------------------------------
// InitGame - Implementation Notes:
// - Resets score, lives and wave to starting values
// - Initializes all subsystems used by the gameplay loop
// - Spawns initial enemy wave
//----------------------------------------------------------------------------------
void InitGame(int* score, int* lives, int* wave, struct LaserManager* lmgr, struct EnemyManager* emgr, struct ForceFieldManager* ffmgr)
{
    *score = 0;
    *lives = 3;
    *wave = 1;
    InitLasers(lmgr);
    InitEnemies(emgr);
    // Spawn the initial set of enemies for the first wave
    SpawnWave(emgr, *wave);
    InitStarfield();
    InitForceField(ffmgr);
}
