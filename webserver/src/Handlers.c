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

    char requestBuf[BUFSIZE];
    strncpy(requestBuf, buf, sizeof(requestBuf));

    printf("%s\n", requestBuf);
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
            handleBadRequest(sd_current, rootDir, requestBuf, 1);
            closeConnection(sd_current);
        }
    }
    printf("Request counter: %d\n", requestCounter);

    if(requestCounter < 3) {
        handleBadRequest(sd_current, rootDir, requestBuf, 1);
        closeConnection(sd_current);
    }

    if(checkUnsuppotedMethod(sd_current, requests[0], rootDir, requestBuf) == 1) {
        closeConnection(sd_current);
    }

    if (strcmp(requests[0], "GET") == 0)
    {
        if (checkVersion(requests[2]) == 1)
        {
            printf("correct version\n");
            handleGET(sd_current, rootDir, requests[1], requestBuf);
        }
        else
        {
            // 400
            handleBadRequest(sd_current, rootDir, requestBuf, 1);
            printf("incorrect version\n");
        }
    }
    else if (strcmp(requests[0], "HEAD") == 0)
    {
        if (checkVersion(requests[2]) == 1)
        {
            printf("correct version\n");
            handleHEAD(sd_current, rootDir, requests[1], requestBuf);
        }
        else
        {
            // 400
            handleBadRequest(sd_current, rootDir, requestBuf, 0);
            printf("incorrect version\n");
        }
    }
    else
    {
        handleInternalServerError(sd_current, rootDir, requestBuf, 1);
    }
    return 0;
}

int handleGET(int sd, char *rootDir, char *path, char * request)
{
    int size = 0;
    char fileContent[BUFSIZE] = "";
    char fullpath[MAX_PATH_STR] = "";
    strncat(fullpath, rootDir, MAX_PATH_STR - strlen(fullpath) - 1);
    strncat(fullpath, path, MAX_PATH_STR - strlen(fullpath) - 1);
    generateHeader(200, fullpath, fileContent, sizeof(fileContent));
    size = sendWithFile(sd, fileContent, rootDir, path, request);
    logToFile(sd, request, 200, size);
    closeConnection(sd);
}

int handleHEAD(int sd, char* rootDir , char *path, char* request){
    // Check if file exist
    FILE *file = checkFile(sd, rootDir, path, request, 0);
    if (file)
    {

        char header[BUFSIZE] = "";
        char fullpath[MAX_PATH_STR] = "";
        strncat(fullpath, rootDir, MAX_PATH_STR - strlen(fullpath) - 1);
        strncat(fullpath, path, MAX_PATH_STR - strlen(fullpath) - 1);
        generateHeader(200, fullpath, header, sizeof(header));
        send(sd, header, strlen(header), MSG_EOR);
        logToFile(sd, request, 200, 0);
        closeConnection(sd);
    }
    else
    {
        handleFileNotFound(sd, rootDir, request, 0);
    }
}


int sendWithFile(int sd, char* fileContent, char* rootDir, char* path, char* request){
    unsigned int filesize = 0;
    //FILE* file = fopen(path,"r");
    char tmpSTR[BUFSIZE] = "";
    // Check if file exist
    FILE *file = checkFile(sd, rootDir , path, request, 1);
    if(file){
        while (fgets(tmpSTR, BUFSIZE, file) != NULL)
        {
            strncat(fileContent, tmpSTR, BUFSIZE - strlen(fileContent) - 1);
            filesize += strlen(tmpSTR);
        }
        send(sd, fileContent, strlen(fileContent), MSG_EOR);
        fclose(file);
    } else {
        handleFileNotFound(sd, rootDir, request, 1);
    }

    return filesize;
}

int handleBadRequest(int sd, char* rootDir, char* request, int incBody) {
    char path[MAX_PATH_STR] = "/BadRequest.html";
    int size = handleFaultyRequest(sd, rootDir, 400, path, request, incBody);
    logToFile(sd, request, 400, size);
    closeConnection(sd);
}

void handleForbiddenRequest(int sd, char* rootDir, char* request, int incBody){
    char path[MAX_PATH_STR] = "/Forbidden.html";
    int size = handleFaultyRequest(sd, rootDir, 403, path, request, incBody);
    logToFile(sd, request, 403, size);
    closeConnection(sd);
}

void handleFileNotFound(int sd, char* rootDir, char* request, int incBody){
    char path[MAX_PATH_STR] = "/NotFound.html";
    int size = handleFaultyRequest(sd, rootDir, 404, path, request, incBody);
    logToFile(sd, request, 404, size);
    closeConnection(sd);
}

void handleInternalServerError(int sd, char* rootDir, char* request, int incBody){
    char path[MAX_PATH_STR] = "/InternalServerError.html";
    int size = handleFaultyRequest(sd, rootDir, 500, path, request, incBody);
    logToFile(sd, request, 500, size);
    closeConnection(sd);
}

void handleNotImplemented(int sd, char* rootDir, char* request, int incBody){
    char path[MAX_PATH_STR] = "/NotImplemented.html";
    int size = handleFaultyRequest(sd, rootDir, 501, path, request, incBody);
    logToFile(sd, request, 501, size);
    closeConnection(sd);
}

int handleFaultyRequest(int sd, char* rootDir, int code, char* fileName, char* request, int incBody){
    char fileContent[BUFSIZE] = "";
    char path[MAX_PATH_STR] = "";
    int size = 0;
    strncat(path, rootDir, MAX_PATH_STR - strlen(path) - 1);
    strncat(path, fileName, MAX_PATH_STR - strlen(path) - 1);

    generateHeader(code, path, fileContent, sizeof(fileContent));
    if(incBody)
        size = sendWithFile(sd, fileContent, rootDir, fileName, request);
    else {
        send(sd, fileContent, strlen(fileContent), MSG_EOR);
        size = 0;
    }
    return size;
}

void logToFile(int sd, char* request, int code, int size){
    char logMessage[LOGSIZE] = "";
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    int res = getpeername(sd, (struct sockaddr *)&addr, &addr_size);
    char ip[20];
    strncpy(ip, inet_ntoa(addr.sin_addr), sizeof(ip));

    char dateString[MAX_PATH_STR] = "";
    time_t t = time(NULL);
    struct tm tm = *gmtime(&t);
    strftime(dateString, sizeof(dateString), "[%d/%b/%Y:%H:%M:%S %z]", &tm);

    char firstLineRequest[MAX_PATH_STR] = "";
    char* ptr = strchr(request, '\r');
    if(ptr == NULL) ptr = strchr(request, '\n');

    if(ptr != NULL){
        int newlinePos = (int)(ptr - request);
        strncpy(firstLineRequest, request, newlinePos);
    }
    else strncpy(firstLineRequest, request, sizeof(firstLineRequest));

    sprintf(logMessage, "%s - - %s \"%s\" %d %d", ip, dateString, firstLineRequest, code, size);

    if(useSyslog){
        syslog(LOG_INFO, logMessage);
    } else {
        if(logFilepointer){
            strncat(logMessage, "\n", LOGSIZE - strlen(logMessage) - 1);
            fputs(logMessage, logFilepointer);
            fflush(logFilepointer);
        }
        else {
            printf("Error: Could not open logfile!\n");
        }
    }
}
//TODO: ADD INTERNAL SERVER ERROR!!!!!!
