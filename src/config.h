#ifndef CONFIG_H
#define CONFIG_H

// Version for display in title screen
#define GAME_VERSION "v1.1"
#define GAME_TITLE "TAILGUNNER"

// Leaderboard configuration
// Game ID is 19 for Tailgunner, 21 for TailgunnerProd(uction)
// !!! Set to 21 for production releases !!!
#define LEADERBOARD_GAME_ID 21
#define LEADERBOARD_BASE_URL "https://geraldburke.com/apis/simple-leaderboard-api/"
#define LEADERBOARD_MAX_SCORES 10
#define LEADERBOARD_NAME_LENGTH 3

// Gameplay tuning constants

// Number of enemies spawned per wave
#define WAVE_SIZE 5
// Let's nerf 2 enemies in the first 3 levels, and 1 enemy in the next 4 levels
#define WAVE_NERF2_LEVELS 3
#define WAVE_NERF1_LEVELS 7

// Default enemy radius
#define ENEMY_DEFAULT_RADIUS  1.0f
#define ENEMY_Z_OFFSET       20.0f
#define ENEMY_XY_START_RANGE 30

// Laser lifetime in seconds when fired
#define LASER_LIFETIME 0.2f

// Laser forward start offset from camera (to avoid near-plane clipping)
#define LASER_START_FORWARD_OFFSET 0.5f

// Enemy movement speeds
#define ENEMY_DT_DFRAME       0.0025f  // Base speed per frame along Bezier curve
#define ENEMY_WAVE_DT_DFRAME  0.00015f // Additional speed per wave (wave difficulty scaling)
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
#define COLOR_TEXT_LEADERBOARD           ?
#define COLOR_CROSSHAIR_CIRCLE           MAROON
#define COLOR_CROSSHAIR_LINES            RED
#define COLOR_LASER                      RED
#define COLOR_ENEMY                      BLUE
#define COLOR_STAR                       WHITE
#define COLOR_FORCEFIELD_GRID            LIME
#define COLOR_FORCEFIELD_UI_READY        GREEN
#define COLOR_FORCEFIELD_UI_CHARGING     YELLOW
#define COLOR_INITIAL_BOX                ?
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
#define COLOR_TEXT_LEADERBOARD           VGREEN
#define COLOR_CROSSHAIR_CIRCLE           VGREEN
#define COLOR_CROSSHAIR_LINES            VGREEN
#define COLOR_LASER                      VGREEN
#define COLOR_ENEMY                      VGREEN
#define COLOR_STAR                       VGREEN
#define COLOR_FORCEFIELD_GRID            VGREEN
#define COLOR_FORCEFIELD_UI_READY        VGREEN
#define COLOR_FORCEFIELD_UI_CHARGING     VGREEN
#define COLOR_INITIAL_BOX                VGREEN
#endif


#endif // CONFIG_H
