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
Location* _find_location(CURL *handle);
Location* _set_location(CURL *handle, const char *city, const char *country);
void _perform_speed_test(CURL *handle, SPEED_TEST_TYPE type, ServerArray* servers, const char *city, const char *country);
Server* _find_best_server_by_location(CURL *handle, ServerArray* servers, const char *city, const char *country);

typedef struct {
    bool a_flag;
    bool l_flag;
    bool d_flag;
    bool c_flag;
    bool b_flag;
    bool u_flag;
    bool C_flag;
} OptFlags;

int main(int argc, char *argv[]) 
{
    OptFlags opt_flags = { false, false, false, false, false, false, false };

    int opt;
    const char *country = NULL;
    const char *city = NULL;
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
        Location *location = _find_location(handle);
        destroy_location(location);
    }


    if (opt_flags.d_flag) {
        _perform_speed_test(handle, DOWNLOAD, servers, city, country);
    }

    if (opt_flags.u_flag) {
        _perform_speed_test(handle, UPLOAD, servers, city, country);
    }

    if (opt_flags.b_flag) {
        Server *best_server = _find_best_server_by_location(handle, servers, city, country); 
        destroy_server(best_server);
    }

    destroy_server_array(servers);

    exit(0);
}

Server* _find_best_server_by_location(CURL *handle, ServerArray* servers, const char *city, const char *country)
{
    Location *location = _set_location(handle, city, country);
    Server* best_server = best_server_by_location(handle, servers, location);
    printf("Best server in location %s, %s - %s\n", location->city, location->country, best_server->host);
    return best_server;
}

void _perform_speed_test(CURL *handle, SPEED_TEST_TYPE type, ServerArray* servers, const char *city, const char *country)
{
    Server *server = _find_best_server_by_location(handle, servers, city, country);
    char *host_url = server->host;
    double speed = speed_test(handle, type, host_url);
    printf("BLANK_FILL_TYPE speed for host %s - %.2f Mb/s\n", host_url, speed);
}

Location* _set_location(CURL *handle, const char *city, const char *country)
{
    Location *location;
    if (city || country)
    {
        location = create_location(country, city);
    } else {
        location = _find_location(handle);
    }
    
    return location;
}

Location* _find_location(CURL *handle)
{
    printf("Searching for user location by IP address...");
    Location *location = find_location(handle);
    printf(
        "User location: City - %s; Country - %s\n", 
        location->city, 
        location->country
    );

    return location;
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
        cJSON *host = cJSON_GetObjectItemCaseSensitive(server_json, "host");
        cJSON *id = cJSON_GetObjectItemCaseSensitive(server_json, "id");

        append_to_server_array(
                servers, 
                country->valuestring, 
                city->valuestring, 
                host->valuestring, 
                id->valueint
        );
    }

    return servers;
}


