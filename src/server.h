#ifndef SERVER_H
#define SERVER_H

#include "location.h"

typedef struct {
	Location *location;
	char *host;
	int id;
} Server;

typedef struct {
	Server **server;
	size_t size;
	size_t capacity;
} ServerArray;

extern Server* create_server(
	const char *country, 
	const char *city, 
	const char *host, 
	int id
);
extern void append_to_server_array(
    	ServerArray *array,
	const char *country, 
	const char *city, 
	const char *host,
	int id
);
extern ServerArray* init_server_array(size_t init_capacity);
extern void destroy_server(Server* server);
extern void destroy_server_array(ServerArray *array);

#endif
