#include "../include/Handlers.h"
#include "../include/Checkers.h"

#define REQUEST_SIZE 64

int handleRequest(int sd_current, char* rootDir){
    /* receive at most sizeof(buf) many bytes and store them in the buffer */
    char buf[BUFSIZE] = "";
    if (recv(sd_current, buf, sizeof(buf), 0) == -1)
    {
        DIE("recv");
    }
    buf[BUFSIZE-1] = '\0';

    printf("%s\n", buf);
    char delim[] = " ";
    char *request = strtok(buf, delim);
    char *requests[REQUEST_SIZE];
    int requestCounter = 0;

    while (request != NULL && requestCounter < REQUEST_SIZE)
    {
        requests[requestCounter++] = request;
        request = strtok(NULL, delim);
    }

    for (int i = 0; i < requestCounter; i++)
    {
        printf("%s\n", requests[i]);
        if(strlen(requests[i]) >= MAX_PATH_STR){
            handleBadRequest(sd_current, rootDir);
            closeConnection(sd_current);
        }
    }
    printf("Request counter: %d\n", requestCounter);

    if(requestCounter < 3) {
        handleBadRequest(sd_current, rootDir);
        closeConnection(sd_current);
    }

    if(checkUnsuppotedMethod(sd_current, requests[0], rootDir) == 1) {
        closeConnection(sd_current);
    }

    if (strcmp(requests[0], "GET") == 0)
    {
        if (checkVersion(requests[2]) == 1)
        {
            printf("correct version\n");
            handleGET(sd_current, rootDir, requests[1]);
        }
        else
        {
            // 400
            handleBadRequest(sd_current, rootDir);
            printf("incorrect version\n");
        }
    }
    else if (strcmp(requests[0], "HEAD") == 0)
    {
        if (checkVersion(requests[2]) == 1)
        {
            printf("correct version\n");
            handleHEAD(sd_current, rootDir , requests[1]);
        }
        else
        {
            // 400
            handleBadRequest(sd_current, rootDir);
            printf("incorrect version\n");
        }
    }
    else
    {
        handleBadRequest(sd_current, rootDir);
    }
    return 0;
}

int handleGET(int sd, char *rootDir, char *path)
{
    char fileContent[BUFSIZE] = "";
    strncat(rootDir, path);
    generateHeader(200, rootDir, fileContent, sizeof(fileContent));
    sendWithFile(sd, fileContent, rootDir);

}

int handleHEAD(int sd, char* rootDir , char *path){
    // Check if file exist
    char fullPath[MAX_PATH_STR];
    strcpy(fullPath, rootDir);
    strcat(fullPath, path);

    char buf[1024];
    char *res = realpath(fullPath, buf);
    FILE *file = checkFile(sd, rootDir, buf);
    if (file)
    {
        char header[BUFSIZE] = "";
        generateHeader(200, buf, header, sizeof(header));
        send(sd, header, strlen(header), MSG_EOR);
    }
    else
    {
        handleFileNotFound(sd, rootDir);
    }
}


void sendWithFile(int sd, char* fileContent, char* fullPath){

    //FILE* file = fopen(path,"r");
    //char tmpSTR[BUFSIZE] = "";
    // Check if file exist

    char buf[1024];
    char *res = realpath(fullPath, buf);
    FILE *file = checkFile(sd, rootDir , buf);
    if(file){
        while (fgets(tmpSTR, BUFSIZE, file) != NULL)
        {
            strncat(fileContent, tmpSTR, BUFSIZE - strlen(fileContent) - 1);
        }
        send(sd, fileContent, strlen(fileContent), MSG_EOR);
        fclose(file);
    } else {
        handleFileNotFound(sd, rootDir);
    }
}

int handleBadRequest(int sd, char* rootDir) {
    handleFaultyRequest(sd, rootDir, 400, "/BadRequest.html");
}

void handleForbiddenRequest(int sd, char* rootDir){
    handleFaultyRequest(sd, rootDir, 403, "/Forbidden.html");
}

void handleFileNotFound(int sd, char* rootDir){
    handleFaultyRequest(sd, rootDir, 404, "/NotFound.html");
}

void handleNotImplemented(int sd, char* rootDir){
    handleFaultyRequest(sd, rootDir, 501, "/NotImplemented.html");
}

void handleInternalServerError(int sd, char* rootDir){
    handleFaultyRequest(sd, rootDir, 500, "/InternalServerError.html");
}

void handleFaultyRequest(int sd, char* rootDir, int code, char* fileName){
    char fileContent[BUFSIZE] = "";
    char path[MAX_PATH_STR] = "";
    strncat(path, rootDir, BUFSIZE - strlen(path) - 1);
    strncat(path, fileName, BUFSIZE - strlen(path) - 1);

    generateHeader(501, path, fileContent, sizeof(fileContent));
    sendWithFile(sd, fileContent, path);
}
//TODO: ADD INTERNAL SERVER ERROR!!!!!!
