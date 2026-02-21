#include <curl/curl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "curl_util.h"
#include "speed_test.h"
#include "file_util.h"

#define DOWNLOAD_ENDPOINT "/random4000x4000.jpg"
#define UPLOAD_ENDPOINT "/upload"
#define POST_SIZE (15LL * 1024 * 1024 * 1024 * 100)
#define LOCATION_API_URL "http://ip-api.com/json/"

SpeedTestParams* _get_speed_test_params(SPEED_TEST_TYPE type);
FILE* _set_specific_opts(SPEED_TEST_TYPE type, CURL *handle);

static double calculate_speed_megabits(curl_off_t speed) {
    return speed / 1024.0 / 1024.0 * 8;
}

SpeedTestParams* _get_speed_test_params(SPEED_TEST_TYPE type)
{
    SpeedTestParams *speed_test_params = (SpeedTestParams*)malloc(sizeof(SpeedTestParams));
    switch (type) {
        case DOWNLOAD:
            speed_test_params->speed_type = CURLINFO_SPEED_DOWNLOAD_T;
            speed_test_params->endpoint = DOWNLOAD_ENDPOINT;
            break;
        case UPLOAD:
            speed_test_params->speed_type = CURLINFO_SPEED_UPLOAD_T;
            speed_test_params->endpoint = UPLOAD_ENDPOINT;
            break;
        default:
            printf("Unrecognized speed test type %d\n", type);
            free(speed_test_params);
            return NULL;
    }
    return speed_test_params;
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
            printf("Unrecognized speed test type %d\n", type);
            break;
    }
    return fp;
}

double speed_test(CURL *handle, SPEED_TEST_TYPE type, const char *host_url)
{
    SpeedTestParams *speed_test_params = _get_speed_test_params(type);

    if (NULL == speed_test_params) return 0.0;
    
    char url[1024];
    curl_off_t speed = 0.0;
    snprintf(url, sizeof(url), "%s%s", host_url, speed_test_params->endpoint);

    set_common_opts(handle, url);
    FILE *fp = _set_specific_opts(type, handle);

    CURLcode res_code = perform_request_safe_ignore_timeout(handle);
 
    res_code = get_info_safe(handle, speed_test_params->speed_type, res_code, &speed);

    free(speed_test_params);
    if (fp) fclose(fp);

    return calculate_speed_megabits(speed);
}

void find_location(CURL *handle)
{
    Response response = { 0 };

    set_common_opts(handle, LOCATION_API_URL);
    set_get_request_opts(handle, (void*)&response);
    
    CURLcode res_code = perform_request_safe_ignore_timeout(handle);

    printf("Response status: %d\n Response text: %s\n", res_code, response.response_body);
}

