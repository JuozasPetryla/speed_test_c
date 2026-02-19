#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <cjson/cJSON.h>
#include <curl/curl.h>
#include "speed_test.h"

int main() {
    char *filename = "data/speedtest_server_list.json";

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("Could not open file %s\n", filename);
        exit(1);
    }

    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *buffer = (char*)malloc((size_t)len + 1);
    fread(buffer, sizeof(char), len, fp);
    fclose(fp);
    buffer[len] = '\0';

    cJSON *servers = NULL;
    cJSON *server = NULL;

    servers = cJSON_Parse(buffer);

    if (!servers) {
        printf("Could not parse JSON");
        exit(1);
    }
    server = cJSON_GetArrayItem(servers, 1);
    cJSON *host = cJSON_GetObjectItemCaseSensitive(server, "host");
    char *host_url = host->valuestring;
    
    CURL *handle = curl_easy_init();
    
    double speedGet = speed_test(handle, DOWNLOAD, host_url);
    printf("GET REQUEST SPEED %.2f Mb/s\n", speedGet);

    double speedPost = speed_test(handle, UPLOAD, host_url);
    printf("POST REQUEST SPEED %.2f Mb/s\n", speedPost);
   // cJSON_ArrayForEach(server, servers)
   // {
   //     cJSON *country = cJSON_GetObjectItemCaseSensitive(server, "country");
   //     printf("Country: %s\n", country->valuestring);
   // }

    exit(0);
}



