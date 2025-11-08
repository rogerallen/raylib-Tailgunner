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
#define ENEMY_REPEL_DT_DFRAME 0.01f    // Speed when repelled

// Maximum counts

#define MAX_LASERS 2
#define MAX_STARS 500

// Force field tuning
#define FORCE_FIELD_TIMEOUT     10.0f
#define FORCE_FIELD_ACTIVE_TIME  1.0f
#define FORCE_FIELD_RADIUS      10.0f

// Color definitions
#if 0
#define COLOR_BACKGROUND                 BLACK
#define COLOR_TEXT_TITLE                 LIGHTGRAY
#define COLOR_TEXT_SUBTITLE              GRAY
#define COLOR_TEXT_SCORE                 GREEN
#define COLOR_TEXT_LIVES                 RED
#define COLOR_TEXT_WAVE                  YELLOW
#define COLOR_TEXT_GAMEOVER              RED
#define COLOR_TEXT_FINAL_SCORE           GREEN
#define COLOR_CROSSHAIR_CIRCLE           MAROON
#define COLOR_CROSSHAIR_LINES            RED
#define COLOR_LASER                      RED
#define COLOR_ENEMY                      BLUE
#define COLOR_STAR                       WHITE
#define COLOR_FORCEFIELD_GRID            LIME
#define COLOR_FORCEFIELD_UI_READY        GREEN
#define COLOR_FORCEFIELD_UI_CHARGING     YELLOW
#else
#define VGREEN CLITERAL(Color){ 0x30, 0xf0, 0xe0, 0xff }
#define COLOR_BACKGROUND                 BLACK
#define COLOR_TEXT_TITLE                 VGREEN
#define COLOR_TEXT_SUBTITLE              VGREEN
#define COLOR_TEXT_SCORE                 VGREEN
#define COLOR_TEXT_LIVES                 VGREEN
#define COLOR_TEXT_WAVE                  VGREEN
#define COLOR_TEXT_GAMEOVER              VGREEN
#define COLOR_TEXT_FINAL_SCORE           VGREEN
#define COLOR_CROSSHAIR_CIRCLE           VGREEN
#define COLOR_CROSSHAIR_LINES            VGREEN
#define COLOR_LASER                      VGREEN
#define COLOR_ENEMY                      VGREEN
#define COLOR_STAR                       VGREEN
#define COLOR_FORCEFIELD_GRID            VGREEN
#define COLOR_FORCEFIELD_UI_READY        VGREEN
#define COLOR_FORCEFIELD_UI_CHARGING     VGREEN
#endif


#endif // CONFIG_H
