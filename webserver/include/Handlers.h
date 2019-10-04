#ifndef HANDLERS
#define HANDLERS

#include "includer.h"

int handleRequest(int sd_current, char* rootDir);
int handleGET(int sd,char* rootDir,  char *path);
int handleHEAD(int sd, char* rootDir, char *path);
int handleBadRequest(int sd, char* rootDir);
void handleFileNotFound(int sd, char* rootDir);
void handleForbiddenRequest(int sd, char* rootDir);
void handleNotImplemented(int sd, char* rootDir);
void handleInternalServerError(int sd, char* rootDir);
void handleFaultyRequest(int sd, char* rootDir, int code, char* fileName);
void sendWithFile(int sd, char* fileContent, char* rootDir, char* path);

#endif
