/*******************************************************************************************
*
*   forcefield.c - Force field defensive system implementation
*
*   See forcefield.h for module interface documentation.
*   
*   Implementation notes:
*   - Uses state machine (ready/active/cooldown) for field management
*   - Repels enemies within FORCE_FIELD_RADIUS when active
*   - Visual representation as 2D grid overlay
*   - Charge system prevents continuous activation
*
*******************************************************************************************/

#include "forcefield.h"
#include "enemy.h"
#include "rlgl.h"

//----------------------------------------------------------------------------------
// Module Variables
//----------------------------------------------------------------------------------

ForceFieldState ff_state;    // Current state of the force field
float ff_charge;            // Current charge level (0.0 to 1.0)
float ff_timer;            // Timer for active duration or cooldown

//----------------------------------------------------------------------------------
// Public Function Implementations (see forcefield.h for documentation)
//----------------------------------------------------------------------------------

void InitForceField(void)
{
    ff_state = FF_STATE_READY;
    ff_charge = 1.0f;
    ff_timer = 0.0f;
}

//----------------------------------------------------------------------------------
// ActivateForceField - Implementation Notes:
// - Only activates if force field is in ready state
// - Plays different sounds for success/failure
// - Sets timer for active duration
//----------------------------------------------------------------------------------
void ActivateForceField(Sound forceFieldSound, Sound forceFailSound)
{
    if (ff_state == FF_STATE_READY)
    {
        ff_state = FF_STATE_ACTIVE;
        ff_timer = FORCE_FIELD_ACTIVE_TIME;
        PlaySound(forceFieldSound);
    } else {
        PlaySound(forceFailSound);
    }
}

//----------------------------------------------------------------------------------
// UpdateForceField - Implementation Notes:
// - Handles state transitions and timers
// - Detects and repels enemies within radius when active
// - Manages cooldown and charge regeneration
// - Plays hit sound when enemies are repelled
//----------------------------------------------------------------------------------
void UpdateForceField(Sound forceFieldHitSound)
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
        bool hit = false;
        for (int i = 0; i < MAX_ENEMIES; i++)
        {
            if (enemies[i].active && enemies[i].position.z < 0 && -enemies[i].position.z < FORCE_FIELD_RADIUS)
            {
                enemies[i].state = ENEMY_STATE_REPELLED;
                hit = true;
                enemies[i].repel_start_pos = enemies[i].position;
                enemies[i].repel_t = 0.0f;
            }
        }

        if (hit) PlaySound(forceFieldHitSound);
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

//----------------------------------------------------------------------------------
// DrawForceField2D - Implementation Notes:
// - Creates grid overlay effect when field is active
// - Divides screen into 4x2 grid of cells
// - Uses LIME color for visual consistency
//----------------------------------------------------------------------------------
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

//----------------------------------------------------------------------------------
// DrawForceFieldUI - Implementation Notes:
// - Shows charge status in text form
// - Uses color coding (GREEN for full, YELLOW for charging)
// - Displays percentage when not fully charged
//----------------------------------------------------------------------------------
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
