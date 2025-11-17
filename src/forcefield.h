/*******************************************************************************************
 *
 *   forcefield.h - Force field defense system for Tailgunner
 *
 *   Manages the player's force field ability which can repel incoming enemies.
 *   Includes state management, visual effects, and enemy repulsion logic.
 *
 *******************************************************************************************/

#ifndef FORCEFIELD_H
#define FORCEFIELD_H

#include "config.h"
#include "raylib.h"
// Need EnemyManager definition to operate on enemies
#include "enemy.h"

// Force field state machine states
typedef enum {
    FF_STATE_READY,   // Can be activated
    FF_STATE_ACTIVE,  // Currently repelling enemies
    FF_STATE_COOLDOWN // Recharging after use
} ForceFieldState;

// Encapsulated manager to avoid globals
typedef struct ForceFieldManager {
    ForceFieldState state;
    float charge; // Current charge level (0.0f to 1.0f)
    float timer;  // Active or cooldown timer
} ForceFieldManager;

//----------------------------------------------------------------------------------
// Force Field Module Functions
//----------------------------------------------------------------------------------

/**
 * Initialize the force field system
 */
void InitForceField(ForceFieldManager *mgr);

/**
 * Update force field state and handle enemy repulsion
 *
 * @param forceFieldHitSound Sound to play when enemies are repelled
 */
// Update will need access to enemies in order to apply repulsion
// Update returns true if any enemies were repelled this frame (caller may play a sound)
bool UpdateForceField(ForceFieldManager *mgr, struct EnemyManager *emgr);

/**
 * Attempt to activate the force field
 *
 * @param forceFieldSound Sound to play on successful activation
 * @param forceFailSound Sound to play if activation fails (cooling down)
 */
// Attempt to activate the force field. Returns true on success, false if activation failed
// (caller may play success/fail sounds).
bool ActivateForceField(ForceFieldManager *mgr);

/**
 * Draw the 2D force field grid effect when active
 */
void DrawForceField2D(ForceFieldManager *mgr);

/**
 * Draw the force field UI showing charge status
 */
void DrawForceFieldUI(ForceFieldManager *mgr);

#endif // FORCEFIELD_H
