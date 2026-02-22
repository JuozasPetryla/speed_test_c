#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cjson/cJSON.h"
#include "curl_util.h"
#include "location.h"

#define LOCATION_API_URL "http://ip-api.com/json/"

Location* create_location(const char *country, const char *city, const char *provider)
{
    Location* location = (Location*)malloc(sizeof(Location));

    location->country = (char*)malloc(strlen(country) + 1);
    location->city = (char*)malloc(strlen(city) + 1);
    location->provider = (char*)malloc(strlen(provider) + 1);

    strcpy(location->country, country);
    strcpy(location->city, city);
    strcpy(location->provider, provider);

    return location;
}

void destroy_location(Location* location)
{
    if (!location) return;

    free(location->country);
    free(location->city);
    free(location->provider);

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
        printf("Could not parse location response body");
        return NULL;
    }

    cJSON *country = cJSON_GetObjectItemCaseSensitive(location_json, "country");
    cJSON *city = cJSON_GetObjectItemCaseSensitive(location_json, "city");
    cJSON *isp = cJSON_GetObjectItemCaseSensitive(location_json, "isp");

    Location *location = create_location(country->valuestring, city->valuestring, isp->valuestring);

    return location;
}


