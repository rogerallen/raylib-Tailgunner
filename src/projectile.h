
#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "raylib.h"

#define MAX_PROJECTILES 100

typedef struct Projectile {
    Vector3 position;
    Vector3 speed;
    bool active;
    Color color;
    float radius;
} Projectile;

extern Projectile projectiles[MAX_PROJECTILES];

void InitProjectiles(void);
void ShootProjectile(Ray ray);
void UpdateProjectiles(void);
void DrawProjectiles(void);

#endif // PROJECTILE_H
