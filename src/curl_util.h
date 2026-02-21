#ifndef CURL_UTIL_H
#define CURL_UTIL_H

#include <curl/curl.h>

typedef struct {
	char *response_body;
	size_t size;
} Response;

void set_common_opts(CURL *handle, char *url);
CURLcode get_info_safe(CURL *handle, CURLINFO speed_type, CURLcode res_code, void *info);
CURLcode perform_request_safe_ignore_timeout(CURL *handle);
void set_get_request_opts_file(CURL *handle, FILE *fp);
void set_get_request_opts(CURL *handle, void *ptr);
void set_post_request_opts_file(CURL *handle, FILE *fp, curl_off_t size);
	
#endif
