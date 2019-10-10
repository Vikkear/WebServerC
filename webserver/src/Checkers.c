#include "../include/Checkers.h"
#include "../include/Headers.h"

char *unSupported[20] = {"POST", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE", "PATCH"};

// Checks what HTTP version the call makes
int checkVersion(char *version)
{
    if (strstr(version, "HTTP/1.0") != NULL)
    {
        return 1;
    }
    return 0;
}

// Check if the user sent in a request that is unsupported
int checkUnsuppotedMethod(int sd, char* method, char* rootDir, char* request){
    for(int i = 0; i < 7; i++){
        if(strcmp(method, unSupported[i]) == 0){
            handleNotImplemented(sd, rootDir, request, 1);
            return 1;
        }
    }
    return 0;
}

//Function to check if file is inside the servers root directory and if we have correct permissions
FILE *checkFile(int sd, char* rootDir, char *fileName, char* request, int incBody)
{
    // Get the realpath to the file
    char fullPath[MAX_PATH_STR];
    strncpy(fullPath, rootDir, sizeof(fullPath));
    strncat(fullPath, fileName, MAX_PATH_STR - strlen(fileName)-1);

    char fullFilename[1024];
    char *res = realpath(fullPath, fullFilename);

    FILE *file;
    char path[MAX_PATH_STR] = "";

    // Check if the file is inside the root directory
    if (!strncmp(rootDir, fullFilename, strlen(rootDir)) == 0){
        // Outside of root dir, 403
        handleForbiddenRequest(sd, rootDir, request, incBody);
        closeConnection(sd);
    }

    //Check if we have read permission on file
    int rPermission = access(fullFilename, R_OK);

    //if we don't have permission return forbidden request
    if (rPermission != 0 && access(fullFilename, F_OK) == 0){
        handleForbiddenRequest(sd, rootDir, request, incBody);
        closeConnection(sd);
    }

    // Opens file and returns it or 404 if the file is not found
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
