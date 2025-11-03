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
#include "enemy.h"

// Laser beam definition
typedef struct Laser {
    Vector3 start;    // Beam start position in world space
    Vector3 end;      // Beam end position in world space
    bool active;      // Whether this laser is currently firing
    float lifeTime;   // Remaining time before beam dissipates
    Color color;      // Beam color
} Laser;

// Encapsulate lasers in a manager to avoid globals
typedef struct LaserManager {
    Laser lasers[MAX_LASERS];
} LaserManager;

//----------------------------------------------------------------------------------
// Laser Module Functions
//----------------------------------------------------------------------------------

/**
 * Initialize the laser system, setting all lasers to inactive
 */
// Initialize lasers within provided manager
void InitLasers(LaserManager* mgr);

/**
 * Fire lasers from camera along the given ray, checking for enemy hits
 *
 * @param ray Ray representing the shot direction
 * @param camera Current camera for shot origin
 * @param explosionSound Sound to play on successful hit
 * @return Number of enemies hit by this shot
 */
// Fire lasers using given manager and optionally inspect enemies via EnemyManager
int FireLasers(LaserManager* lmgr, struct EnemyManager* emgr, Ray ray, Camera camera, Sound explosionSound);

/**
 * Update all active lasers, handling lifetime and deactivation
 */
void UpdateLasers(LaserManager* mgr);

/**
 * Render all active laser beams in 3D space
 */
void DrawLasers(LaserManager* mgr);

#endif // LASER_H
