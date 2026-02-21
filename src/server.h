#ifndef SERVER_H
#define SERVER_H

#include "location.h"

typedef struct {
	Location* location;
	char* host;
	int id;
} Server;

Server* create_server(
	const char* country, 
	const char* city, 
	const char* provider, 
	const char* host, 
	int id
);

void destroy_server(Server* server);

#endif
