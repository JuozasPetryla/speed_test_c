#include <curl/curl.h>
#include <curl/easy.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "curl_util.h"

#define DOWNLOAD_ENDPOINT "/random4000x4000.jpg"
#define UPLOAD_ENDPOINT "/upload"
#define CURL_USER_AGENT "curl/8.15.0"
#define READ_DEV_ZERO_SIZE 100000000
#define CURL_TIMEOUT 15


// TODO: Control both speedtests timeouts
static double calculate_speed_megabits(curl_off_t speed) {
    return speed / 1024.0 / 1024.0 * 8;
}

double get_request(CURL *handle, char *host_url)
{
    char *filename = "/dev/null";
    curl_off_t speed = 0.0;
    char url[1024];
    snprintf(url, sizeof(url), "%s%s", host_url, DOWNLOAD_ENDPOINT);

    FILE *fp = fopen(filename, "w");
    if (!fp) {
        printf("Could not open file %s\n", filename);
        exit(1);
    }
    
    if (!handle) {
        printf("Invalid curl_easy handle");
        return speed;
    };

    curl_easy_reset(handle);

    curl_easy_setopt(handle, CURLOPT_URL, url);
    curl_easy_setopt(handle, CURLOPT_USERAGENT, CURL_USER_AGENT);
    curl_easy_setopt(handle, CURLOPT_TIMEOUT, CURL_TIMEOUT);
    curl_easy_setopt(handle, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, fp);

    CURLcode res_code = curl_easy_perform(handle);
    if (res_code != CURLE_OK) {
        printf("Failed to make a GET request with CURL: %s\n", curl_easy_strerror(res_code));
        return speed;
    }
    
    res_code = curl_easy_getinfo(handle, CURLINFO_SPEED_DOWNLOAD_T, &speed);
    if (res_code != CURLE_OK) {
        printf("Failed to get download speed: %s\n", curl_easy_strerror(res_code));
        return speed;
    }

    return calculate_speed_megabits(speed);
}

double post_request(CURL *handle, char *host_url)
{
    char *filename = "/dev/zero";
    curl_off_t speed = 0.0;
    char url[1024];
    snprintf(url, sizeof(url), "%s%s", host_url, UPLOAD_ENDPOINT);

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("Could not open file %s\n", filename);
        fclose(fp);
        exit(1);
    }

    char *payload = (char*)malloc((size_t)READ_DEV_ZERO_SIZE + 1);
    fread(payload, sizeof(char), READ_DEV_ZERO_SIZE, fp);
    fclose(fp);
    payload[READ_DEV_ZERO_SIZE] = '\0';
    
    if (!handle) {
        printf("Invalid curl_easy handle");
        return speed;
    };

    curl_easy_reset(handle);

    curl_easy_setopt(handle, CURLOPT_URL, url);
    curl_easy_setopt(handle, CURLOPT_USERAGENT, CURL_USER_AGENT);
    curl_easy_setopt(handle, CURLOPT_TIMEOUT, CURL_TIMEOUT);
    curl_easy_setopt(handle, CURLOPT_POST, 1L);
    curl_easy_setopt(handle, CURLOPT_POSTFIELDS, payload);

    CURLcode res_code = curl_easy_perform(handle);
    if (res_code != CURLE_OK) {
        printf("Failed to make a GET request with CURL: %s\n", curl_easy_strerror(res_code));
        return speed;
    }
    
    res_code = curl_easy_getinfo(handle, CURLINFO_SPEED_UPLOAD_T, &speed);
    if (res_code != CURLE_OK) {
        printf("Failed to get download speed: %s\n", curl_easy_strerror(res_code));
        return speed;
    }

    return calculate_speed_megabits(speed);
}





