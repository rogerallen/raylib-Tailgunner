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

#include "raylib.h"
#include "config.h"

// Force field state machine states
typedef enum {
    FF_STATE_READY,     // Can be activated
    FF_STATE_ACTIVE,    // Currently repelling enemies
    FF_STATE_COOLDOWN   // Recharging after use
} ForceFieldState;

// Global force field state (consider making this an opaque struct in future)
extern ForceFieldState ff_state;
extern float ff_charge;  // Current charge level (0.0f to 1.0f)

//----------------------------------------------------------------------------------
// Force Field Module Functions
//----------------------------------------------------------------------------------

/**
 * Initialize the force field system
 */
void InitForceField(void);

/**
 * Update force field state and handle enemy repulsion
 *
 * @param forceFieldHitSound Sound to play when enemies are repelled
 */
void UpdateForceField(Sound forceFieldHitSound);

/**
 * Attempt to activate the force field
 *
 * @param forceFieldSound Sound to play on successful activation
 * @param forceFailSound Sound to play if activation fails (cooling down)
 */
void ActivateForceField(Sound forceFieldSound, Sound forceFailSound);

/**
 * Draw the 2D force field grid effect when active
 */
void DrawForceField2D(void);

/**
 * Draw the force field UI showing charge status
 */
void DrawForceFieldUI(void);

#endif // FORCEFIELD_H
