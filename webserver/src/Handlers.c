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
            handleBadRequest(sd_current, rootDir, requestBuf);
            closeConnection(sd_current);
        }
    }
    printf("Request counter: %d\n", requestCounter);

    if(requestCounter < 3) {
        handleBadRequest(sd_current, rootDir, requestBuf);
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
            handleBadRequest(sd_current, rootDir, requestBuf);
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
            handleBadRequest(sd_current, rootDir, requestBuf);
            printf("incorrect version\n");
        }
    }
    else
    {
        handleBadRequest(sd_current, rootDir, requestBuf);
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
}

int handleHEAD(int sd, char* rootDir , char *path, char* request){
    // Check if file exist
    FILE *file = checkFile(sd, rootDir, path, request);
    if (file)
    {

        char header[BUFSIZE] = "";
        char fullpath[MAX_PATH_STR] = "";
        strncat(fullpath, rootDir, MAX_PATH_STR - strlen(fullpath) - 1);
        strncat(fullpath, path, MAX_PATH_STR - strlen(fullpath) - 1);
        generateHeader(200, fullpath, header, sizeof(header));
        send(sd, header, strlen(header), MSG_EOR);
        logToFile(sd, request, 200, 0);
    }
    else
    {
        handleFileNotFound(sd, rootDir, request);
    }
}


int sendWithFile(int sd, char* fileContent, char* rootDir, char* path, char* request){
    unsigned int filesize = 0;
    //FILE* file = fopen(path,"r");
    char tmpSTR[BUFSIZE] = "";
    // Check if file exist
    FILE *file = checkFile(sd, rootDir , path, request);
    if(file){
        while (fgets(tmpSTR, BUFSIZE, file) != NULL)
        {
            strncat(fileContent, tmpSTR, BUFSIZE - strlen(fileContent) - 1);
            filesize += strlen(tmpSTR);
        }
        send(sd, fileContent, strlen(fileContent), MSG_EOR);
        fclose(file);
    } else {
        handleFileNotFound(sd, rootDir, request);
    }

    return filesize;
}

int handleBadRequest(int sd, char* rootDir, char* request) {
    char path[MAX_PATH_STR] = "/BadRequest.html";
    int size = handleFaultyRequest(sd, rootDir, 400, path, request);
    logToFile(sd, request, 400, size);
}

void handleForbiddenRequest(int sd, char* rootDir, char* request){
    char path[MAX_PATH_STR] = "/Forbidden.html";
    int size = handleFaultyRequest(sd, rootDir, 403, path, request);
    logToFile(sd, request, 403, size);
}

void handleFileNotFound(int sd, char* rootDir, char* request){
    char path[MAX_PATH_STR] = "/NotFound.html";
    int size = handleFaultyRequest(sd, rootDir, 404, path, request);
    logToFile(sd, request, 404, size);
}

void handleInternalServerError(int sd, char* rootDir, char* request){
    char path[MAX_PATH_STR] = "/InternalServerError.html";
    int size = handleFaultyRequest(sd, rootDir, 500, path, request);
    logToFile(sd, request, 500, size);
}

void handleNotImplemented(int sd, char* rootDir, char* request){
    char path[MAX_PATH_STR] = "/NotImplemented.html";
    int size = handleFaultyRequest(sd, rootDir, 501, path, request);
    logToFile(sd, request, 501, size);
}

int handleFaultyRequest(int sd, char* rootDir, int code, char* fileName, char* request){
    char fileContent[BUFSIZE] = "";
    char path[MAX_PATH_STR] = "";
    int size = 0;
    strncat(path, rootDir, MAX_PATH_STR - strlen(path) - 1);
    strncat(path, fileName, MAX_PATH_STR - strlen(path) - 1);

    generateHeader(code, path, fileContent, sizeof(fileContent));
    size = sendWithFile(sd, fileContent, rootDir, fileName, request);
    return size;
}

void logToFile(int sd, char* request, int code, int size){
    char logMessage[LOGSIZE] = "";
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    int res = getpeername(sd, (struct sockaddr *)&addr, &addr_size);
    char ip[20];
    strcpy(ip, inet_ntoa(addr.sin_addr));

    char dateString[MAX_PATH_STR] = "";
    time_t t = time(NULL);
    struct tm tm = *gmtime(&t);
    strftime(dateString, sizeof(dateString), "[%a, %d %b %Y %H:%M:%S %Z]", &tm);

    char firstLineRequest[MAX_PATH_STR] = "";
    char* ptr = strchr(request, '\n');
    if(ptr != NULL){
        int newlinePos = (int)(ptr - request);
        strncpy(firstLineRequest, request, newlinePos);
    }
    else strncpy(firstLineRequest, request, sizeof(firstLineRequest));

    sprintf(logMessage, "%s - - %s \"%s\" %d %d", ip, dateString, firstLineRequest, code, size);

    if(useSyslog){
        syslog(LOG_INFO, logMessage);
    } else {
        FILE* file = fopen(logfile, "a+");
        if(file){
            strncat(logMessage, "\n", LOGSIZE - strlen(logMessage) - 1);
            fputs(logMessage, file);
            fclose(file);
        }
        else {
            printf("Error: Could not open logfile!\n");
        }
    }
}
//TODO: ADD INTERNAL SERVER ERROR!!!!!!
