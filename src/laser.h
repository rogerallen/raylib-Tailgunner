#ifndef LASER_H
#define LASER_H

#include "raylib.h"

#define MAX_LASERS 2

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
