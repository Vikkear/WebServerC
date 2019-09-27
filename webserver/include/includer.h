#ifndef INCLUDER
#define INCLUDER

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/errno.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/stat.h>

#define DIE(str) \
    perror(str); \
    exit(-1);

void closeConnection(int sd);

#endif
