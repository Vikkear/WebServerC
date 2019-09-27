#ifndef CHECKERS
#define CHECKERS

#include "test.h"

#define BUFSIZE 512
#define MAX_PATH_STR 80

int checkVersion(char *version);
FILE *checkFile(int sd, char* rootDir, char *fileName);
int checkUnsuppotedMethod(int sd, char* method);


#endif
