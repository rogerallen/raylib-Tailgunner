#ifndef ENEMY_H
#define ENEMY_H

#include "raylib.h"
#include "config.h"

typedef enum {
    ENEMY_STATE_NORMAL,
    ENEMY_STATE_REPELLED
} EnemyState;

typedef struct Enemy {
    Vector3 position;
    bool active;
    Color color;
    float radius;

    Vector3 p0, p1, p2, p3; // Bezier curve points
    float t; // Progress along the curve

    EnemyState state;
    Vector3 repel_start_pos;
    float repel_t;
    Vector3 rotationAxis;
    float rotationAngle;
} Enemy;

extern Enemy enemies[MAX_ENEMIES];

void InitEnemies(void);
void UpdateEnemies(int* lives, int* wave);
void DrawEnemies(void);
void SpawnWave(int wave);

#endif // ENEMY_H