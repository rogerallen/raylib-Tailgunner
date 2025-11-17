#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#include "config.h"
#include "raylib.h"

typedef struct {
    char name[LEADERBOARD_NAME_LENGTH + 1];
    int score;
} LeaderboardEntry;

// Encapsulated leaderboard manager to avoid file-level globals.
typedef struct LeaderboardManager {
    LeaderboardEntry globalTop10[LEADERBOARD_MAX_SCORES];
    LeaderboardEntry userTop10[LEADERBOARD_MAX_SCORES];
    char playerName[LEADERBOARD_NAME_LENGTH + 1];

    bool isActive;
    bool scoreSubmitted;
    bool globalScoresFetched;
    bool userScoresFetched;
    bool globalScoresFetching;
    bool userScoresFetching;
    bool requestUpdate;

    Rectangle upArrows[LEADERBOARD_NAME_LENGTH];
    Rectangle downArrows[LEADERBOARD_NAME_LENGTH];
    Rectangle charBoxes[LEADERBOARD_NAME_LENGTH];
    Rectangle submitButton;
} LeaderboardManager;

// Public Functions

void DrawLeaderboard(LeaderboardManager *mgr);
void DrawNameInput(LeaderboardManager *mgr);
void InitLeaderboard(LeaderboardManager *mgr);
void ResetLeaderboardFlags(LeaderboardManager *mgr);
void SetLeaderboardActive(LeaderboardManager *mgr, bool active);
void SubmitScore(LeaderboardManager *mgr, int score);
void UpdateLeaderboard(LeaderboardManager *mgr, int *gameState, int score);
bool UpdateNameInput(LeaderboardManager *mgr);

#endif // LEADERBOARD_H
