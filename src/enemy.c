//================================================================================================
//
//   enemy.c - Enemy implementation
//
//   See enemy.h for module interface documentation.
//
//================================================================================================

#include "enemy.h"
#include "config.h"
#include "raymath.h"
#include "rlgl.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

//----------------------------------------------------------------------------------
// Internal Function Declarations
//----------------------------------------------------------------------------------

static Vector3 GetCubicBezierTangent(Vector3 p0, Vector3 p1, Vector3 p2, Vector3 p3, float t);
static Vector3 GetCubicBezierPoint(Vector3 p0, Vector3 p1, Vector3 p2, Vector3 p3, float t);

//----------------------------------------------------------------------------------
// Public Function Implementations
//----------------------------------------------------------------------------------

void InitEnemies(EnemyManager *mgr)
{
    for (int i = 0; i < WAVE_SIZE; i++) {
        mgr->enemies[i].active = false;
        mgr->enemies[i].radius = ENEMY_DEFAULT_RADIUS;
        mgr->enemies[i].color = COLOR_ENEMY;
        mgr->enemies[i].state = ENEMY_STATE_NORMAL;
        mgr->enemies[i].rotationAxis = (Vector3){0.0f, 1.0f, 0.0f};
        mgr->enemies[i].rotationAngle = 0.0f;
    }

    mgr->enemyShader = LoadShader(NULL, NULL);

    // Create the enemy mesh
    const float r = ENEMY_DEFAULT_RADIUS;
    const float fin_r = 1.0f;

    Vector3 vertices[] = {
        // Main body
        {0, 0, r * 2}, {0, r, 0}, 
        {0, 0, r * 2}, {0, -r, 0},
        {0, 0, r * 2}, {-r, 0, 0},
        {0, 0, r * 2}, {r, 0, 0},

        {0, 0, -r * 2}, {0, r, 0},
        {0, 0, -r * 2}, {0, -r, 0},
        {0, 0, -r * 2}, {-r, 0, 0},
        {0, 0, -r * 2}, {r, 0, 0},

        {0, r, 0}, {r, 0, 0},
        {r, 0, 0}, {0, -r, 0},
        {0, -r, 0}, {-r, 0, 0},
        {-r, 0, 0}, {0, r, 0},

        // Fins
        {0, r, 0}, {0, fin_r, -r * 2 - fin_r},
        {0, 0, -r * 2}, {0, fin_r, -r * 2 - fin_r},

        {0, -r, 0}, {0, -fin_r, -r * 2 - fin_r},
        {0, 0, -r * 2}, {0, -fin_r, -r * 2 - fin_r},

        {-r, 0, 0}, {-fin_r, 0, -r * 2 - fin_r},
        {0, 0, -r * 2}, {-fin_r, 0, -r * 2 - fin_r},

        {r, 0, 0}, {fin_r, 0, -r * 2 - fin_r},
        {0, 0, -r * 2}, {fin_r, 0, -r * 2 - fin_r},
    };

    mgr->enemyMesh = (Mesh){0};
    mgr->enemyMesh.vertexCount = 40;
    mgr->enemyMesh.vertices = (float *)RL_MALLOC(mgr->enemyMesh.vertexCount * 3 * sizeof(float));
    memcpy(mgr->enemyMesh.vertices, vertices, mgr->enemyMesh.vertexCount * 3 * sizeof(float));
    mgr->enemyMesh.vboId = (unsigned int *)RL_CALLOC(1, sizeof(unsigned int));
    mgr->enemyMesh.vboId[0] = rlLoadVertexBuffer(mgr->enemyMesh.vertices, mgr->enemyMesh.vertexCount * 3 * sizeof(float), false);
    mgr->enemyMesh.vaoId = rlLoadVertexArray();
    rlEnableVertexArray(mgr->enemyMesh.vaoId);
    rlEnableVertexAttribute(0);
    rlSetVertexAttribute(0, 3, RL_FLOAT, 0, 0, 0);
    rlDisableVertexArray();
}

void SpawnWave(EnemyManager *mgr, int wave)
{
    for (int i = 0; i < WAVE_SIZE; i++) {
        Enemy *e = &mgr->enemies[i];
        e->active = true;
        e->state = ENEMY_STATE_NORMAL;
        e->t = 0.0f;
        e->rotationAngle = 0.0f;
        e->rotationAxis = (Vector3){0.0f, 1.0f, 0.0f};

        float zOffset = i * ENEMY_Z_OFFSET;
        int side = (i % 2 == 0) ? 1 : -1;
        int r0x = GetRandomValue(-ENEMY_XY_START_RANGE, ENEMY_XY_START_RANGE);
        int r0y = GetRandomValue(-ENEMY_XY_START_RANGE, ENEMY_XY_START_RANGE);

        e->p0 = (Vector3){(float)r0x, (float)r0y, -100.0f - zOffset};
        e->p1 = (Vector3){(float)GetRandomValue(-5, 5), (float)GetRandomValue(-5, 5), -50.0f - zOffset / 2.0f};
        e->p2 = (Vector3){(float)GetRandomValue(-40, -20) * side, (float)GetRandomValue(10, 20), -25.0f};
        e->p3 = (Vector3){(float)GetRandomValue(20, 40) * side, (float)GetRandomValue(-20, -10), 1.0f};

        if (wave <= WAVE_NERF2_LEVELS) {
            if (i >= WAVE_SIZE - 2) {
                e->active = false;
            }
        }
        else if (wave <= WAVE_NERF1_LEVELS) {
            if (i >= WAVE_SIZE - 1) {
                e->active = false;
            }
        }
    }
}

void UpdateEnemies(EnemyManager *mgr, int *lives, int *wave)
{
    int activeEnemies = 0;

    for (int i = 0; i < WAVE_SIZE; i++) {
        Enemy *e = &mgr->enemies[i];
        if (e->active) {
            activeEnemies++;

            switch (e->state) {
            case ENEMY_STATE_NORMAL: {
                e->t += ENEMY_DT_DFRAME + (*wave * ENEMY_WAVE_DT_DFRAME);
                e->position = GetCubicBezierPoint(e->p0, e->p1, e->p2, e->p3, e->t);

                if (e->t >= 1.0f) {
                    e->active = false;
                    (*lives)--;
                }
            } break;
            case ENEMY_STATE_REPELLED: {
                e->repel_t += ENEMY_REPEL_DT_DFRAME;
                e->position = Vector3Lerp(e->repel_start_pos, e->p0, e->repel_t);
                e->rotationAngle += 360.0f * GetFrameTime();

                if (e->repel_t >= 1.0f) {
                    e->state = ENEMY_STATE_NORMAL;
                    e->t = 0.0f;
                    e->rotationAngle = 0.0f;
                }
            } break;
            }
        }
    }

    if (activeEnemies == 0) {
        (*wave)++;
        SpawnWave(mgr, *wave);
    }
}

void DrawEnemies(EnemyManager *mgr)
{
    int mvpLoc = GetShaderLocation(mgr->enemyShader, "mvp");

    rlSetLineWidth(2.0f);
    rlEnableShader(mgr->enemyShader.id);

    for (int i = 0; i < WAVE_SIZE; i++) {
        if (mgr->enemies[i].active) {
            Enemy *enemy = &mgr->enemies[i];
            
            Vector3 forward;
            if (enemy->state == ENEMY_STATE_REPELLED) {
                Vector3 to_p0 = Vector3Subtract(enemy->p0, enemy->position);
                if (Vector3LengthSqr(to_p0) > 0.0001f) {
                    forward = Vector3Normalize(to_p0);
                }
                else {
                    forward = GetCubicBezierTangent(enemy->p0, enemy->p1, enemy->p2, enemy->p3, 0.0f);
                }
            }
            else {
                forward = GetCubicBezierTangent(enemy->p0, enemy->p1, enemy->p2, enemy->p3, enemy->t);
                if (Vector3LengthSqr(forward) < 0.0001f) {
                    forward = Vector3Normalize(Vector3Negate(enemy->position));
                }
            }

            Vector3 up = {0.0f, 1.0f, 0.0f};
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

            Matrix mvp = MatrixMultiply(rlGetMatrixModelview(), rlGetMatrixProjection());
            SetShaderValueMatrix(mgr->enemyShader, mvpLoc, mvp);
            
            rlEnableVertexArray(mgr->enemyMesh.vaoId);
            rlColor4ub(enemy->color.r, enemy->color.g, enemy->color.b, enemy->color.a);
            rlDrawVertexArray(0, mgr->enemyMesh.vertexCount);
            rlDisableVertexArray();

            rlPopMatrix();
        }
    }
    rlDisableShader();
    rlSetLineWidth(1.0f);
}

//----------------------------------------------------------------------------------
// Internal Function Implementations
//----------------------------------------------------------------------------------

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

    float len = Vector3Length(result);
    if (len < 1e-6f) return (Vector3){0.0f, 0.0f, 0.0f};
    return Vector3Scale(result, 1.0f / len);
}
