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
            handleBadRequest(sd_current);
            closeConnection(sd_current);
        }
    }
    printf("Request counter: %d\n", requestCounter);

    if(requestCounter < 3) {
        handleBadRequest(sd_current);
        closeConnection(sd_current);
    }

    if(checkUnsuppotedMethod(sd_current, requests[0]) == 1) {
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
            handleBadRequest(sd_current);
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
            handleBadRequest(sd_current);
            printf("incorrect version\n");
        }
    }
    else
    {
        handleBadRequest(sd_current);
    }
    return 0;
}

int handleGET(int sd, char *rootDir, char *path)
{
    // Check if file exist
    char fullPath[MAX_PATH_STR];
    strcpy(fullPath, rootDir);
    strcat(fullPath, path);

    char buf[1024];
    char *res = realpath(fullPath, buf);
    FILE *file = checkFile(sd, rootDir , buf);
    if (file)
    {
        char fileContent[BUFSIZE] = "";
        char tmpSTR[BUFSIZE] = "";
        generateHeader(200, buf, fileContent, sizeof(fileContent));

        while (fgets(tmpSTR, BUFSIZE, file) != NULL)
        {
            strncat(fileContent, tmpSTR, sizeof(fileContent));
        }
        send(sd, fileContent, strlen(fileContent), MSG_EOR);

    }
    else
    {
        handleFileNotFound(sd);
    }
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
        handleFileNotFound(sd);
    }
}

int handleBadRequest(int sd) {
    char fileContent[BUFSIZE] = "";
    char* path = "../../www/BadRequest.html";
    generateHeader(400, path, fileContent, sizeof(fileContent));
    sendWithFile(sd, fileContent, path);

}

void sendWithFile(int sd, char* fileContent, char* path){

    FILE* file = fopen(path,"r");
    char tmpSTR[BUFSIZE] = "";
    if(file){
        while (fgets(tmpSTR, BUFSIZE, file) != NULL)
        {
            strncat(fileContent, tmpSTR, BUFSIZE - strlen(fileContent) - 1);
        }
        send(sd, fileContent, strlen(fileContent), MSG_EOR);
        fclose(file);
    }
}

void handleForbiddenRequest(int sd){
    char fileContent[BUFSIZE] = "";
    char* path = "../../www/Forbidden.html";
    generateHeader(403, path, fileContent, sizeof(fileContent));
    sendWithFile(sd, fileContent, path);
}

void handleFileNotFound(int sd){
    char fileContent[BUFSIZE] = "";
    char* path = "../../www/NotFound.html";
    generateHeader(404, path, fileContent, sizeof(fileContent));
    sendWithFile(sd, fileContent, path);
}
