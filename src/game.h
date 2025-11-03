
/*******************************************************************************************
*
*   game.h - Game state management for Tailgunner
*   
*   Defines the main game states and transitions between them.
*
*******************************************************************************************/

#ifndef GAME_H
#define GAME_H

// Game state machine states
typedef enum GameState {
    STATE_START,     // Title screen, waiting for player to start
    STATE_PLAYING,   // Main gameplay
    STATE_GAME_OVER  // Game over screen showing final score
} GameState;

#endif // GAME_H
