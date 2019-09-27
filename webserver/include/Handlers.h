#ifndef HANDLERS
#define HANDLERS

#define BUFSIZE 512
#define MAX_PATH_STR 80
#include "test.h"

int handleRequest(int sd_current, char* rootDir);
int handleGET(int sd,char* rootDir,  char *path);
int handleHEAD(int sd, char* rootDir, char *path);
int handleBadRequest(int sd);
void handleFileNotFound(int sd);
void handleForbiddenRequest(int sd);

#endif
