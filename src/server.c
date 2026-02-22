#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "location.h"
#include "server.h"

ServerArray* init_server_array(size_t init_capacity)
{
    ServerArray *array = malloc(sizeof(ServerArray));

    printf("Size of init server array %d\n",sizeof(Server*) * init_capacity); 
    array->server = malloc(sizeof(Server*) * init_capacity);
    array->size = 0;
    array->capacity = init_capacity;
    printf("Array initialized\n");

    return array;
}

void append_to_server_array(
    ServerArray *array,
    const char *country,
    const char *city,
    const char *provider,
    const char *host,
    int id
)
{
    if (array->size == array->capacity) {
        array->capacity *= 2;
        array->server = realloc(array->server, array->capacity * sizeof(Server*));
    }

    Server *s = create_server(country, city, provider, host, id);

    printf("Size of server ptr%d\n",sizeof(Server*)); 
    array->server[array->size] = s;
    array->size++;
}

void destroy_server_array(ServerArray *array)
{
    if (!array) return;

    for (int i = 0; i < (int)array->size; i++) {
        destroy_server(array->server[i]);
    }

    free(array->server);
    free(array);
}

Server* create_server(
	const char *country, 
	const char *city, 
	const char *provider,
	const char *host,
	int id
)
{
    Server *server = (Server*)malloc(sizeof(Server));

    server->location = create_location(country, city, provider);
    server->host = (char*)malloc(strlen(host) + 1);
    server->id = id;

    strcpy(server->host, host);

    return server;
}

void destroy_server(Server *server)
{
    if (!server) return;

    destroy_location(server->location);
    free(server->host);

    free(server);
}

