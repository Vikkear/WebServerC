#ifndef HANDLERS
#define HANDLERS

#include "includer.h"

int handleRequest(int sd_current, char* rootDir);
int handleGET(int sd,char* rootDir,  char *path);
int handleHEAD(int sd, char* rootDir, char *path);
int handleBadRequest(int sd, char* rootDir, char* request);
void handleFileNotFound(int sd, char* rootDir, char* request);
void handleForbiddenRequest(int sd, char* rootDir, char* request);
void handleNotImplemented(int sd, char* rootDir, char* request);
void handleInternalServerError(int sd, char* rootDir, char* request);
int handleFaultyRequest(int sd, char* rootDir, int code, char* fileName, char* request);
int sendWithFile(int sd, char* fileContent, char* rootDir, char* path, char* request);
void logToFile(int sd, char* request, char* path, int code, int size);

#endif
