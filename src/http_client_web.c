#if defined(PLATFORM_WEB)

#include "http_client.h"
#include <emscripten/fetch.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    HttpCallback callback;
} FetchUserData;

void on_fetch_success(emscripten_fetch_t *fetch)
{
    HttpResponse response = {.data = (char *)fetch->data, .size = fetch->numBytes, .status = fetch->status};

    FetchUserData *userData = (FetchUserData *)fetch->userData;
    userData->callback(response);

    free(userData);
    emscripten_fetch_close(fetch);
}

void on_fetch_error(emscripten_fetch_t *fetch)
{
    HttpResponse response = {.data = NULL, .size = 0, .status = fetch->status};

    FetchUserData *userData = (FetchUserData *)fetch->userData;
    userData->callback(response);

    free(userData);
    emscripten_fetch_close(fetch);
}

void HttpGet(const char *url, HttpCallback callback)
{
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    attr.onsuccess = on_fetch_success;
    attr.onerror = on_fetch_error;

    FetchUserData *userData = (FetchUserData *)malloc(sizeof(FetchUserData));
    userData->callback = callback;
    attr.userData = userData;

    emscripten_fetch(&attr, url);
}

#endif // PLATFORM_WEB