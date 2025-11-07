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

// No module-level globals: state is stored in ForceFieldManager instances

//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
// Module: ForceFieldManager-based implementation (no globals)
//----------------------------------------------------------------------------------

void InitForceField(ForceFieldManager* mgr)
{
    mgr->state = FF_STATE_READY;
    mgr->charge = 1.0f;
    mgr->timer = 0.0f;
}

bool ActivateForceField(ForceFieldManager* mgr)
{
    if (mgr->state == FF_STATE_READY)
    {
        mgr->state = FF_STATE_ACTIVE;
        mgr->timer = FORCE_FIELD_ACTIVE_TIME;
        return true; // activation succeeded
    }
    return false; // activation failed (cooldown)
}

bool UpdateForceField(ForceFieldManager* mgr, struct EnemyManager* emgr)
{
    bool anyHit = false;

    if (mgr->state == FF_STATE_ACTIVE)
    {
        mgr->timer -= GetFrameTime();
        if (mgr->timer <= 0.0f)
        {
            mgr->state = FF_STATE_COOLDOWN;
            mgr->timer = FORCE_FIELD_TIMEOUT;
            mgr->charge = 0.0f;
        }

        // Push back enemies
        for (int i = 0; i < WAVE_SIZE; i++)
        {
            Enemy* e = &emgr->enemies[i];
            if (e->active && e->position.z < 0 && -e->position.z < FORCE_FIELD_RADIUS)
            {
                e->state = ENEMY_STATE_REPELLED;
                anyHit = true;
                e->repel_start_pos = e->position;
                e->repel_t = 0.0f;
            }
        }
    }
    else if (mgr->state == FF_STATE_COOLDOWN)
    {
        mgr->timer -= GetFrameTime();
        mgr->charge = 1.0f - (mgr->timer / FORCE_FIELD_TIMEOUT);
        if (mgr->timer <= 0.0f)
        {
            mgr->state = FF_STATE_READY;
            mgr->charge = 1.0f;
        }
    }

    return anyHit;
}

void DrawForceField2D(ForceFieldManager* mgr)
{
    if (mgr->state == FF_STATE_ACTIVE)
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

void DrawForceFieldUI(ForceFieldManager* mgr)
{
    if (mgr->state == FF_STATE_READY)
    {
        DrawText("Charge: FULL", 10, 40, 20, GREEN);
    }
    else
    {
        DrawText(TextFormat("Charge: %i%%", (int)(mgr->charge * 100)), 10, 40, 20, YELLOW);
    }
}
// - Displays percentage when not fully charged
// (All UI/draw/update functions now receive a ForceFieldManager* and no globals remain.)
