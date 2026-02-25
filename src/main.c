#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <cjson/cJSON.h>
#include <curl/curl.h>
#include <string.h>
#include <unistd.h>
#include "server.h"
#include "speed_test.h"
#include "location.h"
#include "file_util.h"

#define INIT_SERVER_ARRAY_SIZE 8192
#define SERVER_LIST_FILENAME "data/speedtest_server_list.json"

ServerArray* _parse_server_list();
Location* _find_location(CURL *handle);
void _perform_speed_test(CURL *handle, SPEED_TEST_TYPE type, const char *host);
char* _find_best_host_by_location(CURL *handle, Location *location);
char* _get_speed_type_name(SPEED_TEST_TYPE type);
void _print_server_finding_message(Location *location);
void _print_best_server_message(Location *location, char *host_copy);
void _print_available_options();

typedef struct {
    bool l_flag;
    bool d_flag;
    bool c_flag;
    bool b_flag;
    bool u_flag;
    bool C_flag;
    bool H_flag;
} OptFlags;

int main(int argc, char *argv[]) 
{
    OptFlags opt_flags = { false, false, false, false, false, false, false };

    int opt;
    const char *country = NULL;
    const char *city = NULL;
    const char *host = NULL;
    while((opt = getopt(argc, argv, "ahlbduC:c:H:")) != -1) 
    { 
        switch(opt) 
        { 
            case 'a':
                opt_flags.l_flag = true;
                opt_flags.d_flag = true;
                opt_flags.u_flag = true;
                opt_flags.b_flag = true;
                break;
            case 'd':
                opt_flags.l_flag = true;
                opt_flags.d_flag = true;
                opt_flags.b_flag = true;
                break;
            case 'u':
                opt_flags.l_flag = true;
                opt_flags.u_flag = true;
                opt_flags.b_flag = true;
                break;
            case 'b':
                opt_flags.l_flag = true;
                opt_flags.b_flag = true;
                break;
            case 'l':
                opt_flags.l_flag = true;
                break;
            case 'C':
                country = optarg;
                opt_flags.C_flag = true;
                break;
            case 'c':
                city = optarg;
                opt_flags.c_flag = true;
                break;
            case 'H':
                host = optarg;
                opt_flags.H_flag = true;
                break;
            case 'h':
                _print_available_options();
                exit(0);
            case ':': 
                printf("Option requires an argument; exiting\n");
                exit(0); 
            case '?': 
                printf("Unrecognized option; exiting\n");
                exit(0); 
        } 
    }

    Location *location = NULL;

    CURL *handle = curl_easy_init();

    if (opt_flags.l_flag && !(opt_flags.C_flag || opt_flags.c_flag || opt_flags.H_flag)) {
        location = _find_location(handle);
    } else {
        location = create_location(country, city);
    }

    if (opt_flags.b_flag && !opt_flags.H_flag) {
        host = _find_best_host_by_location(handle, location);
    }

    if (opt_flags.d_flag) {
        _perform_speed_test(handle, DOWNLOAD, host);
    }

    if (opt_flags.u_flag) {
        _perform_speed_test(handle, UPLOAD, host);
    }

    exit(0);
}

void _perform_speed_test(CURL *handle, SPEED_TEST_TYPE type, const char *host_url)
{
    curl_off_t speed = 0.0;
    CURLcode res_code = speed_test(handle, type, host_url, &speed);
    if (CURLE_OK != res_code) {
        fprintf(
            stderr, 
            "%s speed test failed for host %s\n", 
            _get_speed_type_name(type), 
            host_url
        );
    }
    printf(
        "--> %s speed for host %s - %.2f Mb/s\n", 
        _get_speed_type_name(type), 
        host_url, 
        (double)speed
    );
}

char* _find_best_host_by_location(CURL *handle, Location *location)
{
    ServerArray *servers = _parse_server_list();
    if (!servers) {
        perror("Failed to parse servers list\n");
        exit(1);
    }

    _print_server_finding_message(location);
    Server* best_server = best_server_by_location(handle, servers, location);
    if (!best_server) {
        printf("No server found for location\n");
        destroy_server_array(servers);
        exit(1);
    }
    char *host_copy = strdup(best_server->host);   
    destroy_server_array(servers);

    _print_best_server_message(location, host_copy);
    return host_copy;
}

Location* _find_location(CURL *handle)
{
    printf("Searching for user location by IP address...\n");

    Location *location = find_location(handle);
    if (!location || (!location->city && !location->country)) {
        perror("Location not found\n");
        exit(1);
    }

    printf(
        "--> User location: City - %s; Country - %s\n", 
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
        perror("Failed to initialize server array\n");
        exit(1);
    }

    servers_json = cJSON_Parse(buffer);
    if (!servers_json) {
        perror("Could not parse JSON\n");
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

void _print_server_finding_message(Location *location)
{
    if (location->city && location->country) {
        printf("Finding the best server in %s, %s...\n", location->city, location->country);
        return;
    }

    if (location->city && !location->country) {
        printf("Finding the best server in %s...\n", location->city);
        return;
    }

    if (!location->city && location->country) {
        printf("Finding the best server in %s...\n", location->country);
        return;
    }
}

void _print_best_server_message(Location *location, char *host_copy)
{
    if (location->city && location->country) {
        printf("--> Best server in location %s, %s - %s\n", location->city, location->country, host_copy);
        return;
    }

    if (location->city && !location->country) {
        printf("--> Best server in location %s - %s\n", location->city, host_copy);
        return;
    }

    if (!location->city && location->country) {
        printf("--> Best server in location %s - %s\n", location->country, host_copy);
        return;
    }
}

void _print_available_options()
{
    printf("Available options are:\n");
    printf("    - a -> Run full test on user's current location\n");
    printf("    - d -> Run download speed test on user's current location\n");
    printf("    - u -> Run upload test on user's current location\n");
    printf("    - b -> Find the best server in user's current location\n");
    printf("    - l -> Find user's current location\n");
    printf("    - c -> Specify a city instead of user's location when running a test\n");
    printf("    - C -> Specify a country instead of user's location when running a test\n");
    printf("    - H -> Specify a hostname instead of the best host for specified location\n");
}

char* _get_speed_type_name(SPEED_TEST_TYPE type)
{
    switch (type) {
        case DOWNLOAD:
            return "Download";
        case UPLOAD:
            return "Upload";
        default:
            return "Default";
    }
}

