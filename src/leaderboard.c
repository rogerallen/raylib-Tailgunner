#include "leaderboard.h"
#include "game.h"
#include "config.h"
#include "cJSON.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h> // For malloc, realloc, free

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#include <emscripten/fetch.h>
#include <emscripten/html5.h> // For local storage
#else
#include <curl/curl.h>
#endif

// Internal function declarations

static void FetchGlobalTop10();
static void FetchUserTop10(const char* name);

static void RequestLeaderboardUpdate();

static void ParseUserScores(const char *data, size_t size, LeaderboardEntry *entries, bool *fetchedFlag, bool *fetchingFlag);
static void ParseGlobalScores(const char *data, size_t size, LeaderboardEntry *entries, bool *fetchedFlag, bool *fetchingFlag);

// Encapsulate leaderboard state in a single entity to avoid scattered globals.
typedef struct LeaderboardEntity {
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
} LeaderboardEntity;

static LeaderboardEntity g_leaderboard = {
    .playerName = { 'A', 'A', 'A', '\0' },
    .isActive = false,
    .scoreSubmitted = false,
    .globalScoresFetched = false,
    .userScoresFetched = false,
    .globalScoresFetching = false,
    .userScoresFetching = false,
    .requestUpdate = false,
};

#if defined(PLATFORM_WEB)
#define PLAYER_NAME_STORAGE_KEY "tailgunner_player_name"

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

static void onSubmitSuccess(emscripten_fetch_t *fetch) {
    printf("Score submitted successfully.\n");
    g_leaderboard.scoreSubmitted = true;
    emscripten_fetch_close(fetch);
    RequestLeaderboardUpdate();
}

static void onFetchFailure(emscripten_fetch_t *fetch) {
    printf("Failed to fetch data: %s\n", fetch->statusText);
    if (strcmp(fetch->url + strlen(fetch->url) - 8, "topScores") == 0) {
        g_leaderboard.globalScoresFetching = false;
    } else {
        g_leaderboard.userScoresFetching = false;
    }
    emscripten_fetch_close(fetch);
}

static void onGlobalScoresSuccess(emscripten_fetch_t *fetch) {
    printf("Global scores fetched successfully.\n");
    ParseGlobalScores(fetch->data, fetch->numBytes, g_leaderboard.globalTop10, &g_leaderboard.globalScoresFetched, &g_leaderboard.globalScoresFetching);
}

static void onUserScoresSuccess(emscripten_fetch_t *fetch) {
    printf("User scores fetched successfully.\n");
    ParseUserScores(fetch->data, fetch->numBytes, g_leaderboard.userTop10, &g_leaderboard.userScoresFetched, &g_leaderboard.userScoresFetching);
}
#else
// Structure to hold memory for libcurl response
typedef struct {
    char *memory;
    size_t size;
} MemoryStruct;

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    MemoryStruct *mem = (MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if(ptr == NULL) {
        /* out of memory! */
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

// Helper: perform a GET request into memory using libcurl. Caller must free out->memory on success.
static bool CurlGetToMemory(const char *url, MemoryStruct *out)
{
    CURL *curl;
    CURLcode res;

    out->memory = malloc(1);
    if (!out->memory) return false;
    out->size = 0;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (!curl) {
        curl_global_cleanup();
        free(out->memory);
        out->memory = NULL;
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)out);

    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    if (res != CURLE_OK)
    {
        free(out->memory);
        out->memory = NULL;
        out->size = 0;
        return false;
    }

    return true;
}

// Helper: perform a request when we don't need response body (e.g., submit score)
static bool CurlPerformNoWrite(const char *url)
{
    CURL *curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if(!curl) { curl_global_cleanup(); return false; }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);

    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    return (res == CURLE_OK);
}

static const char* GetConfigPath()
{
    const char* homeDir = getenv("HOME");
    if (homeDir == NULL) {
        return ".tailgunner.conf";
    }
    static char path[256];
    snprintf(path, sizeof(path), "%s/.tailgunner.conf", homeDir);
    return path;
}
#endif

static void ParseUserScores(const char *data, size_t size, LeaderboardEntry *entries, bool *fetchedFlag, bool *fetchingFlag)
{
    memset(entries, 0, sizeof(LeaderboardEntry) * LEADERBOARD_MAX_SCORES);
    cJSON *json = cJSON_ParseWithLength(data, size);
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
    int entryIndex = 0;
    for (int i = 0; i < count && entryIndex < LEADERBOARD_MAX_SCORES; i++)
    {
        cJSON *item = cJSON_GetArrayItem(json, i);
        if (item != NULL && item->child != NULL)
        {
            strncpy(entries[entryIndex].name, item->child->string, LEADERBOARD_NAME_LENGTH);
            entries[entryIndex].name[LEADERBOARD_NAME_LENGTH] = '\0';
            entries[entryIndex].score = item->child->valueint;
            printf("Parsed: %s - %d\n", entries[entryIndex].name, entries[entryIndex].score);
            entryIndex++;
        }
    }

    cJSON_Delete(json);
    *fetchedFlag = true;
    *fetchingFlag = false;
}

static void ParseGlobalScores(const char *data, size_t size, LeaderboardEntry *entries, bool *fetchedFlag, bool *fetchingFlag)
{
    memset(entries, 0, sizeof(LeaderboardEntry) * LEADERBOARD_MAX_SCORES);
    cJSON *json = cJSON_ParseWithLength(data, size);
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
    int entryIndex = 0;
    for (int i = 0; i < count && entryIndex < LEADERBOARD_MAX_SCORES; i++)
    {
        cJSON *item = cJSON_GetArrayItem(json, i);
        cJSON *name = cJSON_GetObjectItemCaseSensitive(item, "userName");
        cJSON *score = cJSON_GetObjectItemCaseSensitive(item, "score");

        if (cJSON_IsString(name) && (name->valuestring != NULL) && cJSON_IsNumber(score))
        {
            // Check if this (user, score) pair already exists to avoid exact duplicates
            bool isDuplicate = false;
            for (int j = 0; j < entryIndex; j++)
            {
                if (strcmp(entries[j].name, name->valuestring) == 0 && 
                    entries[j].score == (int)score->valuedouble)
                {
                    isDuplicate = true;
                    break;
                }
            }

            if (!isDuplicate)
            {
                strncpy(entries[entryIndex].name, name->valuestring, LEADERBOARD_NAME_LENGTH);
                entries[entryIndex].name[LEADERBOARD_NAME_LENGTH] = '\0';
                entries[entryIndex].score = (int)score->valuedouble;
                printf("Parsed: %s - %d\n", entries[entryIndex].name, entries[entryIndex].score);
                entryIndex++;
            }
        }
    }

    cJSON_Delete(json);
    *fetchedFlag = true;
    *fetchingFlag = false;
}

static void SavePlayerName(const char* name)
{
#if defined(PLATFORM_WEB)
    emscripten_local_storage_set_item_js(PLAYER_NAME_STORAGE_KEY, g_leaderboard.playerName);
#else
    FILE* f = fopen(GetConfigPath(), "w");
    if (f != NULL) {
        fputs(name, f);
        fclose(f);
    }
#endif
}

static void LoadPlayerName()
{
#if defined(PLATFORM_WEB)
    char *storedName = emscripten_local_storage_get_item_js(PLAYER_NAME_STORAGE_KEY);
    if (storedName != NULL)
    {
        strncpy(g_leaderboard.playerName, storedName, LEADERBOARD_NAME_LENGTH);
        g_leaderboard.playerName[LEADERBOARD_NAME_LENGTH] = '\0';
        free(storedName);
    }
#else
    FILE* f = fopen(GetConfigPath(), "r");
    if (f != NULL) {
        if (fgets(g_leaderboard.playerName, sizeof(g_leaderboard.playerName), f) != NULL) {
            // remove newline character
            g_leaderboard.playerName[strcspn(g_leaderboard.playerName, "\n")] = 0;
        }
        fclose(f);
    }
#endif
}

static void FetchGlobalTop10()
{
    char url[256];
    sprintf(url, "%s?action=topScores&gameID=%d", LEADERBOARD_BASE_URL, LEADERBOARD_GAME_ID);  // &count=10 is default

#if defined(PLATFORM_WEB)
    if (g_leaderboard.globalScoresFetching) return;
    g_leaderboard.globalScoresFetching = true;

    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    attr.onsuccess = onGlobalScoresSuccess;
    attr.onerror = onFetchFailure;

    emscripten_fetch(&attr, url);
#else
    if (g_leaderboard.globalScoresFetching) return;
    g_leaderboard.globalScoresFetching = true;

    MemoryStruct chunk;
    if (!CurlGetToMemory(url, &chunk)) {
        fprintf(stderr, "Failed to fetch global scores from URL: %s\n", url);
        g_leaderboard.globalScoresFetching = false;
    } else {
        printf("Global scores fetched successfully.\n");
        ParseGlobalScores(chunk.memory, chunk.size, g_leaderboard.globalTop10, &g_leaderboard.globalScoresFetched, &g_leaderboard.globalScoresFetching);
        free(chunk.memory);
    }
#endif
}

static void FetchUserTop10(const char* name)
{
    char url[256];
    sprintf(url, "%s?action=userScores&gameID=%d&userName=%s", LEADERBOARD_BASE_URL, LEADERBOARD_GAME_ID, name);  // &count=10 is default

#if defined(PLATFORM_WEB)
    if (g_leaderboard.userScoresFetching) return;
    g_leaderboard.userScoresFetching = true;

    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    attr.onsuccess = onUserScoresSuccess;
    attr.onerror = onFetchFailure;

    emscripten_fetch(&attr, url);
#else
    if (g_leaderboard.userScoresFetching) return;
    g_leaderboard.userScoresFetching = true;

    MemoryStruct chunk;
    if (!CurlGetToMemory(url, &chunk)) {
        fprintf(stderr, "Failed to fetch user scores from URL: %s\n", url);
        g_leaderboard.userScoresFetching = false;
    } else {
        printf("User scores fetched successfully.\n");
        ParseUserScores(chunk.memory, chunk.size, g_leaderboard.userTop10, &g_leaderboard.userScoresFetched, &g_leaderboard.userScoresFetching);
        free(chunk.memory);
    }
#endif
}

static void RequestLeaderboardUpdate()
{
    g_leaderboard.requestUpdate = true;
}

// ================================================================================
// Public functions
// ================================================================================

void DrawLeaderboard()
{
    if (!g_leaderboard.isActive) return;

    DrawText("Leaderboard", GetScreenWidth() / 2 - MeasureText("Leaderboard", 40) / 2, 50, 40, COLOR_TEXT_TITLE);

    if (g_leaderboard.globalScoresFetched && g_leaderboard.userScoresFetched)
    {
        int startY = 120;
        int lineHeight = 30;

        DrawText("GLOBAL TOP 10", GetScreenWidth() / 2 - MeasureText("GLOBAL TOP 10", 25) / 2, startY, 25, COLOR_TEXT_SUBTITLE);
        for (int i = 0; i < LEADERBOARD_MAX_SCORES; i++)
        {
            if (g_leaderboard.globalTop10[i].score > 0)
            {
                DrawText(TextFormat("%d. %s - %d", i + 1, g_leaderboard.globalTop10[i].name, g_leaderboard.globalTop10[i].score),
                         GetScreenWidth() / 2 - MeasureText(TextFormat("%d. %s - %d", i + 1, g_leaderboard.globalTop10[i].name, g_leaderboard.globalTop10[i].score), 20) / 2,
                         startY + (i + 1) * lineHeight, 20, COLOR_TEXT_LEADERBOARD);
            }
        }

        // Check if current player is in global top 10
        bool playerInGlobalTop10 = false;
        for (int i = 0; i < LEADERBOARD_MAX_SCORES; i++)
        {
            if (strcmp(g_leaderboard.globalTop10[i].name, g_leaderboard.playerName) == 0)
            {
                playerInGlobalTop10 = true;
                break;
            }
        }

        if (!playerInGlobalTop10 && g_leaderboard.userTop10[0].score > 0)
        {
            DrawText("YOUR BEST SCORE", GetScreenWidth() / 2 - MeasureText("YOUR BEST SCORE", 25) / 2, startY + (LEADERBOARD_MAX_SCORES + 2) * lineHeight, 25, COLOR_TEXT_SUBTITLE);
            DrawText(TextFormat("%s - %d", g_leaderboard.userTop10[0].name, g_leaderboard.userTop10[0].score),
                     GetScreenWidth() / 2 - MeasureText(TextFormat("%s - %d", g_leaderboard.userTop10[0].name, g_leaderboard.userTop10[0].score), 20) / 2,
                     startY + (LEADERBOARD_MAX_SCORES + 3) * lineHeight, 20, COLOR_TEXT_SUBTITLE);
        }

        DrawText("Press ENTER or CLICK to Continue", GetScreenWidth() / 2 - MeasureText("Press ENTER or CLICK to Continue", 20) / 2, GetScreenHeight() - 50, 20, COLOR_TEXT_SUBTITLE);
    }
    else
    {
        DrawText("Loading...", GetScreenWidth() / 2 - MeasureText("Loading...", 20) / 2, GetScreenHeight() / 2, 20, COLOR_TEXT_SUBTITLE);
    }
}

void DrawNameInput()
{
    DrawText("Enter Your Initials", GetScreenWidth() / 2 - MeasureText("Enter Your Initials", 30) / 2, GetScreenHeight() / 2 - 100, 30, COLOR_TEXT_TITLE);

    for (int i = 0; i < LEADERBOARD_NAME_LENGTH; i++)
    {
        DrawRectangleRec(g_leaderboard.charBoxes[i], COLOR_INITIAL_BOX);
        DrawText(TextFormat("%c", g_leaderboard.playerName[i]), g_leaderboard.charBoxes[i].x + 12, g_leaderboard.charBoxes[i].y + 5, 30, COLOR_BACKGROUND);
        DrawTriangle((Vector2){ g_leaderboard.upArrows[i].x + 20, g_leaderboard.upArrows[i].y }, (Vector2){ g_leaderboard.upArrows[i].x, g_leaderboard.upArrows[i].y + 20 }, (Vector2){ g_leaderboard.upArrows[i].x + 40, g_leaderboard.upArrows[i].y + 20 }, COLOR_INITIAL_BOX);
        DrawTriangle((Vector2){ g_leaderboard.downArrows[i].x + 20, g_leaderboard.downArrows[i].y + 20 }, (Vector2){ g_leaderboard.downArrows[i].x + 40, g_leaderboard.downArrows[i].y }, (Vector2){ g_leaderboard.downArrows[i].x, g_leaderboard.downArrows[i].y }, COLOR_INITIAL_BOX);
    }

    DrawRectangleRec(g_leaderboard.submitButton, COLOR_INITIAL_BOX);
    DrawText("Submit", g_leaderboard.submitButton.x + 30, g_leaderboard.submitButton.y + 5, 20, COLOR_BACKGROUND);
}

void InitLeaderboard()
{
    for (int i = 0; i < LEADERBOARD_NAME_LENGTH; i++)
    {
        g_leaderboard.charBoxes[i] = (Rectangle){ GetScreenWidth() / 2 - 70 + i * 50, GetScreenHeight() / 2 - 20, 40, 40 };
        g_leaderboard.upArrows[i] = (Rectangle){ g_leaderboard.charBoxes[i].x, g_leaderboard.charBoxes[i].y - 30, 40, 20 };
        g_leaderboard.downArrows[i] = (Rectangle){ g_leaderboard.charBoxes[i].x, g_leaderboard.charBoxes[i].y + 50, 40, 20 };
    }
    g_leaderboard.submitButton = (Rectangle){ GetScreenWidth() / 2 - 60, GetScreenHeight() / 2 + 80, 120, 30 };
    LoadPlayerName();
}

void ResetLeaderboardFlags()
{
    g_leaderboard.scoreSubmitted = false;
    g_leaderboard.globalScoresFetched = false;
    g_leaderboard.userScoresFetched = false;
}

void SetLeaderboardActive(bool active)
{
    g_leaderboard.isActive = active;
}

void SubmitScore(int score)
{
    char url[256];
    sprintf(url, "%s?action=newScore&gameID=%d&userName=%s&score=%d", LEADERBOARD_BASE_URL, LEADERBOARD_GAME_ID, g_leaderboard.playerName, score);

#if defined(PLATFORM_WEB)
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "POST");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    attr.onsuccess = onSubmitSuccess;
    attr.onerror = onFetchFailure;

    emscripten_fetch(&attr, url);
#else
    if (!CurlPerformNoWrite(url)) {
        fprintf(stderr, "Score submission failed for URL: %s\n", url);
    } else {
        printf("Score submitted successfully.\n");
        g_leaderboard.scoreSubmitted = true;
        RequestLeaderboardUpdate();
    }
#endif
}

void UpdateLeaderboard(int *gameState, int score)
{
    if (!g_leaderboard.isActive) return;

    if (g_leaderboard.requestUpdate)
    {
        ResetLeaderboardFlags();
        g_leaderboard.requestUpdate = false;
    }

    if (!g_leaderboard.globalScoresFetched && !g_leaderboard.globalScoresFetching) FetchGlobalTop10();
    if (!g_leaderboard.userScoresFetched && !g_leaderboard.userScoresFetching) FetchUserTop10(g_leaderboard.playerName);

    if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        SetLeaderboardActive(false);
        *gameState = STATE_START;
    }
}

bool UpdateNameInput()
{
    for (int i = 0; i < LEADERBOARD_NAME_LENGTH; i++)
    {
        if (CheckCollisionPointRec(GetMousePosition(), g_leaderboard.upArrows[i]) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            g_leaderboard.playerName[i]++;
            if (g_leaderboard.playerName[i] > 'Z') g_leaderboard.playerName[i] = 'A';
        }
        if (CheckCollisionPointRec(GetMousePosition(), g_leaderboard.downArrows[i]) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            g_leaderboard.playerName[i]--;
            if (g_leaderboard.playerName[i] < 'A') g_leaderboard.playerName[i] = 'Z';
        }
    }

    if (CheckCollisionPointRec(GetMousePosition(), g_leaderboard.submitButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        SavePlayerName(g_leaderboard.playerName);
        return true;
    }
    return false;
}

