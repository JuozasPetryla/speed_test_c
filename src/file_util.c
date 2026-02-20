#include <stdio.h>
#include <stdlib.h>

FILE* _open_file_safe(const char *filename, const char *mode)
{
    FILE *fp = fopen(filename, mode);
    if (!fp) {
        printf("Could not open file %s\n", filename);
        exit(1);
    }
    return fp;
}
