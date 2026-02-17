#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <cjson/cJSON.h>
#include <curl/curl.h>

void get_request(CURL *handle, char *host_url);

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
    printf("Host: %s\n", host_url);
    
    CURL *handle = curl_easy_init();
    get_request(handle, host_url);
   // cJSON_ArrayForEach(server, servers)
   // {
   //     cJSON *country = cJSON_GetObjectItemCaseSensitive(server, "country");
   //     printf("Country: %s\n", country->valuestring);
   // }

    exit(0);
}

void get_request(CURL *handle, char *host_url)
{
    curl_easy_reset(handle);
    curl_easy_setopt(handle, CURLOPT_URL, host_url);
    curl_easy_setopt(handle, CURLOPT_HTTPGET, 1L);
    CURLcode success = curl_easy_perform(handle);
    printf("Something %d\n", success);
}


