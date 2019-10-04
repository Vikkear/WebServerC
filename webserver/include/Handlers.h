#ifndef HANDLERS
#define HANDLERS

#include "includer.h"

int handleRequest(int sd_current, char* rootDir);
int handleGET(int sd,char* rootDir,  char *path);
int handleHEAD(int sd, char* rootDir, char *path);
int handleBadRequest(int sd, char* rootDir);
void handleFileNotFound(int sd, char* rootDir);
void handleForbiddenRequest(int sd, char* rootDir);
void sendWithFile(int sd, char* fileContent, char* path);

#endif
