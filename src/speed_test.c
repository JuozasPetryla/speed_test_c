#include <curl/curl.h>
#include <float.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "curl_util.h"
#include "speed_test.h"
#include "file_util.h"
#include "server.h"

#define DOWNLOAD_ENDPOINT "/random4000x4000.jpg"
#define UPLOAD_ENDPOINT "/upload"
#define POST_SIZE (15LL * 1024 * 1024 * 1024 * 100)
#define LATENCY_TIMEOUT 1

bool _get_speed_test_params(SPEED_TEST_TYPE type, CURLINFO *info, const char **endpoint);
FILE* _set_specific_opts(SPEED_TEST_TYPE type, CURL *handle);
CURLcode _get_server_latency(CURL *handle, char *host_url, curl_off_t *latency);

static curl_off_t calculate_speed_megabits(curl_off_t speed) {
    return speed / 1024.0 / 1024.0 * 8;
}

bool _get_speed_test_params(SPEED_TEST_TYPE type, CURLINFO *info, const char **endpoint)
{
    switch (type) {
        case DOWNLOAD:
            *info = CURLINFO_SPEED_DOWNLOAD_T;
            *endpoint = DOWNLOAD_ENDPOINT;
            return true;
        case UPLOAD:
            *info = CURLINFO_SPEED_UPLOAD_T;
            *endpoint = UPLOAD_ENDPOINT;
            return true;
        default:
            return false;
    }
}

FILE* _set_specific_opts(SPEED_TEST_TYPE type, CURL *handle)
{
    FILE *fp = NULL;
    switch (type) {
        case DOWNLOAD:
            fp = _open_file_safe("/dev/null", "wb");
            set_get_request_opts_file(handle, fp);
            break;
        case UPLOAD:
            fp = _open_file_safe("/dev/zero", "rb");
            set_post_request_opts_file(handle, fp, (curl_off_t)POST_SIZE);
            break;
        default:
            break;
    }
    return fp;
}

CURLcode _get_server_latency(CURL *handle, char *host_url, curl_off_t *latency)
{
    FILE *fp = _open_file_safe("/dev/null", "wb");

    set_common_opts(handle, host_url);
    curl_easy_setopt(handle, CURLOPT_TIMEOUT, LATENCY_TIMEOUT);
    set_get_request_opts_file(handle, fp);

    CURLcode res_code = perform_request_safe(handle);
    if (CURLE_OK == res_code) {
        res_code = get_info_safe(handle, CURLINFO_TOTAL_TIME_T, res_code, latency);
    }

    return res_code;
}

CURLcode speed_test(CURL *handle, SPEED_TEST_TYPE type, const char *host_url, curl_off_t *speed)
{
    char url[1024];
    CURLINFO speed_type;
    const char *endpoint;

    bool params_ok = _get_speed_test_params(type, &speed_type, &endpoint);
    if (!params_ok) return CURLE_FAILED_INIT;
    
    snprintf(url, sizeof(url), "%s%s", host_url, endpoint);

    set_common_opts(handle, url);
    curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 0L);

    FILE *fp = _set_specific_opts(type, handle);

    CURLcode res_code = perform_request_safe_ignore_timeout(handle);
    if (CURLE_OK == res_code) {
        res_code = get_info_safe(handle, speed_type, res_code, speed);
    }

    if (fp) fclose(fp);

    *speed = calculate_speed_megabits(*speed);

    return res_code;
}

Server* best_server_by_location(CURL *handle, ServerArray *server_array, Location *location)
{
    double min = DBL_MAX;
    Server *best_server = NULL;

    for (int i = 0; i < (int)server_array->size; i++) {
        Server *server = server_array->server[i];
        curl_off_t min_curr = 0.0;
        if (
            (location->city && strcmp(location->city, server->location->city) == 0) ||
            (location->country && strcmp(location->country, server->location->country) == 0)
        ) {
            CURLcode res_code = _get_server_latency(handle, server->host, &min_curr);
            if (CURLE_OK != res_code) {
                printf("    Hostname - %s, continuing to other\n", server->host);
                continue;
            };

            if (min_curr && min_curr < min) {
                min = min_curr;
                best_server = server;
            }
        }
    }

    return best_server;
}


