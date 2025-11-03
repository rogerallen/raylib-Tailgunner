#ifndef LASER_H
#define LASER_H

#include "raylib.h"

#include "config.h"

typedef struct Laser {
    Vector3 start;
    Vector3 end;
    bool active;
    float lifeTime;
    Color color;
} Laser;

extern Laser lasers[MAX_LASERS];

void InitLasers(void);
int FireLasers(Ray ray, Camera camera, Sound explosionSound);
void UpdateLasers(void);
void DrawLasers(void);

#endif // LASER_H
