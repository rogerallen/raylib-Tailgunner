
#ifndef ENEMY_H
#define ENEMY_H

#include "raylib.h"

#define MAX_ENEMIES 50

typedef struct Enemy {
    Vector3 position;
    Vector3 speed;
    bool active;
    Color color;
    float radius;
} Enemy;

extern Enemy enemies[MAX_ENEMIES];

void InitEnemies(void);
int UpdateEnemies(Sound explosionSound);
void DrawEnemies(void);

#endif // ENEMY_H
