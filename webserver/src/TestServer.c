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

#define DIE(str) perror(str);exit(-1);
#define BUFSIZE 512
#define MAX_PATH_STR 80

char * rootDir = "../../www";

int checkVersion(char* version);
int handleGET(int sd, char* path);
FILE* checkFile(char* fileName);
int validInputStr(char * input);

int main(int argc, char* argv[])
{
    int portnumber;
	struct sockaddr_in sin, pin;
	int sd, sd_current;
	int addrlen;
	char buf[BUFSIZE] = "";

	if(argc != 2) {
		fprintf(stderr, "Usage: %s <port>\n", argv[0]);
		exit(-1);
	}

	portnumber = atoi(argv[1]);

    if((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
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

    if(bind(sd, (struct sockaddr*) &sin, sizeof(sin)) == -1) {
        DIE("bind");
    }

        /* start listening for connections arriving on the bound socket
         * here, we set the backlog of pending connections to 10*/
    if(listen(sd, 10) == -1) {
        DIE("listen");
    }

    addrlen = sizeof(pin);
        /* wait for incomming connections;
         * the address information of the communication partner is placed in
         * the provided sockaddr_in struct */
    int forkID = 0;
    while(forkID == 0){
    	if ((sd_current = accept(sd, (struct sockaddr*) &pin, (socklen_t*) &addrlen)) == -1) {
    		DIE("accept");
    	}
        printf("accepted connection\n");

        //TODO: MAKE THE BEST FORK/THROD
        forkID = fork();

    }

    if (forkID != 0) {
        /* receive at most sizeof(buf) many bytes and store them in the buffer */

        if (recv(sd_current, buf, sizeof(buf), 0) == -1) {
            DIE("recv");
        }
        printf("%s\n", buf);
        char delim[] = " ";
        char* request = strtok(buf, delim);
        char* requests[3];
        int requestCounter = 0;
        while (request != NULL && requestCounter < 3) {
            requests[requestCounter++] = request;
            request = strtok(NULL, delim);
        }
        for (int i = 0; i < requestCounter; i++){
            printf("%s\n", requests[i]);
        }

        if (strcmp(requests[0], "GET") == 0){
            if(checkVersion(requests[2]) == 1){
                printf("correct version\n");
                handleGET(sd_current, requests[1]);
            } else {
                // 400
                printf("incorrect version\n");
            }
        }
        else if(strcmp(requests[0], "HEAD") == 0){
            if(checkVersion(requests[2]) == 1){
                printf("correct version\n");
            } else {
                // 400
                printf("incorrect version\n");
            }
        }
        else{
            // 501
            printf("501\n");
        }

    }

    if (forkID == 0){
        shutdown(sd, SHUT_RD);
    }
    shutdown(sd_current, SHUT_RD);
    exit(0);
}

int handleGET(int sd ,char* path){
    printf("%s\n", path);
    validInputStr(path);
    // Check if file exists
    FILE* file = checkFile(path);
    if(file){
        // 200 File found
        char tmpSTR[BUFSIZE] = "";
        char fileContent[BUFSIZE] = "HTTP/1.0 200 ok\n";
        while (fgets(tmpSTR, BUFSIZE, file) != NULL){
            strcat(fileContent, tmpSTR);
        }
        send(sd, fileContent, BUFSIZE, NULL);
        //sendfile(sd, fileno(file), NULL, BUFSIZE);
    } else {
        // 404 File not found

    }
}

int checkVersion(char* version){
    //NOTE: If client doesn't add newline version can get fucked
    if (strstr(version, "HTTP/1.0") != NULL){
        return 1;
    }
    return 0;
}

FILE* checkFile(char* fileName){
    FILE * file;
    char path[MAX_PATH_STR] = "";
    strncpy(path , rootDir, MAX_PATH_STR);
    strcat(path, fileName);
    file = fopen(path, "r");
    if(file){
        return file;
    } else {
        return NULL;
    }
}

int validInputStr(char * input){

}
