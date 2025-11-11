#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#include "raylib.h"

#define MAX_NAME_LENGTH 3

typedef struct {
    char name[MAX_NAME_LENGTH + 1];
    int score;
} LeaderboardEntry;

void InitLeaderboard();
void UpdateLeaderboard(int *gameState, int score);
void DrawLeaderboard();

void FetchGlobalTop10();
void FetchUserTop10(const char* name);

bool IsLeaderboardActive();
void SetLeaderboardActive(bool active);
void ResetLeaderboardFlags();
void RequestLeaderboardUpdate();

#endif // LEADERBOARD_H
