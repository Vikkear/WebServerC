#include "../include/Handlers.h"
#include "../include/Checkers.h"

#include <syslog.h>

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
    char fullpath[MAX_PATH_STR] = "";
    strncat(fullpath, rootDir, MAX_PATH_STR - strlen(fullpath) - 1);
    strncat(fullpath, path, MAX_PATH_STR - strlen(fullpath) - 1);
    generateHeader(200, fullpath, fileContent, sizeof(fileContent));
    sendWithFile(sd, fileContent, rootDir, path);

    char logMessage[LOGSIZE] = "";
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    int res = getpeername(sd, (struct sockaddr *)&addr, &addr_size);
    char ip[20];
    strcpy(ip, inet_ntoa(addr.sin_addr));
    printf("%s\n", ip);

    char dateString[MAX_PATH_STR] = "";
    time_t t = time(NULL);
    struct tm tm = *gmtime(&t);
    strftime(dateString, sizeof(dateString), "[%a, %d %b %Y %H:%M:%S %Z]\r\n", &tm);

    char requestString[MAX_PATH_STR] = "";

    sprintf(logMessage, "%s - - %s %s %d %d\n", ip, dateString, requestString, 200, 10);
    if(useSyslog){
        syslog(LOG_INFO, logMessage);
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
        handleFileNotFound(sd, rootDir);
    }
}


void sendWithFile(int sd, char* fileContent, char* rootDir, char* path){

    //FILE* file = fopen(path,"r");
    char tmpSTR[BUFSIZE] = "";
    // Check if file exist

    char fullPath[MAX_PATH_STR];
    strcpy(fullPath, rootDir);
    strcat(fullPath, path);

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
    strncat(path, rootDir, MAX_PATH_STR - strlen(path) - 1);
    strncat(path, fileName, MAX_PATH_STR - strlen(path) - 1);

    generateHeader(code, path, fileContent, sizeof(fileContent));
    sendWithFile(sd, fileContent, rootDir, fileName);
}
//TODO: ADD INTERNAL SERVER ERROR!!!!!!
