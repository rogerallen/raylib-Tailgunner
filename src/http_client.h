#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

typedef struct {
    char *data;
    long size;
    int status;
} HttpResponse;

typedef void (*HttpCallback)(HttpResponse response);

void HttpGet(const char *url, HttpCallback callback);

#endif // HTTP_CLIENT_H
