#ifndef CHECKERS
#define CHECKERS

#include "includer.h"
#include "Headers.h"
#include "Handlers.h"

#define BUFSIZE 512
#define MAX_PATH_STR 80

int checkVersion(char *version);
FILE *checkFile(int sd, char* rootDir, char *fileName);
int checkUnsuppotedMethod(int sd, char* method);


#endif
