#include <stdlib.h>
#include <string.h>
#include "location.h"
#include "server.h"

Server* create_server(const char* country, const char* city, const char* provider)
{
    Location* location = (Location*)malloc(sizeof(Location));

    location->country = (char*)malloc(sizeof(strlen(country) + 1));
    location->city = (char*)malloc(sizeof(strlen(city) + 1));
    location->provider = (char*)malloc(sizeof(strlen(provider) + 1));

    strcpy(location->country, country);
    strcpy(location->city, city);
    strcpy(location->provider, provider);

    return location;
}

void destroy_location(Location* location)
{
    if (!location) return;

    free(location->country);
    free(location->city);
    free(location->provider);

    free(location);
}

