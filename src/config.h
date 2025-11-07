#ifndef CONFIG_H
#define CONFIG_H

// Gameplay tuning constants

// Number of enemies spawned per wave
#define WAVE_SIZE 5

// Default enemy radius
#define ENEMY_DEFAULT_RADIUS 1.0f

// Laser lifetime in seconds when fired
#define LASER_LIFETIME 0.2f

// Laser forward start offset from camera (to avoid near-plane clipping)
#define LASER_START_FORWARD_OFFSET 0.5f

// Enemy movement speeds
#define ENEMY_DT_DFRAME       0.0025f  // Base speed per frame along Bezier curve
#define ENEMY_WAVE_DT_DFRAME  0.00025f // Additional speed per wave (wave difficulty scaling)
#define ENEMY_REPEL_DT_DFRAME 0.01f   // Speed when repelled

// Maximum counts

#define MAX_LASERS 2
#if defined(PLATFORM_WEB)
#define MAX_STARS 100
#else
#define MAX_STARS 500
#endif

// Force field tuning
#define FORCE_FIELD_TIMEOUT 10.0f
#define FORCE_FIELD_ACTIVE_TIME 1.0f
#define FORCE_FIELD_RADIUS 10.0f

#endif // CONFIG_H
