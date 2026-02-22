#ifndef SPEED_TEST_H
#define SPEED_TEST_H

#include <curl/curl.h>
#include "server.h"
#include "location.h"

typedef enum {
	DOWNLOAD,
	UPLOAD
} SPEED_TEST_TYPE;

typedef struct {
    const char *endpoint;
    CURLINFO speed_type;
} SpeedTestParams;

double speed_test(CURL *handle, SPEED_TEST_TYPE type, const char *host_url);
Server* best_server_by_location(CURL *handle, Server server_list[], int server_count, Location *location);

#endif
