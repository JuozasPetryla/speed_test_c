#ifndef SPEED_TEST_H
#define SPEED_TEST_H

#include <curl/curl.h>
#include "server.h"
#include "location.h"

typedef enum {
	DOWNLOAD,
	UPLOAD
} SPEED_TEST_TYPE;

double speed_test(CURL *handle, SPEED_TEST_TYPE type, const char *host_url);
Server* best_server_by_location(CURL *handle, ServerArray *server_array, Location *location);

#endif
