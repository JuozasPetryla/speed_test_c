#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cjson/cJSON.h"
#include "curl_util.h"
#include "location.h"

#define LOCATION_API_URL "http://ip-api.com/json/"

Location* create_location(const char *country, const char *city)
{
    Location* location = (Location*)malloc(sizeof(Location));

    if (country) {
        location->country = (char*)malloc(strlen(country) + 1);
        strcpy(location->country, country);
    } else {
        location->country = NULL;
    }

    if (city) {
        location->city = (char*)malloc(strlen(city) + 1);
        strcpy(location->city, city);
    } else {
        location->city = NULL;
    }

    return location;
}

void destroy_location(Location* location)
{
    if (!location) return;

    if (location->country) free(location->country);
    if (location->city) free(location->city);

    free(location);
}

Location* find_location(CURL *handle)
{
    Response response = { 0 };

    set_common_opts(handle, LOCATION_API_URL);
    set_get_request_opts(handle, (void*)&response);
    
    CURLcode res_code = perform_request_safe_ignore_timeout(handle);
    if (CURLE_OK != res_code)
    {
        return NULL;
    }

    cJSON* location_json = cJSON_Parse(response.response_body);
    free(response.response_body);
    if (!location_json) {
        perror("Could not parse location response body");
        return NULL;
    }

    cJSON *country = cJSON_GetObjectItemCaseSensitive(location_json, "country");
    cJSON *city = cJSON_GetObjectItemCaseSensitive(location_json, "city");

    Location *location = create_location(country->valuestring, city->valuestring);

    return location;
}


