//================================================================================================
//
//   enemy.h - Enemy management and rendering for Tailgunner
//
//   Handles enemy spawning, movement along Bezier curves, state management, and rendering.
//   Enemies follow curved paths and can be repelled by the force field.
//
//================================================================================================

#ifndef ENEMY_H
#define ENEMY_H

#include "config.h"
#include "raylib.h"

// Enemy state machine states
typedef enum {
    ENEMY_STATE_NORMAL,  // Moving along Bezier curve path
    ENEMY_STATE_REPELLED // Being pushed back by force field
} EnemyState;

// Enemy entity definition
typedef struct Enemy {
    Vector3 position; // Current world position
    bool active;      // Whether this enemy is currently in play
    Color color;      // Enemy's render color
    float radius;     // Collision and rendering radius

    Vector3 p0, p1, p2, p3; // Bezier curve control points
    float t;                // Progress (0-1) along the curve

    EnemyState state;        // Current state in state machine
    Vector3 repel_start_pos; // Position when repel started
    float repel_t;           // Progress (0-1) of repel motion
    Vector3 rotationAxis;    // Axis for spin animation
    float rotationAngle;     // Current spin angle (degrees)
} Enemy;

// Opaque manager to avoid globals. Keep the array inside this struct so callers
// can allocate or pass around manager instances instead of using a global.
typedef struct EnemyManager {
    Enemy enemies[WAVE_SIZE];
} EnemyManager;

//----------------------------------------------------------------------------------
// Enemy Module Functions
//----------------------------------------------------------------------------------

// Initialize the enemy system, resetting all enemies to inactive state
void InitEnemies(EnemyManager *mgr);

// Update all active enemies, handling movement and state changes
//
// @param lives Pointer to player's life count, decremented when enemies escape
// @param wave Pointer to current wave number, incremented when wave is cleared
void UpdateEnemies(EnemyManager *mgr, int *lives, int *wave);

// Render all active enemies in 3D space
void DrawEnemies(EnemyManager *mgr);

// Spawn a new wave of enemies with curved attack paths
//
// @param wave Current wave number (affects enemy movement speed)
void SpawnWave(EnemyManager *mgr, int wave);

#endif // ENEMY_H
