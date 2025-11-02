
#ifndef ENEMY_H
#define ENEMY_H

#include "raylib.h"

#define MAX_ENEMIES 50

typedef struct Enemy {
    Vector3 position;
    bool active;
    Color color;
    float radius;

    Vector3 p0, p1, p2, p3; // Bezier curve points
    float t; // Progress along the curve
} Enemy;

extern Enemy enemies[MAX_ENEMIES];

void InitEnemies(void);
int UpdateEnemies(Sound explosionSound, int* lives);
void DrawEnemies(void);

#endif // ENEMY_H
