#ifndef SPEED_TEST_H
#define SPEED_TEST_H

#include <curl/curl.h>

typedef enum {
	DOWNLOAD,
	UPLOAD
} SPEED_TEST_TYPE;

typedef struct {
    char *endpoint;
    CURLINFO speed_type;
} SpeedTestParams;

double speed_test(CURL *handle, SPEED_TEST_TYPE type, const char *host_url);

#endif
