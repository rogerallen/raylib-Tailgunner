#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#include "raylib.h"
#include "config.h"

typedef struct {
    char name[LEADERBOARD_NAME_LENGTH + 1];
    int score;
} LeaderboardEntry;

// Public Functions

void DrawLeaderboard();
void DrawNameInput();
void InitLeaderboard();
void ResetLeaderboardFlags();
void SetLeaderboardActive(bool active);
void SubmitScore(int score);
void UpdateLeaderboard(int *gameState, int score);
bool UpdateNameInput();

#endif // LEADERBOARD_H
