#ifndef CURL_UTIL_H
#define CURL_UTIL_H

#include <curl/curl.h>

typedef struct {
    char *response;
    size_t size;
} Response;

double get_request(CURL *handle, char *host_url);
double post_request(CURL *handle, char *host_url);

#endif
