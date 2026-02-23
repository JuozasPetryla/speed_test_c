#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <cjson/cJSON.h>
#include <curl/curl.h>
#include <unistd.h>
#include "server.h"
#include "speed_test.h"
#include "location.h"
#include "file_util.h"

#define INIT_SERVER_ARRAY_SIZE 8192
#define SERVER_LIST_FILENAME "data/speedtest_server_list.json"

ServerArray* parse_server_list();

typedef struct {
    bool l_flag;
    bool d_flag;
    bool c_flag;
    bool b_flag;
    bool u_flag;
    bool C_flag;
} OptFlags;

int main(int argc, char *argv[]) 
{
    OptFlags opt_flags = { false, false, false, false, false, false };

    int opt;
    while((opt = getopt(argc, argv, "hlbC:c:d::u::")) != -1) 
    { 
        switch(opt) 
        { 
            case 'd':
                opt_flags.d_flag = true;
                break;
            case 'u':
                opt_flags.u_flag = true;
                break;
            case 'C':
                opt_flags.C_flag = true;
                break;
            case 'c':
                opt_flags.c_flag = true;
                break;
            case 'b':
                opt_flags.b_flag = true;
                break;
            case 'l':
                opt_flags.l_flag = true;
                break;
            case 'h':
                printf("Available options are hlb:d:u:");
                break;
            case ':': 
                printf("Option requires an argument");
                break; 
            case '?': 
                printf("Unrecognized option");
                break; 
        } 
    }

    
    ServerArray *servers = parse_server_list();

    CURL *handle = curl_easy_init();
    Location *location = find_location(handle);
    printf(
        "User location: City - %s; Country - %s; Internet service provider - %s\n", 
        location->city, 
        location->country, 
        location->provider
    );
    
    Server* best_server = best_server_by_location(handle, servers, location);
    printf("Best server for %s -> host %s; id %d\n", location->city, best_server->host, best_server->id);


    destroy_server_array(servers);

    exit(0);
}

ServerArray* parse_server_list()
{
    FILE *fp = _open_file_safe(SERVER_LIST_FILENAME, "r");

    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *buffer = (char*)malloc((size_t)len + 1);
    fread(buffer, sizeof(char), len, fp);
    fclose(fp);
    buffer[len] = '\0';

    cJSON *servers_json = NULL;
    cJSON *server_json = NULL;
    
    ServerArray *servers = init_server_array(INIT_SERVER_ARRAY_SIZE);
    if (!servers) {
        printf("Failed to initialize server array");
        exit(1);
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

}


