#include "../include/Checkers.h"

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
            generateHeader(501, fileContent, sizeof(fileContent));
            send(sd, fileContent, strlen(fileContent), MSG_EOR);
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

    file = fopen(fileName, "r");
    if (rPermission != 0 && file){
        handleForbiddenRequest(sd);
        closeConnection(sd);
    }

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
