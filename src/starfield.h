//================================================================================================
//
//   starfield.h - Background star field effect for Tailgunner
//
//   Manages a scrolling field of stars to create parallax space effect.
//   Stars are rendered as small spheres that scroll toward the camera.
//
//================================================================================================

#ifndef STARFIELD_H
#define STARFIELD_H

#include "config.h"
#include "raylib.h"

typedef struct Starfield {
    Mesh mesh;
    Material material;
    Matrix *transforms;
    Vector3 *positions;
} Starfield;

extern Starfield starfield;

//----------------------------------------------------------------------------------
// Starfield Module Functions
//----------------------------------------------------------------------------------

// Initialize the star field, randomly placing all stars
void InitStarfield(void);

// Unload star field data and resources
void UnloadStarfield(void);

// Update star positions, moving them toward camera and wrapping
void UpdateStarfield(void);

// Render all stars in 3D space
void DrawStarfield(void);

#endif // STARFIELD_H
