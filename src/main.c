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

ServerArray* _parse_server_list();

typedef struct {
    bool a_flag;
    bool l_flag;
    bool d_flag;
    bool c_flag;
    bool b_flag;
    bool u_flag;
    bool C_flag;
} OptFlags;

// TODO: Control flow for options
int main(int argc, char *argv[]) 
{
    OptFlags opt_flags = { false, false, false, false, false, false, false };

    int opt;
    char* country;
    char* city;
    while((opt = getopt(argc, argv, "ahlbC:c:d::u::")) != -1) 
    { 
        switch(opt) 
        { 
            case 'a':
                opt_flags.a_flag = true;
                break;
            case 'd':
                opt_flags.d_flag = true;
                break;
            case 'u':
                opt_flags.u_flag = true;
                break;
            case 'C':
                country = optarg;
                opt_flags.C_flag = true;
                break;
            case 'c':
                city = optarg;
                opt_flags.c_flag = true;
                break;
            case 'b':
                opt_flags.b_flag = true;
                break;
            case 'l':
                opt_flags.l_flag = true;
                break;
            case 'h':
                printf("Available options are hlb:d:u:\n");
                break;
            case ':': 
                printf("Option requires an argument\n");
                break; 
            case '?': 
                printf("Unrecognized option\n");
                break; 
        } 
    }

    CURL *handle = curl_easy_init();

    ServerArray *servers = _parse_server_list();

    if (opt_flags.l_flag) {
        Location *location = find_location(handle);
        printf(
            "User location: City - %s; Country - %s; Internet service provider - %s\n", 
            location->city, 
            location->country, 
            location->provider
        );
    }

    // TODO: handle server selection by location, default to current location
    if (opt_flags.d_flag) {
        char* host_url = servers->server[1]->host;
        printf("host_URL\n");
        double speed = speed_test(handle, DOWNLOAD, host_url);
        printf("Download speed for host %s - %.2f Mb/s\n", host_url, speed);
    }

    if (opt_flags.u_flag) {
        char* host_url = servers->server[1]->host;
        double speed = speed_test(handle, UPLOAD, host_url);
        printf("Upload speed for host %s - %.2f Mb/s\n", host_url, speed);
    }

    if (opt_flags.b_flag) {
        Location *location;
        if (opt_flags.c_flag || opt_flags.C_flag) {
            location = create_location(country, city, NULL);
        } else {
            location = find_location(handle);
        }
        Server* best_server = best_server_by_location(handle, servers, location);
        printf("Best server in location %s, %s - %s\n", location->city, location->country, best_server->host);
    }

    destroy_server_array(servers);

    exit(0);
}



ServerArray* _parse_server_list()
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

    return servers;
}


