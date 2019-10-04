#ifndef CHECKERS
#define CHECKERS

#include "includer.h"
#include "Headers.h"
#include "Handlers.h"

int checkVersion(char *version);
FILE *checkFile(int sd, char* rootDir, char *fileName, char* request);
int checkUnsuppotedMethod(int sd, char* method, char* rootDir);


#endif
