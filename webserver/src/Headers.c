#include "../include/Headers.h"

void generateHeader(int code, char* header, int size){
    char responseCode[MAX_PATH_STR] = "";
    switch (code) {
        case 200:
            strncpy(responseCode, "HTTP/1.0 200 OK\r\n", sizeof(responseCode));
            break;
        case 400:
            strncpy(responseCode, "HTTP/1.0 400 Bad Request\r\n", sizeof(responseCode));
            break;
        case 403:
            strncpy(responseCode, "HTTP/1.0 403 Forbidden\r\n", sizeof(responseCode));
            break;
        case 404:
            strncpy(responseCode, "HTTP/1.0 404 Not Found\r\n", sizeof(responseCode));
            break;
        case 500:
            strncpy(responseCode, "HTTP/1.0 500 Internal Server Error\r\n", sizeof(responseCode));
            break;
        case 501:
            strncpy(responseCode, "HTTP/1.0 501 Not Implemented\r\n", sizeof(responseCode));
            break;
    }
    char date[MAX_PATH_STR] = "";
    time_t t = time(NULL);
    struct tm tm = *gmtime(&t);
    strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S %Z\r\n", &tm);
    char* serverName = "Server: Lupuche/1338 (UNOX) (Green-Hat/LUNOX)\r\n";
    char* connectionType = "Connection: close\r\n\r\n";

    strncat(header, responseCode, size);
    strncat(header, date, size);
    strncat(header, serverName, size);
    strncat(header, connectionType, size);
}

void fgenerateHeader(FILE* file, int code, char* path, char* header, int size) {
    char responseCode[MAX_PATH_STR] = "";
    switch (code) {
        case 200:
            strncpy(responseCode, "HTTP/1.0 200 OK\r\n", sizeof(responseCode));
            break;
        case 400:
            strncpy(responseCode, "HTTP/1.0 400 Bad Request\r\n", sizeof(responseCode));
            break;
        case 403:
            strncpy(responseCode, "HTTP/1.0 403 Forbidden\r\n", sizeof(responseCode));
            break;
        case 404:
            strncpy(responseCode, "HTTP/1.0 404 Not Found\r\n", sizeof(responseCode));
            break;
        case 500:
            strncpy(responseCode, "HTTP/1.0 500 Internal Server Error\r\n", sizeof(responseCode));
            break;
        case 501:
            strncpy(responseCode, "HTTP/1.0 501 Not Implemented\r\n", sizeof(responseCode));
            break;
    }
    char date[MAX_PATH_STR] = "";
    time_t t = time(NULL);
    struct tm tm = *gmtime(&t);
    strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S %Z\r\n", &tm);

    char * contentType = "Content-Type: text/html; charset=UTF-8\r\n";

    fseek(file, 0L, SEEK_END);
    long fileLength = ftell(file);
    rewind(file);

    char lastMod[MAX_PATH_STR] = "";
    struct stat attrib;
    stat(path, &attrib);
    strftime(lastMod, sizeof(lastMod), "%a, %d %b %Y %H:%M:%S %Z\r\n", gmtime(&(attrib.st_ctime)));

    char* serverName = "Server: Lupuche/1338 (UNOX) (Green-Hat/LUNOX)\r\n";
    char* connectionType = "Connection: close\r\n\r\n";

    strncat(header, responseCode, size);
    strncat(header, date, size);
    strncat(header, contentType, size);
    strncat(header, lastMod, size);
    strncat(header, serverName, size);
    strncat(header, connectionType, size);

}
