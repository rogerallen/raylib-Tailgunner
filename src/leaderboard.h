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

// DrawLeaderboard - Implementation Notes:
// - Renders the global top 10 and the user's best scores when available
// - Shows a loading message while data is being fetched
// - Centers and formats text for the current screen resolution
void DrawLeaderboard(const LeaderboardManager *mgr);

// DrawNameInput - Implementation Notes:
// - Renders character input boxes for player initials
// - Draws up/down arrows for selecting characters and a submit button
void DrawNameInput(const LeaderboardManager *mgr);

// InitLeaderboard - Implementation Notes:
// - Initializes UI rectangles, default player initials, and state flags
// - Loads saved player name from persistent storage (platform-specific)
void InitLeaderboard(LeaderboardManager *mgr);

// ResetLeaderboardFlags - Implementation Notes:
// - Clears fetched/submitted flags so the UI will request fresh data
void ResetLeaderboardFlags(LeaderboardManager *mgr);

// SetLeaderboardActive - Implementation Notes:
// - Sets the active/inactive state of the leaderboard UI
// - Caller can use this to show or hide the leaderboard
// @param active True to activate leaderboard, false to deactivate
void SetLeaderboardActive(LeaderboardManager *mgr, bool active);

// SubmitScore - Implementation Notes:
// - Submits the provided score to the remote leaderboard service
// - Uses platform-specific HTTP code (emscripten fetch on web, libcurl on native)
// - On success, marks scoreSubmitted and requests a leaderboard update
// @param score The score value to submit
void SubmitScore(LeaderboardManager *mgr, int score);

// UpdateLeaderboard - Implementation Notes:
// - Polls for updates and triggers network fetches for global/user scores
// - Handles input to close the leaderboard (Enter or click)
void UpdateLeaderboard(LeaderboardManager *mgr, int *gameState);

// UpdateNameInput - Implementation Notes:
// - Handles mouse interaction with character boxes and submit button
// - Returns true when the player clicks Submit (caller may act on it)
bool UpdateNameInput(LeaderboardManager *mgr);

#endif // LEADERBOARD_H
