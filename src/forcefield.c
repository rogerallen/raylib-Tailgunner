//================================================================================================
//
//   forcefield.c - Force field defensive system implementation
//
//   See forcefield.h for module interface documentation.
//
//   Implementation notes:
//   - Uses state machine (ready/active/cooldown) for field management
//   - Repels enemies within FORCE_FIELD_RADIUS when active
//   - Visual representation as 2D grid overlay
//   - Charge system prevents continuous activation
//
//================================================================================================

#include "forcefield.h"
#include "enemy.h"
#include "rlgl.h"

// No module-level globals: state is stored in ForceFieldManager instances

//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
// Module: ForceFieldManager-based implementation (no globals)
//----------------------------------------------------------------------------------

void InitForceField(ForceFieldManager *mgr)
{
    mgr->state = FF_STATE_READY;
    mgr->charge = 1.0f;
    mgr->timer = 0.0f;
}

bool ActivateForceField(ForceFieldManager *mgr)
{
    if (mgr->state == FF_STATE_READY) {
        mgr->state = FF_STATE_ACTIVE;
        mgr->timer = FORCE_FIELD_ACTIVE_TIME;
        return true; // activation succeeded
    }
    return false; // activation failed (cooldown)
}

bool UpdateForceField(ForceFieldManager *mgr, struct EnemyManager *emgr)
{
    bool anyHit = false;

    if (mgr->state == FF_STATE_ACTIVE) {
        mgr->timer -= GetFrameTime();
        if (mgr->timer <= 0.0f) {
            mgr->state = FF_STATE_COOLDOWN;
            mgr->timer = FORCE_FIELD_TIMEOUT;
            mgr->charge = 0.0f;
        }

        // Push back enemies
        for (int i = 0; i < WAVE_SIZE; i++) {
            Enemy *e = &emgr->enemies[i];
            if (e->active && e->position.z < 0 && -e->position.z < FORCE_FIELD_RADIUS) {
                e->state = ENEMY_STATE_REPELLED;
                anyHit = true;
                e->repel_start_pos = e->position;
                e->repel_t = 0.0f;
            }
        }
    }
    else if (mgr->state == FF_STATE_COOLDOWN) {
        mgr->timer -= GetFrameTime();
        mgr->charge = 1.0f - (mgr->timer / FORCE_FIELD_TIMEOUT);
        if (mgr->timer <= 0.0f) {
            mgr->state = FF_STATE_READY;
            mgr->charge = 1.0f;
        }
    }

    return anyHit;
}
//
// DrawForceField2D - Implementation Notes:
// - 3 rotated squares horizontally in 2 rows for grid effect
// - so, square diagonals align with screen axes and are 3 wide, 2 high
// - square diagonal is min(screenWidth*0.8/3, screenHeight*0.8/2)
//
void DrawForceField2D(ForceFieldManager *mgr)
{
    if (mgr->state == FF_STATE_ACTIVE) {
        int ffWidth = GetScreenWidth() * 0.80f;
        int ffHeight = GetScreenHeight() * 0.80f;
        int cellDiagonal = (ffWidth / 3 < ffHeight / 2) ? (ffWidth / 3) : (ffHeight / 2);
        int xc = GetScreenWidth() / 2;
        int yc = GetScreenHeight() / 2;
        // GRID is x0, x1, xc, x2, x3 and y0, y1, yc, y2, y3 (upper left origin)
        int x0 = xc - cellDiagonal * 1.5f;
        int x1 = xc - cellDiagonal;
        int x2 = xc + cellDiagonal;
        int x3 = xc + cellDiagonal * 1.5f;
        int y0 = yc - cellDiagonal;
        int y1 = yc - cellDiagonal / 2;
        int y2 = yc + cellDiagonal / 2;
        int y3 = yc + cellDiagonal;
        // Draw the lines that make up the grid, first to SE
        DrawLine(x0, y2, x1, y3, COLOR_FORCEFIELD_GRID);
        DrawLine(x0, y1, xc, y3, COLOR_FORCEFIELD_GRID);
        DrawLine(x1, y0, x2, y3, COLOR_FORCEFIELD_GRID);
        DrawLine(xc, y0, x3, y2, COLOR_FORCEFIELD_GRID);
        DrawLine(x2, y0, x3, y1, COLOR_FORCEFIELD_GRID);
        // then to NE
        DrawLine(x0, y1, x1, y0, COLOR_FORCEFIELD_GRID);
        DrawLine(x0, y2, xc, y0, COLOR_FORCEFIELD_GRID);
        DrawLine(x1, y3, x2, y0, COLOR_FORCEFIELD_GRID);
        DrawLine(xc, y3, x3, y1, COLOR_FORCEFIELD_GRID);
        DrawLine(x2, y3, x3, y2, COLOR_FORCEFIELD_GRID);
    }
}

void DrawForceFieldUI(ForceFieldManager *mgr)
{
    if (mgr->state == FF_STATE_READY) {
        DrawText("Charge: FULL", 10, 40, 20, COLOR_FORCEFIELD_UI_READY);
    }
    else {
        DrawText(TextFormat("Charge: %i%%", (int)(mgr->charge * 100)), 10, 40, 20, COLOR_FORCEFIELD_UI_CHARGING);
    }
}
// - Displays percentage when not fully charged
// (All UI/draw/update functions now receive a ForceFieldManager* and no globals remain.)
