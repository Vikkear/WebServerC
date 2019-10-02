#include "../include/includer.h"
#include "../include/Handlers.h"
#include "../include/Headers.h"
#include "../include/Checkers.h"

#define DIE(str) \
    perror(str); \
    exit(-1);

char *rootDirLink = "../../www";
char rootDir[MAX_PATH_STR];


void printHelp();

int main(int argc, char *argv[])
{
    int portnumber = -1;
    struct sockaddr_in sin, pin;
    int sd, sd_current;
    int addrlen;
    realpath(rootDirLink, rootDir);

    // Command line options:
    // -h Print help text
    // "-p port" Listen to port number "port"

    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i], "-h") == 0) printHelp();
        if(strcmp(argv[i], "-p") == 0){
            if(i+1 < argc) portnumber = atoi(argv[i+1]);
        }
    }

    if (argc < 2 || portnumber == -1){
        printHelp();
    }

    //portnumber = atoi(argv[1]);

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        DIE("socket");
    }

    /* zero out the sockaddr_in struct */
    memset(&sin, 0, sizeof(sin));
    /* setup the struct to inform the operating system that we would like
    * to bind the socket the the given port number on any network
    * interface using IPv4 */
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(portnumber);

    if (bind(sd, (struct sockaddr *)&sin, sizeof(sin)) == -1)
    {
        DIE("bind");
    }

    /* start listening for connections arriving on the bound socket
        * here, we set the backlog of pending connections to 10*/
    if (listen(sd, 10) == -1)
    {
        DIE("listen");
    }

    addrlen = sizeof(pin);
    /* wait for incomming connections;
         * the address information of the communication partner is placed in
         * the provided sockaddr_in struct */
    int forkID = 0;
    while (forkID == 0)
    {
        if ((sd_current = accept(sd, (struct sockaddr *)&pin, (socklen_t *)&addrlen)) == -1)
        {
            DIE("accept");
        }
        printf("accepted connection\n");

        //TODO: MAKE THE BEST FORK/THROD
        forkID = fork();
    }

    if (forkID != 0)
    {
        handleRequest(sd_current, rootDir);
    }

    if (forkID == 0)
    {
        shutdown(sd, SHUT_RD);
    }

    shutdown(sd_current, SHUT_WR);
    close(sd_current);
    exit(0);
}

void printHelp(){
    printf("Usage: ./webserver [h|p]\n");
    printf("-h, Print this help menu\n");
    printf("-p <port>, Select which port to listen on\n");
    exit(0);
}
