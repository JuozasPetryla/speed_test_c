#ifndef CURL_UTIL_H
#define CURL_UTIL_H

#include <curl/curl.h>

void set_common_opts(CURL *handle, char *url);
CURLcode get_info_safe(CURL *handle, CURLINFO speed_type, CURLcode res_code, void *info);
CURLcode perform_request_safe_ignore_timeout(CURL *handle);

#endif
