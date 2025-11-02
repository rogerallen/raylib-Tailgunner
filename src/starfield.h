
#ifndef STARFIELD_H
#define STARFIELD_H

#include "raylib.h"

#define MAX_STARS 500

typedef struct Star {
    Vector3 position;
    Color color;
} Star;

extern Star stars[MAX_STARS];

void InitStarfield(void);
void UpdateStarfield(void);
void DrawStarfield(void);

#endif // STARFIELD_H
