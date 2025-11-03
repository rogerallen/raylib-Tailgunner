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

//----------------------------------------------------------------------------------
// Module Variables
//----------------------------------------------------------------------------------

Enemy enemies[MAX_ENEMIES];

//----------------------------------------------------------------------------------
// Internal Function Declarations
//----------------------------------------------------------------------------------

/**
 * Render a single enemy ship using line-based 3D geometry
 * 
 * @param enemy The enemy to render, must be active
 */
static void DrawEnemyShip(Enemy enemy);

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
    if (len < 1e-6f) return (Vector3){ 0.0f, 0.0f, 1.0f };
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
void InitEnemies(void)
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        enemies[i].active = false;
        enemies[i].radius = ENEMY_DEFAULT_RADIUS;
        enemies[i].color = BLUE;
        enemies[i].state = ENEMY_STATE_NORMAL;
        enemies[i].rotationAxis = (Vector3){ 0.0f, 1.0f, 0.0f };
        enemies[i].rotationAngle = 0.0f;
    }
}

//----------------------------------------------------------------------------------
// SpawnWave - Implementation Notes:
// - Finds inactive enemies to reuse
// - Assigns random but controlled Bezier curve paths
// - Alternates enemies between left/right approach paths
// - Staggers enemy positions with z-offset
//----------------------------------------------------------------------------------
void SpawnWave(int wave)
{
    int waveSize = WAVE_SIZE;
    int enemyIndex[waveSize];
    int found = 0;

    // Find enough inactive enemies for a wave
    for (int i = 0; i < MAX_ENEMIES && found < waveSize; i++)
    {
        if (!enemies[i].active)
        {
            enemyIndex[found] = i;
            found++;
        }
    }

    if (found == waveSize)
    {
        for (int i = 0; i < waveSize; i++)
        {
            enemies[enemyIndex[i]].active = true;
            enemies[enemyIndex[i]].state = ENEMY_STATE_NORMAL;
            enemies[enemyIndex[i]].t = 0.0f;

            float zOffset = i * -10.0f;
            int side = (i % 2 == 0) ? 1 : -1;

            enemies[enemyIndex[i]].p0 = (Vector3){ (float)GetRandomValue(-20, 20), (float)GetRandomValue(-20, 20), -100.0f + zOffset };
            enemies[enemyIndex[i]].p1 = (Vector3){ (float)GetRandomValue(-5, 5), (float)GetRandomValue(-5, 5), -50.0f + zOffset };
            enemies[enemyIndex[i]].p2 = (Vector3){ (float)GetRandomValue(-40, -20) * side, (float)GetRandomValue(10, 20), -25.0f };
            enemies[enemyIndex[i]].p3 = (Vector3){ (float)GetRandomValue(20, 40) * side, (float)GetRandomValue(-20, -10), 10.0f };
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
void UpdateEnemies(int* lives, int* wave)
{
    int activeEnemies = 0;

    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (enemies[i].active)
        {
            activeEnemies++;

            switch (enemies[i].state)
            {
                case ENEMY_STATE_NORMAL:
                {
                    enemies[i].t += ENEMY_DT_DFRAME + (*wave * ENEMY_WAVE_DT_DFRAME);
                    enemies[i].position = GetCubicBezierPoint(enemies[i].p0, enemies[i].p1, enemies[i].p2, enemies[i].p3, enemies[i].t);

                    if (enemies[i].t >= 1.0f)
                    {
                        enemies[i].active = false;
                        (*lives)--;
                    }
                } break;
                case ENEMY_STATE_REPELLED:
                {
                    enemies[i].repel_t += ENEMY_REPEL_DT_DFRAME;
                    enemies[i].position = Vector3Lerp(enemies[i].repel_start_pos, enemies[i].p0, enemies[i].repel_t);
                    enemies[i].rotationAngle += 360.0f * GetFrameTime();

                    if (enemies[i].repel_t >= 1.0f)
                    {
                        enemies[i].state = ENEMY_STATE_NORMAL;
                        enemies[i].t = 0.0f;
                        enemies[i].rotationAngle = 0.0f;
                    }
                } break;
            }
        }
    }

    if (activeEnemies == 0)
    {
        (*wave)++;
        SpawnWave(*wave);
    }
}

//----------------------------------------------------------------------------------
// DrawEnemies - Implementation Notes:
// - Renders only active enemies
// - Uses line-based 3D geometry for wireframe look
// - Handles orientation based on movement direction
// - Adds rotation effect during repel state
//----------------------------------------------------------------------------------
void DrawEnemies(void)
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (enemies[i].active)
        {
            DrawEnemyShip(enemies[i]);
        }
    }
}

//----------------------------------------------------------------------------------
// Internal Function Implementations
//----------------------------------------------------------------------------------

static void DrawEnemyShip(Enemy enemy)
{
    float r = enemy.radius;
    float fin_r = 1.0f; // Fin size relative to body radius

    Vector3 forward = GetCubicBezierTangent(enemy.p0, enemy.p1, enemy.p2, enemy.p3, enemy.t);
    if (enemy.state == ENEMY_STATE_REPELLED) forward = Vector3Normalize(Vector3Subtract(enemy.p0, enemy.position));

    Vector3 up = { 0.0f, 1.0f, 0.0f };
    Vector3 right = Vector3CrossProduct(forward, up);
    up = Vector3CrossProduct(right, forward);

    Matrix transform = {
        right.x, up.x, forward.x, enemy.position.x,
        right.y, up.y, forward.y, enemy.position.y,
        right.z, up.z, forward.z, enemy.position.z,
        0, 0, 0, 1
    };

    rlPushMatrix();
    rlMultMatrixf(MatrixToFloat(transform));
    rlRotatef(enemy.rotationAngle, enemy.rotationAxis.x, enemy.rotationAxis.y, enemy.rotationAxis.z);

    Vector3 v_top = { 0, r, 0 };
    Vector3 v_bottom = { 0, -r, 0 };
    Vector3 v_right = { r, 0, 0 };
    Vector3 v_left = { -r, 0, 0 };
    Vector3 v_front = { 0, 0, r * 2 }; // 2x longer
    Vector3 v_back = { 0, 0, -r * 2 }; // 2x longer

    // Main body
    DrawLine3D(v_front, v_top, enemy.color);
    DrawLine3D(v_front, v_bottom, enemy.color);
    DrawLine3D(v_front, v_left, enemy.color);
    DrawLine3D(v_front, v_right, enemy.color);

    DrawLine3D(v_back, v_top, enemy.color);
    DrawLine3D(v_back, v_bottom, enemy.color);
    DrawLine3D(v_back, v_left, enemy.color);
    DrawLine3D(v_back, v_right, enemy.color);

    DrawLine3D(v_top, v_right, enemy.color);
    DrawLine3D(v_right, v_bottom, enemy.color);
    DrawLine3D(v_bottom, v_left, enemy.color);
    DrawLine3D(v_left, v_top, enemy.color);

    // Fins
    Vector3 fin_top_back = { 0, fin_r, -r * 2 - fin_r };
    Vector3 fin_bottom_back = { 0, -fin_r, -r * 2 - fin_r };
    Vector3 fin_left_back = { -fin_r, 0, -r * 2 - fin_r };
    Vector3 fin_right_back = { fin_r, 0, -r * 2 - fin_r };

    DrawLine3D(v_top, fin_top_back, enemy.color);
    DrawLine3D(v_back, fin_top_back, enemy.color);

    DrawLine3D(v_bottom, fin_bottom_back, enemy.color);
    DrawLine3D(v_back, fin_bottom_back, enemy.color);

    DrawLine3D(v_left, fin_left_back, enemy.color);
    DrawLine3D(v_back, fin_left_back, enemy.color);

    DrawLine3D(v_right, fin_right_back, enemy.color);
    DrawLine3D(v_back, fin_right_back, enemy.color);

    rlPopMatrix();
}
