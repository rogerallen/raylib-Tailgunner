#include "forcefield.h"
#include "enemy.h"
#include "rlgl.h"

ForceFieldState ff_state;
float ff_charge;
float ff_timer;

void InitForceField(void)
{
    ff_state = FF_STATE_READY;
    ff_charge = 1.0f;
    ff_timer = 0.0f;
}

void ActivateForceField(void)
{
    if (ff_state == FF_STATE_READY)
    {
        ff_state = FF_STATE_ACTIVE;
        ff_timer = FORCE_FIELD_ACTIVE_TIME;
    }
}

void UpdateForceField(void)
{
    if (ff_state == FF_STATE_ACTIVE)
    {
        ff_timer -= GetFrameTime();
        if (ff_timer <= 0.0f)
        {
            ff_state = FF_STATE_COOLDOWN;
            ff_timer = FORCE_FIELD_TIMEOUT;
            ff_charge = 0.0f;
        }

        // Push back enemies
        for (int i = 0; i < MAX_ENEMIES; i++)
        {
            if (enemies[i].active && enemies[i].position.z > 0 && enemies[i].position.z < FORCE_FIELD_RADIUS)
            {
                enemies[i].state = ENEMY_STATE_REPELLED;
                enemies[i].repel_start_pos = enemies[i].position;
                enemies[i].repel_t = 0.0f;
            }
        }
    }
    else if (ff_state == FF_STATE_COOLDOWN)
    {
        ff_timer -= GetFrameTime();
        ff_charge = 1.0f - (ff_timer / FORCE_FIELD_TIMEOUT);
        if (ff_timer <= 0.0f)
        {
            ff_state = FF_STATE_READY;
            ff_charge = 1.0f;
        }
    }
}

void DrawForceField2D(void)
{
    if (ff_state == FF_STATE_ACTIVE)
    {
        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();
        int cellWidth = screenWidth / 4;
        int cellHeight = screenHeight / 2;

        for (int y = 0; y < 2; y++)
        {
            for (int x = 0; x < 4; x++)
            {
                DrawRectangleLines(x * cellWidth, y * cellHeight, cellWidth, cellHeight, LIME);
            }
        }
    }
}

void DrawForceFieldUI(void)
{
    if (ff_state == FF_STATE_READY)
    {
        DrawText("Charge: FULL", 10, 40, 20, GREEN);
    }
    else
    {
        DrawText(TextFormat("Charge: %i%%", (int)(ff_charge * 100)), 10, 40, 20, YELLOW);
    }
}
