//================================================================================================
//
// leaderboard.c - Leaderboard management
//
// See leaderboard.h for module interface documentation.
//
// Implementation notes:
// - Uses cJSON for JSON parsing
// - Uses emscripten_fetch for web requests on web platforms
// - Uses libcurl for HTTP requests on native platforms
//
//================================================================================================

#include "leaderboard.h"
#include "cJSON.h"
#include "config.h"
#include "game.h"
#include <stdio.h>
#include <stdlib.h> // For malloc, realloc, free
#include <string.h>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#include <emscripten/fetch.h>
#include <emscripten/html5.h> // For local storage
#else
#include <curl/curl.h>
#endif

//----------------------------------------------------------------------------------
// Internal Function Declarations
//----------------------------------------------------------------------------------
#if defined(PLATFORM_WEB)
// onSubmitSuccess - Emscripten fetch success callback for score submission
// @param fetch: pointer to emscripten_fetch_t containing response data and metadata
static void onSubmitSuccess(emscripten_fetch_t *fetch);

// onFetchFailure - Emscripten fetch error callback for any leaderboard request
// @param fetch: pointer to emscripten_fetch_t containing error info
static void onFetchFailure(emscripten_fetch_t *fetch);

// onGlobalScoresSuccess - Emscripten fetch success callback for global top scores
// @param fetch: pointer to emscripten_fetch_t containing JSON payload
static void onGlobalScoresSuccess(emscripten_fetch_t *fetch);

// onUserScoresSuccess - Emscripten fetch success callback for user-specific scores
// @param fetch: pointer to emscripten_fetch_t containing JSON payload
static void onUserScoresSuccess(emscripten_fetch_t *fetch);
#else
// Structure to hold memory for libcurl response
typedef struct {
    char *memory; // pointer to allocated buffer containing response body
    size_t size;  // size of the buffer (bytes)
} MemoryStruct;

// WriteMemoryCallback - libcurl write callback that appends incoming data to MemoryStruct
// @param contents: pointer to incoming data buffer
// @param size: size of an element (usually 1)
// @param nmemb: number of elements
// @param userp: pointer to MemoryStruct instance to append into
// @return number of bytes handled (size * nmemb) on success, 0 on failure
static size_t WriteMemoryCallback(const void *contents, size_t size, size_t nmemb, void *userp);

// CurlGetToMemory - Perform HTTP GET and store response body in MemoryStruct
// @param url: null-terminated URL to fetch
// @param out: pointer to MemoryStruct to receive response (caller frees out->memory)
// @return true on success, false on failure
static bool CurlGetToMemory(const char *url, MemoryStruct *out);

// CurlPerformNoWrite - Perform HTTP request when response body is not needed
// @param url: null-terminated URL to request
// @return true on success (HTTP request completed), false on failure
static bool CurlPerformNoWrite(const char *url);

// GetConfigPath - Returns path to local configuration file for native platforms
// @return pointer to static buffer containing path (do not free)
static const char *GetConfigPath(void);
#endif

// ParseUserScores - Parse JSON payload into user-specific leaderboard entries
// @param data: pointer to JSON data buffer
// @param size: length of data buffer in bytes
// @param entries: output array of LeaderboardEntry to populate
// @param fetchedFlag: pointer to bool set to true when parsing completes
// @param fetchingFlag: pointer to bool cleared when parsing completes
static void ParseUserScores(const char *data, size_t size, LeaderboardEntry *entries, bool *fetchedFlag,
                            bool *fetchingFlag);

// ParseGlobalScores - Parse JSON payload into global leaderboard entries
// @param data: pointer to JSON data buffer
// @param size: length of data buffer in bytes
// @param entries: output array of LeaderboardEntry to populate
// @param fetchedFlag: pointer to bool set to true when parsing completes
// @param fetchingFlag: pointer to bool cleared when parsing completes
static void ParseGlobalScores(const char *data, size_t size, LeaderboardEntry *entries, bool *fetchedFlag,
                              bool *fetchingFlag);

// UpdateLeaderboardLayout - Recompute positions of name input boxes and buttons
// This ensures UI elements follow the current screen size when the window is resized
static void UpdateLeaderboardLayout(LeaderboardManager *mgr);

// SavePlayerName - Persist current player name to platform storage
// @param mgr: pointer to LeaderboardManager containing playerName to save
static void SavePlayerName(const LeaderboardManager *mgr);

// LoadPlayerName - Load saved player name from platform storage into manager
// @param mgr: pointer to LeaderboardManager to receive loaded name
static void LoadPlayerName(LeaderboardManager *mgr);

// FetchGlobalTop10 - Initiate retrieval of global top scores (asynchronous)
// @param mgr: pointer to LeaderboardManager which will receive results
static void FetchGlobalTop10(LeaderboardManager *mgr);

// FetchUserTop10 - Initiate retrieval of user's top scores (asynchronous)
// @param mgr: pointer to LeaderboardManager which will receive results
// @param name: user name string to query scores for
static void FetchUserTop10(LeaderboardManager *mgr, const char *name);

// RequestLeaderboardUpdate - Mark leaderboard manager to refresh data on next update
// (used by callbacks that cannot receive a user pointer)
static void RequestLeaderboardUpdate(void);

// Use the LeaderboardManager declared in the header.
// A small static pointer is used only for web callbacks which cannot receive
// a user-data pointer from emscripten_fetch handlers.
static LeaderboardManager *s_lb_for_callbacks = NULL;

// ================================================================================
// Public functions
// ================================================================================

void DrawLeaderboard(const LeaderboardManager *mgr)
{
    if (!mgr || !mgr->isActive) return;

    DrawText("Leaderboard", GetScreenWidth() / 2 - MeasureText("Leaderboard", 40) / 2, 50, 40, COLOR_TEXT_TITLE);

    if (mgr->globalScoresFetched && mgr->userScoresFetched) {
        int startY = 120;
        int lineHeight = 30;

        DrawText("GLOBAL TOP 10", GetScreenWidth() / 2 - MeasureText("GLOBAL TOP 10", 25) / 2, startY, 25,
                 COLOR_TEXT_SUBTITLE);
        for (int i = 0; i < LEADERBOARD_MAX_SCORES; i++) {
            if (mgr->globalTop10[i].score > 0) {
                DrawText(TextFormat("%d. %s - %d", i + 1, mgr->globalTop10[i].name, mgr->globalTop10[i].score),
                         GetScreenWidth() / 2 - MeasureText(TextFormat("%d. %s - %d", i + 1, mgr->globalTop10[i].name,
                                                                       mgr->globalTop10[i].score),
                                                            20) /
                                                    2,
                         startY + (i + 1) * lineHeight, 20, COLOR_TEXT_LEADERBOARD);
            }
        }

        // Check if current player is in global top 10
        bool playerInGlobalTop10 = false;
        for (int i = 0; i < LEADERBOARD_MAX_SCORES; i++) {
            if (strcmp(mgr->globalTop10[i].name, mgr->playerName) == 0) {
                playerInGlobalTop10 = true;
                break;
            }
        }

        if (!playerInGlobalTop10 && mgr->userTop10[0].score > 0) {
            DrawText("YOUR BEST SCORE", GetScreenWidth() / 2 - MeasureText("YOUR BEST SCORE", 25) / 2,
                     startY + (LEADERBOARD_MAX_SCORES + 2) * lineHeight, 25, COLOR_TEXT_SUBTITLE);
            DrawText(TextFormat("%s - %d", mgr->userTop10[0].name, mgr->userTop10[0].score),
                     GetScreenWidth() / 2 -
                         MeasureText(TextFormat("%s - %d", mgr->userTop10[0].name, mgr->userTop10[0].score), 20) / 2,
                     startY + (LEADERBOARD_MAX_SCORES + 3) * lineHeight, 20, COLOR_TEXT_SUBTITLE);
        }

        DrawText("Press ENTER or CLICK to Continue",
                 GetScreenWidth() / 2 - MeasureText("Press ENTER or CLICK to Continue", 20) / 2, GetScreenHeight() - 50,
                 20, COLOR_TEXT_SUBTITLE);
    }
    else {
        DrawText("Loading...", GetScreenWidth() / 2 - MeasureText("Loading...", 20) / 2, GetScreenHeight() / 2, 20,
                 COLOR_TEXT_SUBTITLE);
    }
}

void DrawNameInput(LeaderboardManager *mgr)
{
    // Ensure layout is up-to-date with the current screen size
    UpdateLeaderboardLayout((LeaderboardManager *)mgr);

    DrawText("Enter Your Initials", GetScreenWidth() / 2 - MeasureText("Enter Your Initials", 30) / 2,
             GetScreenHeight() / 2 - 100, 30, COLOR_TEXT_TITLE);

    for (int i = 0; i < LEADERBOARD_NAME_LENGTH; i++) {
        // Draw outlined character box and reversed-color initial
        // DrawRectangleLinesEx(mgr->charBoxes[i], 2, COLOR_INITIAL_BOX);
        DrawText(TextFormat("%c", mgr->playerName[i]), mgr->charBoxes[i].x + 12, mgr->charBoxes[i].y + 5, 30,
                 COLOR_INITIAL_BOX);
        // Draw outlined up/down arrows
        DrawTriangleLines((Vector2){mgr->upArrows[i].x + 20, mgr->upArrows[i].y},
                          (Vector2){mgr->upArrows[i].x, mgr->upArrows[i].y + 20},
                          (Vector2){mgr->upArrows[i].x + 40, mgr->upArrows[i].y + 20}, COLOR_INITIAL_BOX);
        DrawTriangleLines((Vector2){mgr->downArrows[i].x + 20, mgr->downArrows[i].y + 20},
                          (Vector2){mgr->downArrows[i].x + 40, mgr->downArrows[i].y},
                          (Vector2){mgr->downArrows[i].x, mgr->downArrows[i].y}, COLOR_INITIAL_BOX);
    }

    // Draw outlined Submit and Skip buttons with reversed text color
    DrawRectangleLinesEx(mgr->submitButton, 2, COLOR_BUTTON_BOX);
    DrawText("Submit", mgr->submitButton.x + 30, mgr->submitButton.y + 5, 20, COLOR_BUTTON_BOX);

    DrawRectangleLinesEx(mgr->skipButton, 2, COLOR_BUTTON_BOX);
    DrawText("Skip", mgr->skipButton.x + 35, mgr->skipButton.y + 5, 20, COLOR_BUTTON_BOX);
}

void InitLeaderboard(LeaderboardManager *mgr)
{
    if (!mgr) return;
    // set defaults
    mgr->playerName[0] = 'A';
    mgr->playerName[1] = 'A';
    mgr->playerName[2] = 'A';
    mgr->playerName[3] = '\0';
    mgr->isActive = false;
    mgr->scoreSubmitted = false;
    mgr->globalScoresFetched = false;
    mgr->userScoresFetched = false;
    mgr->globalScoresFetching = false;
    mgr->userScoresFetching = false;
    mgr->requestUpdate = false;
    mgr->skipSubmission = false;

    // Compute layout based on current screen size
    UpdateLeaderboardLayout(mgr);

    // Save pointer for emscripten callbacks
    s_lb_for_callbacks = mgr;

    LoadPlayerName(mgr);
}

void ResetLeaderboardFlags(LeaderboardManager *mgr)
{
    if (!mgr) return;
    mgr->scoreSubmitted = false;
    mgr->globalScoresFetched = false;
    mgr->userScoresFetched = false;
}

void SetLeaderboardActive(LeaderboardManager *mgr, bool active)
{
    if (!mgr) return;
    mgr->isActive = active;
}

void SubmitScore(LeaderboardManager *mgr, int score)
{
    char url[256];
    if (!mgr) return;
    sprintf(url, "%s?action=newScore&gameID=%d&userName=%s&score=%d", LEADERBOARD_BASE_URL, LEADERBOARD_GAME_ID,
            mgr->playerName, score);

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
    }
    else {
        printf("Score submitted successfully.\n");
        mgr->scoreSubmitted = true;
        mgr->requestUpdate = true;
        RequestLeaderboardUpdate();
    }
#endif
}

void UpdateLeaderboard(LeaderboardManager *mgr, int *gameState)
{
    if (!mgr || !mgr->isActive) return;

    if (mgr->requestUpdate) {
        ResetLeaderboardFlags(mgr);
        mgr->requestUpdate = false;
    }

    if (!mgr->globalScoresFetched && !mgr->globalScoresFetching) FetchGlobalTop10(mgr);
    if (!mgr->userScoresFetched && !mgr->userScoresFetching) FetchUserTop10(mgr, mgr->playerName);

    if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        SetLeaderboardActive(mgr, false);
        *gameState = STATE_START;
    }
}

bool UpdateNameInput(LeaderboardManager *mgr)
{
    if (!mgr) return false;
    // Ensure hitboxes follow current window size
    UpdateLeaderboardLayout(mgr);
    for (int i = 0; i < LEADERBOARD_NAME_LENGTH; i++) {
        if (CheckCollisionPointRec(GetMousePosition(), mgr->upArrows[i]) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            mgr->playerName[i]++;
            if (mgr->playerName[i] > 'Z') mgr->playerName[i] = 'A';
        }
        if (CheckCollisionPointRec(GetMousePosition(), mgr->downArrows[i]) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            mgr->playerName[i]--;
            if (mgr->playerName[i] < 'A') mgr->playerName[i] = 'Z';
        }
    }

    if (CheckCollisionPointRec(GetMousePosition(), mgr->submitButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        SavePlayerName(mgr);
        return true;
    }
    if (CheckCollisionPointRec(GetMousePosition(), mgr->skipButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        // User chose not to submit their score
        mgr->skipSubmission = true;
        return true;
    }
    return false;
}

//----------------------------------------------------------------------------------
// Internal Function Implementations
//----------------------------------------------------------------------------------

#if defined(PLATFORM_WEB)
#define PLAYER_NAME_STORAGE_KEY "tailgunner_player_name"
// clang-format off
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
// clang-format on

static void onSubmitSuccess(emscripten_fetch_t *fetch)
{
    printf("Score submitted successfully.\n");
    if (s_lb_for_callbacks) s_lb_for_callbacks->scoreSubmitted = true;
    emscripten_fetch_close(fetch);
    RequestLeaderboardUpdate();
}

static void onFetchFailure(emscripten_fetch_t *fetch)
{
    printf("Failed to fetch data: %s\n", fetch->statusText);
    if (!s_lb_for_callbacks) {
        emscripten_fetch_close(fetch);
        return;
    }
    if (strcmp(fetch->url + strlen(fetch->url) - 8, "topScores") == 0) {
        s_lb_for_callbacks->globalScoresFetching = false;
    }
    else {
        s_lb_for_callbacks->userScoresFetching = false;
    }
    emscripten_fetch_close(fetch);
}

static void onGlobalScoresSuccess(emscripten_fetch_t *fetch)
{
    printf("Global scores fetched successfully.\n");
    if (!s_lb_for_callbacks) return;
    ParseGlobalScores(fetch->data, fetch->numBytes, s_lb_for_callbacks->globalTop10,
                      &s_lb_for_callbacks->globalScoresFetched, &s_lb_for_callbacks->globalScoresFetching);
}

static void onUserScoresSuccess(emscripten_fetch_t *fetch)
{
    printf("User scores fetched successfully.\n");
    if (!s_lb_for_callbacks) return;
    ParseUserScores(fetch->data, fetch->numBytes, s_lb_for_callbacks->userTop10, &s_lb_for_callbacks->userScoresFetched,
                    &s_lb_for_callbacks->userScoresFetching);
}
#else
static size_t WriteMemoryCallback(const void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    MemoryStruct *mem = (MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (ptr == NULL) {
        // out of memory!
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

    if (res != CURLE_OK) {
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
    if (!curl) {
        curl_global_cleanup();
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);

    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    return (res == CURLE_OK);
}

static const char *GetConfigPath(void)
{
    const char *homeDir = getenv("HOME");
    if (homeDir == NULL) {
        return ".tailgunner.conf";
    }
    static char path[256];
    snprintf(path, sizeof(path), "%s/.tailgunner.conf", homeDir);
    return path;
}
#endif

static void ParseUserScores(const char *data, size_t size, LeaderboardEntry *entries, bool *fetchedFlag,
                            bool *fetchingFlag)
{
    memset(entries, 0, sizeof(LeaderboardEntry) * LEADERBOARD_MAX_SCORES);
    cJSON *json = cJSON_ParseWithLength(data, size);
    if (json == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }
        *fetchedFlag = true; // Mark as fetched to avoid continuous retries on parse error
        *fetchingFlag = false;
        return;
    }

    int count = cJSON_GetArraySize(json);
    int entryIndex = 0;
    for (int i = 0; i < count && entryIndex < LEADERBOARD_MAX_SCORES; i++) {
        cJSON *item = cJSON_GetArrayItem(json, i);
        if (item != NULL && item->child != NULL) {
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

static void ParseGlobalScores(const char *data, size_t size, LeaderboardEntry *entries, bool *fetchedFlag,
                              bool *fetchingFlag)
{
    memset(entries, 0, sizeof(LeaderboardEntry) * LEADERBOARD_MAX_SCORES);
    cJSON *json = cJSON_ParseWithLength(data, size);
    if (json == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }
        *fetchedFlag = true; // Mark as fetched to avoid continuous retries on parse error
        *fetchingFlag = false;
        return;
    }

    int count = cJSON_GetArraySize(json);
    int entryIndex = 0;
    for (int i = 0; i < count && entryIndex < LEADERBOARD_MAX_SCORES; i++) {
        cJSON *item = cJSON_GetArrayItem(json, i);
        const cJSON *name = cJSON_GetObjectItemCaseSensitive(item, "userName");
        cJSON *score = cJSON_GetObjectItemCaseSensitive(item, "score");

        if (cJSON_IsString(name) && (name->valuestring != NULL) && cJSON_IsNumber(score)) {
            // Check if this (user, score) pair already exists to avoid exact duplicates
            bool isDuplicate = false;
            for (int j = 0; j < entryIndex; j++) {
                if (strcmp(entries[j].name, name->valuestring) == 0 && entries[j].score == (int)score->valuedouble) {
                    isDuplicate = true;
                    break;
                }
            }

            if (!isDuplicate) {
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

static void UpdateLeaderboardLayout(LeaderboardManager *mgr)
{
    if (!mgr) return;
    int baseX = GetScreenWidth() / 2 - 70;
    int baseY = GetScreenHeight() / 2 - 20;

    for (int i = 0; i < LEADERBOARD_NAME_LENGTH; i++) {
        mgr->charBoxes[i] = (Rectangle){baseX + i * 50, baseY, 40, 40};
        mgr->upArrows[i] = (Rectangle){mgr->charBoxes[i].x, mgr->charBoxes[i].y - 30, 40, 20};
        mgr->downArrows[i] = (Rectangle){mgr->charBoxes[i].x, mgr->charBoxes[i].y + 50, 40, 20};
    }
    mgr->submitButton = (Rectangle){GetScreenWidth() / 2 - 140, GetScreenHeight() / 2 + 80, 120, 30};
    mgr->skipButton = (Rectangle){mgr->submitButton.x + mgr->submitButton.width + 20, mgr->submitButton.y, 120, 30};
}

static void SavePlayerName(const LeaderboardManager *mgr)
{
#if defined(PLATFORM_WEB)
    emscripten_local_storage_set_item_js(PLAYER_NAME_STORAGE_KEY, mgr->playerName);
#else
    FILE *f = fopen(GetConfigPath(), "w");
    if (f != NULL) {
        fputs(mgr->playerName, f);
        fclose(f);
    }
#endif
}

static void LoadPlayerName(LeaderboardManager *mgr)
{
#if defined(PLATFORM_WEB)
    char *storedName = emscripten_local_storage_get_item_js(PLAYER_NAME_STORAGE_KEY);
    if (storedName != NULL) {
        strncpy(mgr->playerName, storedName, LEADERBOARD_NAME_LENGTH);
        mgr->playerName[LEADERBOARD_NAME_LENGTH] = '\0';
        free(storedName);
    }
#else
    FILE *f = fopen(GetConfigPath(), "r");
    if (f != NULL) {
        if (fgets(mgr->playerName, sizeof(mgr->playerName), f) != NULL) {
            // remove newline character
            mgr->playerName[strcspn(mgr->playerName, "\n")] = 0;
        }
        fclose(f);
    }
#endif
}

static void FetchGlobalTop10(LeaderboardManager *mgr)
{
    char url[256];
    sprintf(url, "%s?action=topScores&gameID=%d", LEADERBOARD_BASE_URL, LEADERBOARD_GAME_ID); // &count=10 is default

#if defined(PLATFORM_WEB)
    if (mgr->globalScoresFetching) return;
    mgr->globalScoresFetching = true;

    // arrange for callbacks to reference this manager
    s_lb_for_callbacks = mgr;

    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    attr.onsuccess = onGlobalScoresSuccess;
    attr.onerror = onFetchFailure;

    emscripten_fetch(&attr, url);
#else
    if (mgr->globalScoresFetching) return;
    mgr->globalScoresFetching = true;

    MemoryStruct chunk;
    if (!CurlGetToMemory(url, &chunk)) {
        fprintf(stderr, "Failed to fetch global scores from URL: %s\n", url);
        mgr->globalScoresFetching = false;
    }
    else {
        printf("Global scores fetched successfully.\n");
        ParseGlobalScores(chunk.memory, chunk.size, mgr->globalTop10, &mgr->globalScoresFetched,
                          &mgr->globalScoresFetching);
        free(chunk.memory);
    }
#endif
}

static void FetchUserTop10(LeaderboardManager *mgr, const char *name)
{
    char url[256];
    sprintf(url, "%s?action=userScores&gameID=%d&userName=%s", LEADERBOARD_BASE_URL, LEADERBOARD_GAME_ID,
            name); // &count=10 is default

#if defined(PLATFORM_WEB)
    if (mgr->userScoresFetching) return;
    mgr->userScoresFetching = true;

    // arrange for callbacks to reference this manager
    s_lb_for_callbacks = mgr;

    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    attr.onsuccess = onUserScoresSuccess;
    attr.onerror = onFetchFailure;

    emscripten_fetch(&attr, url);
#else
    if (mgr->userScoresFetching) return;
    mgr->userScoresFetching = true;

    MemoryStruct chunk;
    if (!CurlGetToMemory(url, &chunk)) {
        fprintf(stderr, "Failed to fetch user scores from URL: %s\n", url);
        mgr->userScoresFetching = false;
    }
    else {
        printf("User scores fetched successfully.\n");
        ParseUserScores(chunk.memory, chunk.size, mgr->userTop10, &mgr->userScoresFetched, &mgr->userScoresFetching);
        free(chunk.memory);
    }
#endif
}

static void RequestLeaderboardUpdate(void)
{
    if (s_lb_for_callbacks) s_lb_for_callbacks->requestUpdate = true;
}
