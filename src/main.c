#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <cjson/cJSON.h>
#include <curl/curl.h>
#include "server.h"
#include "speed_test.h"
#include "location.h"
#include "file_util.h"

int main() {
    char *filename = "data/speedtest_server_list.json";
     
    FILE *fp = _open_file_safe(filename, "r");

    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *buffer = (char*)malloc((size_t)len + 1);
    fread(buffer, sizeof(char), len, fp);
    fclose(fp);
    buffer[len] = '\0';

    cJSON *servers_json = NULL;
    cJSON *server_json = NULL;
    ServerArray *servers = init_server_array(128);
    if (!servers) {
        printf("Failed to initialize server array");
    }

    servers_json = cJSON_Parse(buffer);
    if (!servers_json) {
        printf("Could not parse JSON\n");
        exit(1);
    }

    cJSON_ArrayForEach(server_json, servers_json)
    {
        cJSON *country = cJSON_GetObjectItemCaseSensitive(server_json, "country");
        cJSON *city = cJSON_GetObjectItemCaseSensitive(server_json, "city");
        cJSON *provider = cJSON_GetObjectItemCaseSensitive(server_json, "provider");
        cJSON *host = cJSON_GetObjectItemCaseSensitive(server_json, "host");
        cJSON *id = cJSON_GetObjectItemCaseSensitive(server_json, "id");

        append_to_server_array(
                servers, 
                country->valuestring, 
                city->valuestring, 
                provider->valuestring, 
                host->valuestring, 
                id->valueint
        );
    }
    
    CURL *handle = curl_easy_init();
    Location *location = find_location(handle);
    printf("User location: city %s; country %s; provider %s\n", location->city, location->country, location->provider);
    
    Server* best_server = best_server_by_location(handle, servers, location);
    printf("Best server for %s -> host %s; id %d", location->city, best_server->host, best_server->id);


    exit(0);
}



