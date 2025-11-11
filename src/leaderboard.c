#include "leaderboard.h"
#include "game.h"
#include "config.h"
#include "cJSON.h"
#include <stdio.h>
#include <string.h>

#define BASE_LEADERBOARD_URL "https://geraldburke.com/apis/simple-leaderboard-api/"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#include <emscripten/fetch.h>
#include <emscripten/html5.h> // For local storage
#include <stdlib.h> // For malloc

EM_JS(char*, emscripten_local_storage_get_item_js, (const char* key_ptr), {
  var key = UTF8ToString(key_ptr);
  var value = localStorage.getItem(key);
  if (value === null) {
    return 0; // Return null pointer if item not found
  }
  var length = lengthBytesUTF8(value) + 1;
  var value_ptr = _malloc(length);
  stringToUTF8(value, value_ptr, length);
  return value_ptr;
});

EM_JS(int, emscripten_local_storage_set_item_js, (const char* key_ptr, const char* value_ptr), {
  var key = UTF8ToString(key_ptr);
  var value = UTF8ToString(value_ptr);
  try {
    localStorage.setItem(key, value);
    return 0; // Success
  } catch (e) {
    return 1; // Failure
  }
});

#endif

#define MAX_SCORES 10
#define GAME_ID 19
#define PLAYER_NAME_STORAGE_KEY "tailgunner_player_name"

static LeaderboardEntry globalTop10[MAX_SCORES];
static LeaderboardEntry userTop10[MAX_SCORES];
static char playerName[MAX_NAME_LENGTH + 1] = { 'A', 'A', 'A', '\0' };
static int charIndex = 0;

static bool isLeaderboardActive = false;
static bool scoreSubmitted = false;
static bool globalScoresFetched = false;
static bool userScoresFetched = false;
static bool globalScoresFetching = false;
static bool userScoresFetching = false;
static bool requestLeaderboardUpdate = false;

static Rectangle upArrows[MAX_NAME_LENGTH];
static Rectangle downArrows[MAX_NAME_LENGTH];
static Rectangle charBoxes[MAX_NAME_LENGTH];
static Rectangle submitButton;

static void ParseLeaderboardData(emscripten_fetch_t *fetch, LeaderboardEntry *entries, bool *fetchedFlag, bool *fetchingFlag)
{
    memset(entries, 0, sizeof(LeaderboardEntry) * MAX_SCORES);
    cJSON *json = cJSON_ParseWithLength(fetch->data, fetch->numBytes);
    if (json == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }
        *fetchedFlag = true; // Mark as fetched to avoid continuous retries on parse error
        *fetchingFlag = false;
        return;
    }

    int count = cJSON_GetArraySize(json);
    int lastScore = -1;
    int entryIndex = 0;
    for (int i = 0; i < count && entryIndex < MAX_SCORES; i++)
    {
        cJSON *item = cJSON_GetArrayItem(json, i);
        cJSON *name = cJSON_GetObjectItemCaseSensitive(item, "userName");
        cJSON *score = cJSON_GetObjectItemCaseSensitive(item, "score");

        if (cJSON_IsString(name) && (name->valuestring != NULL) && cJSON_IsNumber(score))
        {
            if ((int)score->valuedouble != lastScore)
            {
                strncpy(entries[entryIndex].name, name->valuestring, MAX_NAME_LENGTH);
                entries[entryIndex].name[MAX_NAME_LENGTH] = '\0';
                entries[entryIndex].score = (int)score->valuedouble;
                lastScore = entries[entryIndex].score;
                printf("Parsed: %s - %d\n", entries[entryIndex].name, entries[entryIndex].score);
                entryIndex++;
            }
        }
    }

    cJSON_Delete(json);
    *fetchedFlag = true;
    *fetchingFlag = false;
}

void onSubmitSuccess(emscripten_fetch_t *fetch) {
    printf("Score submitted successfully.\n");
    scoreSubmitted = true;
    emscripten_fetch_close(fetch);
    RequestLeaderboardUpdate();
}

void onFetchFailure(emscripten_fetch_t *fetch) {
    printf("Failed to fetch data: %s\n", fetch->statusText);
    if (strcmp(fetch->url + strlen(fetch->url) - 8, "topScores") == 0) {
        globalScoresFetching = false;
    } else {
        userScoresFetching = false;
    }
    emscripten_fetch_close(fetch);
}

void onGlobalScoresSuccess(emscripten_fetch_t *fetch) {
    printf("Global scores fetched successfully.\n");
    ParseLeaderboardData(fetch, globalTop10, &globalScoresFetched, &globalScoresFetching);
}

void onUserScoresSuccess(emscripten_fetch_t *fetch) {
    printf("User scores fetched successfully.\n");
    ParseLeaderboardData(fetch, userTop10, &userScoresFetched, &userScoresFetching);
}

void InitLeaderboard()
{
    for (int i = 0; i < MAX_NAME_LENGTH; i++)
    {
        charBoxes[i] = (Rectangle){ GetScreenWidth() / 2 - 70 + i * 50, GetScreenHeight() / 2 - 20, 40, 40 };
        upArrows[i] = (Rectangle){ charBoxes[i].x, charBoxes[i].y - 30, 40, 20 };
        downArrows[i] = (Rectangle){ charBoxes[i].x, charBoxes[i].y + 50, 40, 20 };
    }
    submitButton = (Rectangle){ GetScreenWidth() / 2 - 60, GetScreenHeight() / 2 + 80, 120, 30 };

#if defined(PLATFORM_WEB)
    char *storedName = emscripten_local_storage_get_item_js(PLAYER_NAME_STORAGE_KEY);
    if (storedName != NULL)
    {
        strncpy(playerName, storedName, MAX_NAME_LENGTH);
        playerName[MAX_NAME_LENGTH] = '\0';
        free(storedName);
    }
#endif
}

bool UpdateNameInput()
{
    for (int i = 0; i < MAX_NAME_LENGTH; i++)
    {
        if (CheckCollisionPointRec(GetMousePosition(), upArrows[i]) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            playerName[i]++;
            if (playerName[i] > 'Z') playerName[i] = 'A';
        }
        if (CheckCollisionPointRec(GetMousePosition(), downArrows[i]) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            playerName[i]--;
            if (playerName[i] < 'A') playerName[i] = 'Z';
        }
    }

    if (CheckCollisionPointRec(GetMousePosition(), submitButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
#if defined(PLATFORM_WEB)
        emscripten_local_storage_set_item_js(PLAYER_NAME_STORAGE_KEY, playerName);
#endif
        return true;
    }
    return false;
}

void DrawNameInput()
{
    DrawText("Enter Your Initials", GetScreenWidth() / 2 - MeasureText("Enter Your Initials", 30) / 2, GetScreenHeight() / 2 - 100, 30, COLOR_TEXT_TITLE);

    for (int i = 0; i < MAX_NAME_LENGTH; i++)
    {
        DrawRectangleRec(charBoxes[i], LIGHTGRAY);
        DrawText(TextFormat("%c", playerName[i]), charBoxes[i].x + 12, charBoxes[i].y + 5, 30, BLACK);
        DrawTriangle((Vector2){ upArrows[i].x + 20, upArrows[i].y }, (Vector2){ upArrows[i].x, upArrows[i].y + 20 }, (Vector2){ upArrows[i].x + 40, upArrows[i].y + 20 }, MAROON);
        DrawTriangle((Vector2){ downArrows[i].x + 20, downArrows[i].y + 20 }, (Vector2){ downArrows[i].x + 40, downArrows[i].y }, (Vector2){ downArrows[i].x, downArrows[i].y }, MAROON);
    }

    DrawRectangleRec(submitButton, LIME);
    DrawText("Submit", submitButton.x + 30, submitButton.y + 5, 20, BLACK);
}

void SubmitScore(int score)
{
#if defined(PLATFORM_WEB)
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "POST");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    attr.onsuccess = onSubmitSuccess;
    attr.onerror = onFetchFailure;

    char url[256];
    //BAD URL: sprintf(url, "https://simple-leaderboard-api.onrender.com/game/%d/leaderboard?player_name=%s&score=%d", GAME_ID, playerName, score);
    sprintf(url, "%s?action=newScore&gameID=%d&userName=%s&score=%d", BASE_LEADERBOARD_URL, GAME_ID, playerName, score);
    emscripten_fetch(&attr, url);
#else
    // No-op for non-web platforms for now
    scoreSubmitted = true;
#endif
}

void FetchGlobalTop10()
{
#if defined(PLATFORM_WEB)
    if (globalScoresFetching) return;
    globalScoresFetching = true;

    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    attr.onsuccess = onGlobalScoresSuccess;
    attr.onerror = onFetchFailure;

    char url[256];
    // BAD URL sprintf(url, "https://simple-leaderboard-api.onrender.com/game/%d/leaderboard", GAME_ID);
    sprintf(url, "%s?action=topScores&gameID=%d", BASE_LEADERBOARD_URL, GAME_ID);  // &count=10 is default
    emscripten_fetch(&attr, url);
#else
    // No-op for non-web platforms for now
    globalScoresFetched = true;
#endif
}

void FetchUserTop10(const char* name)
{
#if defined(PLATFORM_WEB)
    if (userScoresFetching) return;
    userScoresFetching = true;

    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    attr.onsuccess = onUserScoresSuccess;
    attr.onerror = onFetchFailure;

    char url[256];
    // BAD URL sprintf(url, "https://simple-leaderboard-api.onrender.com/game/%d/leaderboard?player_name=%s", GAME_ID, name);
    sprintf(url, "%s?action=userScores&gameID=%d&userName=%s", BASE_LEADERBOARD_URL, GAME_ID, name);  // &count=10 is default
    emscripten_fetch(&attr, url);
#else
    // No-op for non-web platforms for now
    userScoresFetched = true;
#endif
}

void UpdateLeaderboard(int *gameState, int score)
{
    if (!isLeaderboardActive) return;

    if (requestLeaderboardUpdate)
    {
        ResetLeaderboardFlags();
        requestLeaderboardUpdate = false;
    }

    if (!globalScoresFetched && !globalScoresFetching) FetchGlobalTop10();
    if (!userScoresFetched && !userScoresFetching) FetchUserTop10(playerName);


    // This is a placeholder for the full leaderboard logic
    if (IsKeyPressed(KEY_ENTER))
    {
        SetLeaderboardActive(false);
        *gameState = STATE_START;
    }
}

void ResetLeaderboardFlags()
{
    scoreSubmitted = false;
    globalScoresFetched = false;
    userScoresFetched = false;
}

void RequestLeaderboardUpdate()
{
    requestLeaderboardUpdate = true;
}

void DrawLeaderboard()
{
    if (!isLeaderboardActive) return;

    DrawText("Leaderboard", GetScreenWidth() / 2 - MeasureText("Leaderboard", 40) / 2, 50, 40, COLOR_TEXT_TITLE);

    if (globalScoresFetched && userScoresFetched)
    {
        int startY = 120;
        int lineHeight = 30;

        DrawText("GLOBAL TOP 10", GetScreenWidth() / 2 - MeasureText("GLOBAL TOP 10", 25) / 2, startY, 25, COLOR_TEXT_SUBTITLE);
        for (int i = 0; i < MAX_SCORES; i++)
        {
            if (globalTop10[i].score > 0)
            {
                DrawText(TextFormat("%d. %s - %d", i + 1, globalTop10[i].name, globalTop10[i].score),
                         GetScreenWidth() / 2 - MeasureText(TextFormat("%d. %s - %d", i + 1, globalTop10[i].name, globalTop10[i].score), 20) / 2,
                         startY + (i + 1) * lineHeight, 20, WHITE);
            }
        }

        // Check if current player is in global top 10
        bool playerInGlobalTop10 = false;
        for (int i = 0; i < MAX_SCORES; i++)
        {
            if (strcmp(globalTop10[i].name, playerName) == 0)
            {
                playerInGlobalTop10 = true;
                break;
            }
        }

        if (!playerInGlobalTop10 && userTop10[0].score > 0)
        {
            DrawText("YOUR BEST SCORE", GetScreenWidth() / 2 - MeasureText("YOUR BEST SCORE", 25) / 2, startY + (MAX_SCORES + 2) * lineHeight, 25, COLOR_TEXT_SUBTITLE);
            DrawText(TextFormat("%s - %d", userTop10[0].name, userTop10[0].score),
                     GetScreenWidth() / 2 - MeasureText(TextFormat("%s - %d", userTop10[0].name, userTop10[0].score), 20) / 2,
                     startY + (MAX_SCORES + 3) * lineHeight, 20, YELLOW);
        }

        DrawText("Press ENTER to continue", GetScreenWidth() / 2 - MeasureText("Press ENTER to continue", 20) / 2, GetScreenHeight() - 50, 20, COLOR_TEXT_SUBTITLE);
    }
    else
    {
        DrawText("Loading...", GetScreenWidth() / 2 - MeasureText("Loading...", 20) / 2, GetScreenHeight() / 2, 20, COLOR_TEXT_SUBTITLE);
    }
}

bool IsLeaderboardActive()
{
    return isLeaderboardActive;
}

void SetLeaderboardActive(bool active)
{
    isLeaderboardActive = active;
}
