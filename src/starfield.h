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

// Star definition
typedef struct Star {
    Vector3 position; // Star position in 3D space
    Color color;      // Star color including alpha for depth effect
} Star;

// Global star array (consider making this an opaque pointer in future)
extern Star stars[MAX_STARS];

//----------------------------------------------------------------------------------
// Starfield Module Functions
//----------------------------------------------------------------------------------

// Initialize the star field, randomly placing all stars
void InitStarfield(void);

// Update star positions, moving them toward camera and wrapping
void UpdateStarfield(void);

// Render all stars in 3D space
void DrawStarfield(void);

#endif // STARFIELD_H
