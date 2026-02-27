#ifndef LOCATION_H
#define LOCATION_H

#include <curl/curl.h>

typedef struct {
	char* country;
	char* city;
} Location;

extern Location* find_location(CURL *handle);
extern Location* create_location(const char *country, const char *city);
extern void destroy_location(Location* location);

#endif
