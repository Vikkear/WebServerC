#include "../include/Handlers.h"
#include "../include/Checkers.h"

#include <syslog.h>

#define REQUEST_SIZE 64

// Called from main function every time someone makes a connection
int handleRequest(int sd_current, char* rootDir){
    /* receive at most sizeof(buf) many bytes and store them in the buffer */
    char buf[BUFSIZE] = "";
    if (recv(sd_current, buf, sizeof(buf), 0) == -1)
    {
        DIE("recv");
    }
    //Set thelast character to null terminator incase the user didn't submit it
    buf[BUFSIZE-1] = '\0';

    char requestBuf[BUFSIZE];
    strncpy(requestBuf, buf, sizeof(requestBuf));

    //Split the string with the delimiter "space" to create arguments
    char delim[] = " ";
    char *request = strtok(buf, delim);
    char *requests[REQUEST_SIZE];
    int requestCounter = 0;

    while (request != NULL && requestCounter < REQUEST_SIZE)
    {
        requests[requestCounter++] = request;
        request = strtok(NULL, delim);
    }

    // Loops through the request arguments and checks if the length is not too big.
    // If it is, then it is a bad request and will be closed.
    for (int i = 0; i < requestCounter; i++)
    {
        if(strlen(requests[i]) >= MAX_PATH_STR){
            handleBadRequest(sd_current, rootDir, requestBuf, 1);
            closeConnection(sd_current);
        }
    }

    //If we have less than three arguments we will get bad request
    if(requestCounter < 3) {
        handleBadRequest(sd_current, rootDir, requestBuf, 1);
        closeConnection(sd_current);
    }

    // Check if the user sent in a request that is unsupported
    if(checkUnsuppotedMethod(sd_current, requests[0], rootDir, requestBuf) == 1) {
        closeConnection(sd_current);
    }

    //Check if the first argument is GET and call handleGET
    if (strcmp(requests[0], "GET") == 0)
    {
        //Check if we received correct http version
        if (checkVersion(requests[2]) == 1)
        {
            handleGET(sd_current, rootDir, requests[1], requestBuf);
        }
        else
        {
            // 400
            handleBadRequest(sd_current, rootDir, requestBuf, 1);
        }
    }
    //Check if the first argument is GET and call handleHEAD
    else if (strcmp(requests[0], "HEAD") == 0)
    {
        //Check if we received correct http version
        if (checkVersion(requests[2]) == 1)
        {
            handleHEAD(sd_current, rootDir, requests[1], requestBuf);
        }
        else
        {
            // 400
            handleBadRequest(sd_current, rootDir, requestBuf, 0);
        }
    }
    //If no condition is met return InternalServerError
    else
    {
        handleInternalServerError(sd_current, rootDir, requestBuf, 1);
    }
    return 0;
}

// Handles the request if it is a GET
int handleGET(int sd, char *rootDir, char *path, char * request)
{
    // get the full file path
    int size = 0;
    char fileContent[BUFSIZE] = "";
    char fullpath[MAX_PATH_STR] = "";
    strncat(fullpath, rootDir, MAX_PATH_STR - strlen(fullpath) - 1);
    strncat(fullpath, path, MAX_PATH_STR - strlen(fullpath) - 1);

    // Get the header and then send the header together with the file.
    generateHeader(200, fullpath, fileContent, sizeof(fileContent));
    size = sendWithFile(sd, fileContent, rootDir, path, request);

    // Log to file and close the connection
    logToFile(sd, request, 200, size);
    closeConnection(sd);
}

// Handles the request if it is a HEAD
int handleHEAD(int sd, char* rootDir , char *path, char* request){
    // Check if file exist
    FILE *file = checkFile(sd, rootDir, path, request, 0);
    if (file)
    {
        // Get the full file path
        char header[BUFSIZE] = "";
        char fullpath[MAX_PATH_STR] = "";
        strncat(fullpath, rootDir, MAX_PATH_STR - strlen(fullpath) - 1);
        strncat(fullpath, path, MAX_PATH_STR - strlen(fullpath) - 1);

        // Get the header and then send the header
        generateHeader(200, fullpath, header, sizeof(header));
        send(sd, header, strlen(header), MSG_EOR);

        // Log to file and close the connection
        logToFile(sd, request, 200, 0);
        closeConnection(sd);
    }
    else
    {
        handleFileNotFound(sd, rootDir, request, 0);
    }
}

//Concatinate header and file and then send it back to the client
int sendWithFile(int sd, char* fileContent, char* rootDir, char* path, char* request){
    unsigned int filesize = 0;
    //FILE* file = fopen(path,"r");
    char tmpSTR[BUFSIZE] = "";
    // Check if file exist
    FILE *file = checkFile(sd, rootDir , path, request, 1);
    if(file){
        //Loop through file and append it to fileContent
        while (fgets(tmpSTR, BUFSIZE, file) != NULL)
        {
            strncat(fileContent, tmpSTR, BUFSIZE - strlen(fileContent) - 1);
            filesize += strlen(tmpSTR);
        }
        //Send the buffer
        send(sd, fileContent, strlen(fileContent), MSG_EOR);
        fclose(file);
    } else {
        //If file is not found return FileNotFound
        handleFileNotFound(sd, rootDir, request, 1);
    }

    return filesize;
}

// Handles the request if it is a bad request (400)
int handleBadRequest(int sd, char* rootDir, char* request, int incBody) {
    char path[MAX_PATH_STR] = "/BadRequest.html";
    int size = handleFaultyRequest(sd, rootDir, 400, path, request, incBody);
    logToFile(sd, request, 400, size);
    closeConnection(sd);
}

// Handles the request if it is a forbidden request (403)
void handleForbiddenRequest(int sd, char* rootDir, char* request, int incBody){
    char path[MAX_PATH_STR] = "/Forbidden.html";
    int size = handleFaultyRequest(sd, rootDir, 403, path, request, incBody);
    logToFile(sd, request, 403, size);
    closeConnection(sd);
}

// Handles the request if the file was not found (404)
void handleFileNotFound(int sd, char* rootDir, char* request, int incBody){
    char path[MAX_PATH_STR] = "/NotFound.html";
    int size = handleFaultyRequest(sd, rootDir, 404, path, request, incBody);
    logToFile(sd, request, 404, size);
    closeConnection(sd);
}

// Handles the request if it is a internal server error (500)
void handleInternalServerError(int sd, char* rootDir, char* request, int incBody){
    char path[MAX_PATH_STR] = "/InternalServerError.html";
    int size = handleFaultyRequest(sd, rootDir, 500, path, request, incBody);
    logToFile(sd, request, 500, size);
    closeConnection(sd);
}

// Handles the request if the method sent is not implemented (501)
void handleNotImplemented(int sd, char* rootDir, char* request, int incBody){
    char path[MAX_PATH_STR] = "/NotImplemented.html";
    int size = handleFaultyRequest(sd, rootDir, 501, path, request, incBody);
    logToFile(sd, request, 501, size);
    closeConnection(sd);
}

//Help function for error responses to get generate header and send file
int handleFaultyRequest(int sd, char* rootDir, int code, char* fileName, char* request, int incBody){
    char fileContent[BUFSIZE] = "";
    char path[MAX_PATH_STR] = "";
    int size = 0;

    //Get full file path
    strncat(path, rootDir, MAX_PATH_STR - strlen(path) - 1);
    strncat(path, fileName, MAX_PATH_STR - strlen(path) - 1);

    //Generate header
    generateHeader(code, path, fileContent, sizeof(fileContent));

    //Check if response should include a body
    if(incBody)
        size = sendWithFile(sd, fileContent, rootDir, fileName, request);
    else {
        send(sd, fileContent, strlen(fileContent), MSG_EOR);
        size = 0;
    }
    return size;
}

//Log request content to file in CLF format
void logToFile(int sd, char* request, int code, int size){
    char logMessage[LOGSIZE] = "";
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    int res = getpeername(sd, (struct sockaddr *)&addr, &addr_size);
    char ip[20];
    strncpy(ip, inet_ntoa(addr.sin_addr), sizeof(ip));

    //Generate a date string
    char dateString[MAX_PATH_STR] = "";
    time_t t = time(NULL);
    struct tm tm = *gmtime(&t);
    strftime(dateString, sizeof(dateString), "[%d/%b/%Y:%H:%M:%S %z]", &tm);

    //Cut the out the first line of request
    char firstLineRequest[MAX_PATH_STR] = "";
    char* ptr = strchr(request, '\r');
    if(ptr == NULL) ptr = strchr(request, '\n');

    if(ptr != NULL){
        int newlinePos = (int)(ptr - request);
        strncpy(firstLineRequest, request, newlinePos);
    }
    else strncpy(firstLineRequest, request, sizeof(firstLineRequest));

    //Format the string
    snprintf(logMessage, "%s - - %s \"%s\" %d %d", ip, dateString, firstLineRequest, code, size, sizeof(logMessage));

    // Use the syslog if no filename is given
    if(useSyslog){
        syslog(LOG_INFO, logMessage);
    } else {
        if(logFilepointer){
            strncat(logMessage, "\n", LOGSIZE - strlen(logMessage) - 1);
            fputs(logMessage, logFilepointer);
            fflush(logFilepointer);
        }
    }
}
