#ifndef LOCATION_H
#define LOCATION_H

#include <curl/curl.h>

typedef struct {
	char* country;
	char* city;
} Location;

Location* find_location(CURL *handle);
Location* create_location(const char *country, const char *city);
void destroy_location(Location* location);

#endif
