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

int checkUnsuppotedMethod(int sd, char* method, char* rootDir){
    // Check if unsupported method
    for(int i = 0; i < 7; i++){
        if(strcmp(method, unSupported[i]) == 0){
            handleNotImplemented(sd, rootDir);
            return 1;
        }
    }
    return 0;
}

FILE *checkFile(int sd, char* rootDir, char *fileName)
{
    FILE *file;
    char path[MAX_PATH_STR] = "";

    if (!strncmp(rootDir, fileName, strlen(rootDir)) == 0){
        // Outside of root dir, 403
        handleForbiddenRequest(sd, rootDir);
        closeConnection(sd);
    }

    int rPermission = access(fileName, R_OK);

    if (rPermission != 0 && access(fileName, F_OK) == 0){
        handleForbiddenRequest(sd, rootDir);
        closeConnection(sd);
    }

    file = fopen(fileName, "r");
    if (file)
    {
        return file;
    }
    else
    {
        handleFileNotFound(sd, rootDir);
        closeConnection(sd);
        return NULL;
    }
}
