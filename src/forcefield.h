//================================================================================================
//
//   forcefield.h - Force field defense system for Tailgunner
//
//   Manages the player's force field ability which can repel incoming enemies.
//   Includes state management, visual effects, and enemy repulsion logic.
//
//================================================================================================

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

// Initialize the force field system
void InitForceField(ForceFieldManager *mgr);

// Attempt to activate the force field
//
// Returns true on success, false if cooldown is still active.
// Caller should handle playing success/fail sounds based on return value.
bool ActivateForceField(ForceFieldManager *mgr);

// Update force field state and handle enemy repulsion
//
// Returns true if any enemies were repelled this frame (caller may play hit sound).
bool UpdateForceField(ForceFieldManager *mgr, struct EnemyManager *emgr);

// Draw the 2D force field grid effect when active
void DrawForceField2D(ForceFieldManager *mgr);

// Draw the force field UI showing charge status
void DrawForceFieldUI(ForceFieldManager *mgr);

#endif // FORCEFIELD_H
