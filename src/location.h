#ifndef LOCATION_H
#define LOCATION_H

#include <curl/curl.h>

typedef struct {
	char *country;
	char *city;
	char *provider;
} Location;

Location* find_location(CURL *handle);
Location* create_location(const char *country, const char *city, const char *provider);
void destroy_location(Location *location);

#endif
