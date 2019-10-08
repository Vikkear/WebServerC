#ifndef HANDLERS
#define HANDLERS

#include "includer.h"

int handleRequest(int sd_current, char* rootDir);
int handleGET(int sd,char* rootDir,  char *path, char* request);
int handleHEAD(int sd, char* rootDir, char *path, char* request);
int handleBadRequest(int sd, char* rootDir, char* request, int incBody);
void handleFileNotFound(int sd, char* rootDir, char* request, int incBody);
void handleForbiddenRequest(int sd, char* rootDir, char* request, int incBody);
void handleNotImplemented(int sd, char* rootDir, char* request, int incBody);
void handleInternalServerError(int sd, char* rootDir, char* request, int incBody);
int handleFaultyRequest(int sd, char* rootDir, int code, char* fileName, char* request, int incBody);
int sendWithFile(int sd, char* fileContent, char* rootDir, char* path, char* request);
void logToFile(int sd, char* request, int code, int size);

#endif
