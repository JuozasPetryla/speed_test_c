#include <curl/curl.h>
#include <curl/easy.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define CURL_USER_AGENT "curl/8.15.0"
#define CURL_TIMEOUT 15


void set_common_opts(CURL *handle, char *url)
{
    if (!handle) {
        printf("Invalid curl_easy handle; exiting");
        exit(1);
    };

    curl_easy_reset(handle);

    curl_easy_setopt(handle, CURLOPT_URL, url);
    curl_easy_setopt(handle, CURLOPT_USERAGENT, CURL_USER_AGENT);
    curl_easy_setopt(handle, CURLOPT_TIMEOUT, CURL_TIMEOUT);
}

CURLcode get_info_safe(CURL *handle, CURLINFO speed_type, CURLcode res_code, void *info)
{
    res_code = curl_easy_getinfo(handle, speed_type, info);
    if (res_code != CURLE_OK) {
        printf("Failed to get download speed: %s\n", curl_easy_strerror(res_code));
    }

    return res_code;
} 

CURLcode perform_request_safe_ignore_timeout(CURL *handle)
{
    CURLcode res_code = curl_easy_perform(handle);
    if (CURLE_OPERATION_TIMEOUTED != res_code && CURLE_OK != res_code) {
        printf("Failed to make a GET request with CURL: %s\n", curl_easy_strerror(res_code));
        return res_code;
    }

    return CURLE_OK;
}

