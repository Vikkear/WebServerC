#ifndef HEADERS
#define HEADERS

#include "includer.h"
#define BUFSIZE 512
#define MAX_PATH_STR 80

void fgenerateHeader(FILE* file, int code, char *path, char* header, int size);
void generateHeader(int code, char* header, int size);

#endif
