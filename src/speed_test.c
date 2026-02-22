#include <curl/curl.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "curl_util.h"
#include "speed_test.h"
#include "file_util.h"
#include "server.h"

#define DOWNLOAD_ENDPOINT "/random4000x4000.jpg"
#define UPLOAD_ENDPOINT "/upload"
#define POST_SIZE (15LL * 1024 * 1024 * 1024 * 100)

bool _get_speed_test_params(SPEED_TEST_TYPE type, CURLINFO *info, const char **endpoint);
FILE* _set_specific_opts(SPEED_TEST_TYPE type, CURL *handle);

static double calculate_speed_megabits(curl_off_t speed) {
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

double speed_test(CURL *handle, SPEED_TEST_TYPE type, const char *host_url)
{
    char url[1024];
    curl_off_t speed;
    CURLINFO speed_type;
    const char *endpoint;

    bool params_ok = _get_speed_test_params(type, &speed_type, &endpoint);

    if (!params_ok) return 0.0;
    
    snprintf(url, sizeof(url), "%s%s", host_url, endpoint);

    set_common_opts(handle, url);
    curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 0L);
    FILE *fp = _set_specific_opts(type, handle);

    CURLcode res_code = perform_request_safe_ignore_timeout(handle);
 
    res_code = get_info_safe(handle, speed_type, res_code, &speed);

    if (fp) fclose(fp);

    return calculate_speed_megabits(speed);
}

Server* best_server_by_location(
    CURL *handle, 
    Server server_list[], 
    int server_count, 
    Location *location
)
{
    if (!location || (!location->city && !location->country)) {
        printf("No location provided");
        return NULL;
    }

    double max = 0.0;
    Server *best_server = NULL;

    for (int i = 0; i < server_count; i++) {
        Server *server = &server_list[i];
        double max_curr = 0.0;
        if (
            location->city == server->location->city ||
            location->country == server->location->country
        ) {
            max_curr += speed_test(handle, DOWNLOAD, server->host);
            max_curr += speed_test(handle, UPLOAD, server->host);
        }
        if (max_curr > max) {
            max = max_curr;
            best_server = server;
        }
    }

    return best_server;
}


