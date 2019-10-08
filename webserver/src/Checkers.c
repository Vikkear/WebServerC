#include "../include/Checkers.h"
#include "../include/Headers.h"


char *unSupported[20] = {"POST", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE", "PATCH"};


int checkVersion(char *version)
{
    //NOTE: If client doesn't add newline version can get fucked
    if (strstr(version, "HTTP/1.0") != NULL)
    {
        return 1;
    }
    return 0;
}

int checkUnsuppotedMethod(int sd, char* method, char* rootDir, char* request){
    // Check if unsupported method
    for(int i = 0; i < 7; i++){
        if(strcmp(method, unSupported[i]) == 0){
            handleNotImplemented(sd, rootDir, request, 1);
            return 1;
        }
    }
    return 0;
}

FILE *checkFile(int sd, char* rootDir, char *fileName, char* request, int incBody)
{
    char fullPath[MAX_PATH_STR];
    strcpy(fullPath, rootDir);
    strcat(fullPath, fileName);

    char fullFilename[1024];
    char *res = realpath(fullPath, fullFilename);

    FILE *file;
    char path[MAX_PATH_STR] = "";

    if (!strncmp(rootDir, fullFilename, strlen(rootDir)) == 0){
        // Outside of root dir, 403
        handleForbiddenRequest(sd, rootDir, request, incBody);
        closeConnection(sd);
    }

    int rPermission = access(fullFilename, R_OK);

    if (rPermission != 0 && access(fullFilename, F_OK) == 0){
        handleForbiddenRequest(sd, rootDir, request, incBody);
        closeConnection(sd);
    }

    file = fopen(fullFilename, "r");
    if (file)
    {
        return file;
    }
    else
    {
        handleFileNotFound(sd, rootDir, request, incBody);
        closeConnection(sd);
        return NULL;
    }
}
