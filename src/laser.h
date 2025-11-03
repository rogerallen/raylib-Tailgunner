/*******************************************************************************************
*
*   laser.h - Laser weapon system for Tailgunner
*   
*   Manages player's laser shots, including firing, hit detection, and rendering.
*   Lasers are rendered as lines in 3D space and can hit enemies.
*
*******************************************************************************************/

#ifndef LASER_H
#define LASER_H

#include "raylib.h"
#include "config.h"

// Laser beam definition
typedef struct Laser {
    Vector3 start;    // Beam start position in world space
    Vector3 end;      // Beam end position in world space
    bool active;      // Whether this laser is currently firing
    float lifeTime;   // Remaining time before beam dissipates
    Color color;      // Beam color
} Laser;

// Global laser array (consider making this an opaque pointer in future)
extern Laser lasers[MAX_LASERS];

//----------------------------------------------------------------------------------
// Laser Module Functions
//----------------------------------------------------------------------------------

/**
 * Initialize the laser system, setting all lasers to inactive
 */
void InitLasers(void);

/**
 * Fire lasers from camera along the given ray, checking for enemy hits
 *
 * @param ray Ray representing the shot direction
 * @param camera Current camera for shot origin
 * @param explosionSound Sound to play on successful hit
 * @return Number of enemies hit by this shot
 */
int FireLasers(Ray ray, Camera camera, Sound explosionSound);

/**
 * Update all active lasers, handling lifetime and deactivation
 */
void UpdateLasers(void);

/**
 * Render all active laser beams in 3D space
 */
void DrawLasers(void);

#endif // LASER_H
