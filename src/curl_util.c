#include <curl/curl.h>
#include <curl/easy.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "curl_util.h"

#define CURL_USER_AGENT "curl/8.15.0"
#define CURL_TIMEOUT 2

static size_t _write_callback(char *data, size_t size, size_t nmemb, void *user_data)
{
  size_t realsize = nmemb * size;
  Response *res = (Response*)user_data;
 
  char *ptr = realloc(res->response_body, res->size + realsize + 1);
  if(!ptr)
    return 0;
 
  res->response_body = ptr;
  memcpy(&(res->response_body[res->size]), data, realsize);
  res->size += realsize;
  res->response_body[res->size] = 0;
 
  return realsize;
}

void set_get_request_opts_file(CURL *handle, FILE *fp)
{
    curl_easy_setopt(handle, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, fp);
}

void set_get_request_opts(CURL *handle, void *ptr)
{
    curl_easy_setopt(handle, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, _write_callback);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, ptr);
}

void set_post_request_opts_file(CURL *handle, FILE *fp, curl_off_t size)
{
    curl_easy_setopt(handle, CURLOPT_POST, 1L);
    curl_easy_setopt(handle, CURLOPT_READDATA, fp);
    curl_easy_setopt(handle, CURLOPT_POSTFIELDSIZE_LARGE, size);
}

void set_common_opts(CURL *handle, char *url)
{
    if (!handle) {
        perror("Invalid curl_easy handle; exiting");
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
    if (CURLE_OK != res_code) {
        fprintf(stderr, "Failed to get download speed: %s\n", curl_easy_strerror(res_code));
    }

    return res_code;
} 

CURLcode perform_request_safe(CURL *handle)
{
    CURLcode res_code = curl_easy_perform(handle);
    if (CURLE_OK != res_code) {
        fprintf(stderr, "Failed to make a GET request with CURL: %s\n", curl_easy_strerror(res_code));
        return res_code;
    }

    return CURLE_OK;
}

CURLcode perform_request_safe_ignore_timeout(CURL *handle)
{
    CURLcode res_code = curl_easy_perform(handle);
    if (CURLE_OPERATION_TIMEOUTED != res_code && CURLE_OK != res_code) {
        fprintf(stderr, "Failed to make a GET request with CURL: %s\n", curl_easy_strerror(res_code));
        return res_code;
    }

    return CURLE_OK;
}

