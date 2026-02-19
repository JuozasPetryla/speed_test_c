#include <curl/curl.h>
#include <curl/easy.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "curl_util.h"
#include "speed_test.h"

#define DOWNLOAD_ENDPOINT "/random4000x4000.jpg"
#define UPLOAD_ENDPOINT "/upload"
#define POST_SIZE 100000000

double _download_test(CURL *handle, const char *host_url);
double _upload_test(CURL *handle, const char *host_url);

static double calculate_speed_megabits(curl_off_t speed) {
    return speed / 1024.0 / 1024.0 * 8;
}

double speed_test(CURL *handle, SPEED_TEST_TYPE type, const char *host_url)
{
    switch (type) {
        case DOWNLOAD: 
            return _download_test(handle, host_url);
        case UPLOAD:
            return _upload_test(handle, host_url);
        default:
            printf("Invalid speed test type of %d\n", type);
            return 0L;
    }	
}


double _download_test(CURL *handle, const char *host_url)
{
    char *filename = "/dev/null";
    char url[1024];
    curl_off_t speed = 0.0;
    snprintf(url, sizeof(url), "%s%s", host_url, DOWNLOAD_ENDPOINT);

    FILE *fp = fopen(filename, "w");
    if (!fp) {
        printf("Could not open file %s\n", filename);
        fclose(fp);
        exit(1);
    }

    set_common_opts(handle, url);

    curl_easy_setopt(handle, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, fp);

    CURLcode res_code = perform_request_safe_ignore_timeout(handle);
 
    res_code = get_info_safe(handle, CURLINFO_SPEED_DOWNLOAD_T, res_code, &speed);

    return calculate_speed_megabits(speed);
}

double _upload_test(CURL *handle, const char *host_url)
{
    char *filename = "/dev/zero";
    char url[1024];
    curl_off_t speed = 0.0;
    snprintf(url, sizeof(url), "%s%s", host_url, UPLOAD_ENDPOINT);

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("Could not open file %s\n", filename);
        fclose(fp);
        exit(1);
    }

    char *payload = (char*)malloc((size_t)POST_SIZE + 1);
    fread(payload, sizeof(char), POST_SIZE, fp);
    fclose(fp);
    payload[POST_SIZE] = '\0';

    set_common_opts(handle, url);

    curl_easy_setopt(handle, CURLOPT_POST, 1L);
    curl_easy_setopt(handle, CURLOPT_POSTFIELDS, payload);

    CURLcode res_code = perform_request_safe_ignore_timeout(handle);
    
    res_code = get_info_safe(handle, CURLINFO_SPEED_UPLOAD_T, res_code, &speed);

    return calculate_speed_megabits(speed);
}

