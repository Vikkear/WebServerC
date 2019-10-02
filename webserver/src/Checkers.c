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

int checkUnsuppotedMethod(int sd,char* method){
    // Check if unsupported method
    for(int i = 0; i < 7; i++){
        if(strcmp(method, unSupported[i]) == 0){
            char fileContent[BUFSIZE] = "";
            char *path = "../../www/NotImplemented.html";
            generateHeader(501, path, fileContent, sizeof(fileContent));
            sendWithFile(sd, fileContent, path);
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
        handleForbiddenRequest(sd);
        closeConnection(sd);
    }

    int rPermission = access(fileName, R_OK);

    if (rPermission != 0 && access(fileName, F_OK) == 0){
        handleForbiddenRequest(sd);
        closeConnection(sd);
    }

    file = fopen(fileName, "r");
    if (file)
    {
        return file;
    }
    else
    {
        handleFileNotFound(sd);
        closeConnection(sd);
        return NULL;
    }
}
