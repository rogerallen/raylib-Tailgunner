#ifndef CONFIG_H
#define CONFIG_H

// Gameplay tuning constants

// Number of enemies spawned per wave
#define WAVE_SIZE 3

// Default enemy radius
#define ENEMY_DEFAULT_RADIUS 1.0f

// Laser lifetime in seconds when fired
#define LASER_LIFETIME 0.2f

// Laser forward start offset from camera (to avoid near-plane clipping)
#define LASER_START_FORWARD_OFFSET 0.5f

// Maximum counts
#define MAX_ENEMIES 50
#define MAX_LASERS 2
#define MAX_STARS 500

// Force field tuning
#define FORCE_FIELD_TIMEOUT 10.0f
#define FORCE_FIELD_ACTIVE_TIME 1.0f
#define FORCE_FIELD_RADIUS 10.0f

#endif // CONFIG_H
