#include "raylib.h"
#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif
#include "config.h"
#include "enemy.h"
#include "forcefield.h"
#include "game.h"
#include "laser.h"
#include "leaderboard.h"
#include "starfield.h"
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

void InitGame(int *score, int *lives, int *wave, struct LaserManager *lmgr, struct EnemyManager *emgr,
              struct ForceFieldManager *ffmgr, struct LeaderboardManager *lbmgr);

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
    Camera camera = {0};
    camera.position = (Vector3){0.0f, 0.0f, 0.0f};
    camera.target = (Vector3){0.0f, 0.0f, -1.0f};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    int score = 0;
    int score_at_last_life = 0;
    int lives = 0;
    int wave = 0;
    bool nameRequired = true;

    // Encapsulated managers (avoid globals)
    LaserManager laserMgr = {0};
    EnemyManager enemyMgr = {0};
    ForceFieldManager ffMgr = {0};

    InitAudioDevice();

    // Leaderboard manager (avoid globals)
    LeaderboardManager lbMgr = {0};
    InitLeaderboard(&lbMgr);

    Sound shootSound = LoadSound("resources/shoot.wav");
    Sound explosionSound = LoadSound("resources/explosion.wav");
    Sound forceFieldSound = LoadSound("resources/forcefield.wav");
    Sound forceFailSound = LoadSound("resources/forcefail.wav");
    Sound forceFieldHitSound = LoadSound("resources/bounce.wav");
    Sound lostLifeSound = LoadSound("resources/past.wav");
    Sound extraLifeSound = LoadSound("resources/forcefield.wav");

    Rectangle showTop10Button = (Rectangle){GetScreenWidth() / 2 - 60, GetScreenHeight() / 2 + 80, 120, 30};
    Rectangle showHelpButton = (Rectangle){GetScreenWidth() / 2 - 60, GetScreenHeight() / 2 + 120, 120, 30};

    int frameCount = 0;
    int touch_count_last_frame = 0;
    double previousTime = GetTime();

    while (!WindowShouldClose()) {
        switch (gameState) {
        case STATE_START: {
            if (CheckCollisionPointRec(GetMousePosition(), showTop10Button) &&
                IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                ResetLeaderboardFlags(&lbMgr);
                gameState = STATE_LEADERBOARD;
                SetLeaderboardActive(&lbMgr, true);
            }
            else if (CheckCollisionPointRec(GetMousePosition(), showHelpButton) &&
                     IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                gameState = STATE_HELP;
            }
            else if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                InitGame(&score, &lives, &wave, &laserMgr, &enemyMgr, &ffMgr, &lbMgr);
                gameState = STATE_PLAYING;
                HideCursor();
            }
        } break;
        case STATE_PLAYING: {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                Ray ray = GetMouseRay(GetMousePosition(), camera);
                int hits = FireLasers(&laserMgr, &enemyMgr, ray, camera, explosionSound);
                score += hits;
                if (hits > 0) PlaySound(explosionSound);
                PlaySound(shootSound);
            }

            if (IsKeyPressed(KEY_SPACE)) {
                bool activated = ActivateForceField(&ffMgr);
                if (activated)
                    PlaySound(forceFieldSound);
                else
                    PlaySound(forceFailSound);
            }

            int touch_count = GetTouchPointCount();
            if (touch_count == 2 && touch_count_last_frame != 2) {
                bool activated = ActivateForceField(&ffMgr);
                if (activated)
                    PlaySound(forceFieldSound);
                else
                    PlaySound(forceFailSound);
            }
            touch_count_last_frame = touch_count;

            UpdateLasers(&laserMgr);
            UpdateStarfield();
            int curLives = lives;
            UpdateEnemies(&enemyMgr, &lives, &wave);
            bool ffHit = UpdateForceField(&ffMgr, &enemyMgr);
            if (ffHit) PlaySound(forceFieldHitSound);

            if (curLives > lives) {
                // Lost a life this frame
                PlaySound(lostLifeSound);
            }

            if (lives <= 0) {
                gameState = STATE_GAME_OVER;
                ShowCursor();
            }

            // Check for extra life
            if (score - score_at_last_life >= POINTS_FOR_EXTRA_LIFE) {
                lives++;
                score_at_last_life += POINTS_FOR_EXTRA_LIFE;
                PlaySound(extraLifeSound);
            }

        } break;
        case STATE_GAME_OVER: {
            if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (nameRequired) {
                    gameState = STATE_ENTER_NAME;
                }
                else {
                    gameState = STATE_SUBMIT_SCORE;
                }
            }
        } break;
        case STATE_ENTER_NAME: {
            // UpdateNameInput is in leaderboard.c
            // It returns true when the name is submitted
            if (UpdateNameInput(&lbMgr)) {
                nameRequired = false;
                gameState = STATE_SUBMIT_SCORE;
            }
        } break;
        case STATE_SUBMIT_SCORE: {
            SubmitScore(&lbMgr, score);
            ResetLeaderboardFlags(&lbMgr);
            gameState = STATE_LEADERBOARD;
            SetLeaderboardActive(&lbMgr, true);
        } break;
        case STATE_LEADERBOARD: {
            UpdateLeaderboard(&lbMgr, (int *)&gameState, score);
        } break;
        case STATE_HELP: {
            if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                gameState = STATE_START;
            }
        } break;
        default: {
            printf("ERROR: Unknown game state!  Just going to the start.\n");
            gameState = STATE_START; // Just go to start on unknown state
        } break;
        }

        BeginDrawing();
        ClearBackground(COLOR_BACKGROUND);

        if (gameState == STATE_START) {
            DrawText(GAME_TITLE, GetScreenWidth() / 2 - MeasureText(GAME_TITLE, 40) / 2, GetScreenHeight() / 2 - 40, 40,
                     COLOR_TEXT_TITLE);
            DrawText("Press ENTER or CLICK to Start",
                     GetScreenWidth() / 2 - MeasureText("Press ENTER or CLICK to Start", 20) / 2,
                     GetScreenHeight() / 2 + 20, 20, COLOR_TEXT_SUBTITLE);

            DrawRectangleLinesEx(showTop10Button, 2, COLOR_BUTTON_BOX);
            DrawText("Top 10", showTop10Button.x + 30, showTop10Button.y + 5, 20, COLOR_BUTTON_BOX);

            DrawRectangleLinesEx(showHelpButton, 2, COLOR_BUTTON_BOX);
            DrawText("Help", showHelpButton.x + 40, showHelpButton.y + 5, 20, COLOR_BUTTON_BOX);

            DrawText(GAME_VERSION, 10, GetScreenHeight() - 20, 12, COLOR_TEXT_SUBTITLE);
        }
        else if (gameState == STATE_PLAYING) {
            BeginMode3D(camera);
            DrawStarfield();
            DrawEnemies(&enemyMgr);
            DrawLasers(&laserMgr);
            EndMode3D();
            DrawForceField2D(&ffMgr);

            DrawCircleLines(GetMouseX(), GetMouseY(), 10, COLOR_CROSSHAIR_CIRCLE);
            DrawLine(GetMouseX() - 20, GetMouseY(), GetMouseX() + 20, GetMouseY(), COLOR_CROSSHAIR_LINES);
            DrawLine(GetMouseX(), GetMouseY() - 20, GetMouseX(), GetMouseY() + 20, COLOR_CROSSHAIR_LINES);

            DrawText(TextFormat("Score: %i", score), 10, 10, 20, COLOR_TEXT_SCORE);
            DrawText(TextFormat("Lives: %i", lives), GetScreenWidth() - 100, 10, 20, COLOR_TEXT_LIVES);
            DrawText(TextFormat("Wave: %i", wave), GetScreenWidth() / 2 - 20, 10, 20, COLOR_TEXT_WAVE);
            DrawForceFieldUI(&ffMgr);
        }
        else if (gameState == STATE_GAME_OVER) {
            DrawText("GAME OVER", GetScreenWidth() / 2 - MeasureText("GAME OVER", 40) / 2, GetScreenHeight() / 2 - 40,
                     40, COLOR_TEXT_GAMEOVER);
            DrawText(TextFormat("Final Score: %i", score),
                     GetScreenWidth() / 2 - MeasureText(TextFormat("Final Score: %i", score), 20) / 2,
                     GetScreenHeight() / 2 + 20, 20, COLOR_TEXT_FINAL_SCORE);
            DrawText("Press ENTER or CLICK to Continue",
                     GetScreenWidth() / 2 - MeasureText("Press ENTER or CLICK to Continue", 20) / 2,
                     GetScreenHeight() / 2 + 60, 20, COLOR_TEXT_SUBTITLE);
        }
        else if (gameState == STATE_ENTER_NAME) {
            DrawNameInput(&lbMgr);
        }
        else if (gameState == STATE_LEADERBOARD) {
            DrawLeaderboard(&lbMgr);
        }
        else if (gameState == STATE_HELP) {
            DrawText("Help / Instructions", 100, 50, 30, COLOR_TEXT_TITLE);
            DrawText("Use the mouse or touch to aim and click to shoot lasers at incoming enemies.", 100, 100, 20,
                     COLOR_TEXT_SUBTITLE);
            DrawText("Press SPACE or use two-finger touch to activate the force field.", 100, 125, 20,
                     COLOR_TEXT_SUBTITLE);
            DrawText("Avoid letting enemies reach you, or you'll lose lives!", 100, 150, 20, COLOR_TEXT_SUBTITLE);
            DrawText("Gain an extra life every 50 points scored.", 100, 175, 20, COLOR_TEXT_SUBTITLE);
            DrawText("Survive as many waves as you can and achieve a high score!", 100, 200, 20, COLOR_TEXT_SUBTITLE);
            DrawText("Press ENTER or CLICK to return to the main menu.", 100, 250, 20, COLOR_TEXT_SUBTITLE);
        }

        EndDrawing();

        frameCount++;
        double currentTime = GetTime();
        double elapsedTime = currentTime - previousTime;
        if (elapsedTime >= 10.0) {
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
void InitGame(int *score, int *lives, int *wave, struct LaserManager *lmgr, struct EnemyManager *emgr,
              struct ForceFieldManager *ffmgr, struct LeaderboardManager *lbmgr)
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
    ResetLeaderboardFlags(lbmgr);
}
