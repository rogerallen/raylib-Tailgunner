/*******************************************************************************************
*
*   enemy.c - Enemy implementation
*
*   See enemy.h for module interface documentation.
*
*******************************************************************************************/

#include "enemy.h"
#include "config.h"
#include "raymath.h"
#include "rlgl.h"

#include <stdio.h>

// No module-level globals: all enemy state lives inside EnemyManager passed by callers

//----------------------------------------------------------------------------------
// Internal Function Declarations
//----------------------------------------------------------------------------------

/**
 * Render a single enemy ship using line-based 3D geometry
 * 
 * @param enemy The enemy to render, must be active
 */
static void DrawEnemyShip(const Enemy* enemy);

/**
 * Calculate the normalized tangent vector at point t along a cubic Bezier curve
 * 
 * @param p0,p1,p2,p3 Control points defining the curve
 * @param t Parameter value along curve [0,1]
 * @return Normalized tangent vector (or forward vector if tangent is zero)
 */
static Vector3 GetCubicBezierTangent(Vector3 p0, Vector3 p1, Vector3 p2, Vector3 p3, float t);

/**
 * Calculate a point on a cubic Bezier curve using De Casteljau's algorithm
 * 
 * @param p0,p1,p2,p3 Control points defining the curve
 * @param t Parameter value along curve [0,1]
 * @return Position vector of point on curve
 */
static Vector3 GetCubicBezierPoint(Vector3 p0, Vector3 p1, Vector3 p2, Vector3 p3, float t)
{
    Vector3 result;
    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    result.x = uuu * p0.x + 3 * uu * t * p1.x + 3 * u * tt * p2.x + ttt * p3.x;
    result.y = uuu * p0.y + 3 * uu * t * p1.y + 3 * u * tt * p2.y + ttt * p3.y;
    result.z = uuu * p0.z + 3 * uu * t * p1.z + 3 * u * tt * p2.z + ttt * p3.z;

    return result;
}

static Vector3 GetCubicBezierTangent(Vector3 p0, Vector3 p1, Vector3 p2, Vector3 p3, float t)
{
    Vector3 result;
    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;

    result.x = 3 * uu * (p1.x - p0.x) + 6 * u * t * (p2.x - p1.x) + 3 * tt * (p3.x - p2.x);
    result.y = 3 * uu * (p1.y - p0.y) + 6 * u * t * (p2.y - p1.y) + 3 * tt * (p3.y - p2.y);
    result.z = 3 * uu * (p1.z - p0.z) + 6 * u * t * (p2.z - p1.z) + 3 * tt * (p3.z - p2.z);

    // Avoid normalizing a near-zero vector which can produce NaNs
    float len = Vector3Length(result);
    if (len < 1e-6f) return (Vector3){ 0.0f, 0.0f, 0.0f };
    return Vector3Scale(result, 1.0f / len);
}

//----------------------------------------------------------------------------------
// Public Function Implementations (see enemy.h for documentation)
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
// InitEnemies - Implementation Notes:
// - Sets all enemies to inactive
// - Initializes default properties (radius, color)
// - Sets up transform (axis, angle) for rotation effects
//----------------------------------------------------------------------------------
void InitEnemies(EnemyManager* mgr)
{
    for (int i = 0; i < WAVE_SIZE; i++)
    {
        mgr->enemies[i].active = false;
        mgr->enemies[i].radius = ENEMY_DEFAULT_RADIUS;
        mgr->enemies[i].color = COLOR_ENEMY;
        mgr->enemies[i].state = ENEMY_STATE_NORMAL;
        mgr->enemies[i].rotationAxis = (Vector3){ 0.0f, 1.0f, 0.0f };
        mgr->enemies[i].rotationAngle = 0.0f;
    }
}

//----------------------------------------------------------------------------------
// SpawnWave - Implementation Notes:
// - Enemies are all inactive before calling
// - Assigns random but controlled Bezier curve paths
// - Alternates enemies between left/right approach paths
// - Staggers enemy positions with z-offset
//----------------------------------------------------------------------------------
void SpawnWave(EnemyManager* mgr, int wave)
{

    for (int i = 0; i < WAVE_SIZE; i++)
    {
        Enemy* e = &mgr->enemies[i];
        e->active = true;
        e->state = ENEMY_STATE_NORMAL;
        e->t = 0.0f;
        e->rotationAngle = 0.0f;
        e->rotationAxis = (Vector3){ 0.0f, 1.0f, 0.0f };

        float zOffset = i * ENEMY_Z_OFFSET;
        int side = (i % 2 == 0) ? 1 : -1;
        int r0x = GetRandomValue(-ENEMY_XY_START_RANGE, ENEMY_XY_START_RANGE);
        int r0y = GetRandomValue(-ENEMY_XY_START_RANGE, ENEMY_XY_START_RANGE);

        e->p0 = (Vector3){ (float)r0x,                             (float)r0y,                     -100.0f - zOffset };
        e->p1 = (Vector3){ (float)GetRandomValue(-5, 5),           (float)GetRandomValue(-5, 5),    -50.0f - zOffset/2.0f };
        e->p2 = (Vector3){ (float)GetRandomValue(-40, -20) * side, (float)GetRandomValue(10, 20),   -25.0f };
        e->p3 = (Vector3){ (float)GetRandomValue(20, 40) * side,   (float)GetRandomValue(-20, -10),  10.0f };

        // Apply wave-based nerfing of enemies
        // waves count from 1,2,3,...
        if (wave <= WAVE_NERF2_LEVELS) {
            if(i >= WAVE_SIZE - 2) {
                printf("Nerfing enemy at index %d for wave %d\n", i, wave);
                e->active = false;
            }
        } else if (wave <= WAVE_NERF1_LEVELS) {
            if(i >= WAVE_SIZE - 1) {
                printf("Nerfing enemy at index %d for wave %d\n", i, wave);
                e->active = false;
            }
        }
    }
    
}

//----------------------------------------------------------------------------------
// UpdateEnemies - Implementation Notes:
// - Handles enemy state transitions (normal/repelled)
// - Moves enemies along Bezier paths or linear repel paths
// - Increases speed with wave number
// - Spawns new wave when all enemies inactive
// - Updates lives when enemies pass player
//----------------------------------------------------------------------------------
void UpdateEnemies(EnemyManager* mgr, int* lives, int* wave)
{
    int activeEnemies = 0;

    for (int i = 0; i < WAVE_SIZE; i++)
    {
        Enemy* e = &mgr->enemies[i];
        if (e->active)
        {
            activeEnemies++;

            switch (e->state)
            {
                case ENEMY_STATE_NORMAL:
                {
                    e->t += ENEMY_DT_DFRAME + (*wave * ENEMY_WAVE_DT_DFRAME);
                    e->position = GetCubicBezierPoint(e->p0, e->p1, e->p2, e->p3, e->t);

                    if (e->t >= 1.0f)
                    {
                        e->active = false;
                        (*lives)--;
                    }
                } break;
                case ENEMY_STATE_REPELLED:
                {
                    e->repel_t += ENEMY_REPEL_DT_DFRAME;
                    e->position = Vector3Lerp(e->repel_start_pos, e->p0, e->repel_t);
                    e->rotationAngle += 360.0f * GetFrameTime();

                    if (e->repel_t >= 1.0f)
                    {
                        e->state = ENEMY_STATE_NORMAL;
                        e->t = 0.0f;
                        e->rotationAngle = 0.0f;
                    }
                } break;
            }
        }
    }

    if (activeEnemies == 0)
    {
        (*wave)++;
        SpawnWave(mgr, *wave);
    }
}

//----------------------------------------------------------------------------------
// DrawEnemies - Implementation Notes:
// - Renders only active enemies
// - Uses line-based 3D geometry for wireframe look
// - Handles orientation based on movement direction
// - Adds rotation effect during repel state
//----------------------------------------------------------------------------------
void DrawEnemies(EnemyManager* mgr)
{
    for (int i = 0; i < WAVE_SIZE; i++)
    {
        if (mgr->enemies[i].active)
        {
            DrawEnemyShip(&mgr->enemies[i]);
        }
    }
}

//----------------------------------------------------------------------------------
// Internal Function Implementations
//----------------------------------------------------------------------------------

static void DrawEnemyShip(const Enemy* enemy)
{
    float r = enemy->radius;
    float fin_r = 1.0f; // Fin size relative to body radius
    Vector3 forward;
    if (enemy->state == ENEMY_STATE_REPELLED) {
        Vector3 to_p0 = Vector3Subtract(enemy->p0, enemy->position);
        if (Vector3LengthSqr(to_p0) > 0.0001f) {
            forward = Vector3Normalize(to_p0);
        } else {
            forward = GetCubicBezierTangent(enemy->p0, enemy->p1, enemy->p2, enemy->p3, 0.0f);
        }
    } else {
        forward = GetCubicBezierTangent(enemy->p0, enemy->p1, enemy->p2, enemy->p3, enemy->t);
        if (Vector3LengthSqr(forward) < 0.0001f) {
            forward = Vector3Normalize(Vector3Negate(enemy->position));
        }
    }

    Vector3 up = { 0.0f, 1.0f, 0.0f };
    Vector3 right = Vector3CrossProduct(forward, up);
    up = Vector3CrossProduct(right, forward);

    Matrix transform = {
        right.x, up.x, forward.x, enemy->position.x,
        right.y, up.y, forward.y, enemy->position.y,
        right.z, up.z, forward.z, enemy->position.z,
        0, 0, 0, 1
    };

    rlPushMatrix();
    rlMultMatrixf(MatrixToFloat(transform));
    rlRotatef(enemy->rotationAngle, enemy->rotationAxis.x, enemy->rotationAxis.y, enemy->rotationAxis.z);

    Vector3 v_top = { 0, r, 0 };
    Vector3 v_bottom = { 0, -r, 0 };
    Vector3 v_right = { r, 0, 0 };
    Vector3 v_left = { -r, 0, 0 };
    Vector3 v_front = { 0, 0, r * 2 }; // 2x longer
    Vector3 v_back = { 0, 0, -r * 2 }; // 2x longer

    // Main body
    DrawLine3D(v_front, v_top, enemy->color);
    DrawLine3D(v_front, v_bottom, enemy->color);
    DrawLine3D(v_front, v_left, enemy->color);
    DrawLine3D(v_front, v_right, enemy->color);

    DrawLine3D(v_back, v_top, enemy->color);
    DrawLine3D(v_back, v_bottom, enemy->color);
    DrawLine3D(v_back, v_left, enemy->color);
    DrawLine3D(v_back, v_right, enemy->color);

    DrawLine3D(v_top, v_right, enemy->color);
    DrawLine3D(v_right, v_bottom, enemy->color);
    DrawLine3D(v_bottom, v_left, enemy->color);
    DrawLine3D(v_left, v_top, enemy->color);

    // Fins
    Vector3 fin_top_back = { 0, fin_r, -r * 2 - fin_r };
    Vector3 fin_bottom_back = { 0, -fin_r, -r * 2 - fin_r };
    Vector3 fin_left_back = { -fin_r, 0, -r * 2 - fin_r };
    Vector3 fin_right_back = { fin_r, 0, -r * 2 - fin_r };

    DrawLine3D(v_top, fin_top_back, enemy->color);
    DrawLine3D(v_back, fin_top_back, enemy->color);

    DrawLine3D(v_bottom, fin_bottom_back, enemy->color);
    DrawLine3D(v_back, fin_bottom_back, enemy->color);

    DrawLine3D(v_left, fin_left_back, enemy->color);
    DrawLine3D(v_back, fin_left_back, enemy->color);

    DrawLine3D(v_right, fin_right_back, enemy->color);
    DrawLine3D(v_back, fin_right_back, enemy->color);

    rlPopMatrix();
}
