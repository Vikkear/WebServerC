#include "../include/includer.h"
#include "../include/Handlers.h"
#include "../include/Headers.h"
#include "../include/Checkers.h"

#include <signal.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <unistd.h>
#include <syslog.h>
#include <pthread.h>


#define DIE(str) \
    perror(str); \
    exit(-1);

char *rootDirLink = "../../www";
char rootDir[MAX_PATH_STR];

int amountOfArguments = 5;
char *commandList[] = {"-p", "-h", "-d", "-l", "-s"};
char requestMethod[MAX_PATH_STR] = "";

int portnumber = -1;

void *handleRequestThread(void *vargp);
void daemonize();
void loadConfig();
void printHelp();

int main(int argc, char *argv[])
{
    struct sockaddr_in sin, pin;
    int sd, sd_current;
    int addrlen;
    realpath(rootDirLink, rootDir);

    loadConfig();
    // Command line options:
    // -h Print help text
    // "-p port" Listen to port number "port"
    // "-d" 

    memset(logfile, 0, MAX_PATH_STR); // <--- TODO: move somewhere nice
    for(int i = 1; i < argc; i++){
        int argcheck = 0;
        for (int j = 0; j < amountOfArguments; j++){
            if(strncmp(argv[i], commandList[j], 2) == 0){
                argcheck = 1;
            }
        }
        if(argcheck == 0) printHelp();

        if(strcmp(argv[i], "-h") == 0) printHelp();
        if(strcmp(argv[i], "-d") == 0) daemonize();
        if(strcmp(argv[i], "-p") == 0){
            if(i+1 < argc) {
                i++;
                portnumber = atoi(argv[i]);
            }
        }
        if(strcmp(argv[i], "-l") == 0){
            if(i+1 < argc) {
                i++;
                strncpy(logfile, argv[i], MAX_PATH_STR);
            }
        }
        if(strcmp(argv[i], "-s") == 0){
            if(i+1 < argc) {
                i++;
                strncpy(requestMethod, argv[i], MAX_PATH_STR);
            }
        }
    }

    char *requestMethodList[] = {"fork", "thread"};
    int validRequestMethod = 0;
    for(int i = 0; i < 2; i++){
        if(strncmp(requestMethod, requestMethodList[i], sizeof(requestMethod)) == 0){
            validRequestMethod = 1;
        }
    }

    if(validRequestMethod == 0){
        printf("Invalid request method! \n");
        printHelp();
    }

    if(strcmp(logfile, "") == 0){
        openlog("Webserver", LOG_NDELAY, LOG_USER);
        useSyslog = 1;
    }
    else {
        logFilepointer = fopen(logfile, "a+");
        useSyslog = 0;
    }

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

     //jail here
     chdir(rootDir);
     int rooted = chroot(rootDir);
     if(rooted == 0) {
         memset(rootDir, 0, sizeof(rootDir));
         printf("Jail succeded!\n");
     }
     else {
         printf("The process is not privileged (run as sudo for jail to function properly)\n");
     }

    if(strncmp(requestMethod, "fork", sizeof(requestMethod)) == 0){
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
    }
    else if(strncmp(requestMethod, "thread", sizeof(requestMethod)) == 0){
        while(1){
            if ((sd_current = accept(sd, (struct sockaddr *)&pin, (socklen_t *)&addrlen)) == -1)
            {
                DIE("accept");
            }
            printf("accepted connection\n");

            pthread_t thread_id;
            pthread_create(&thread_id, NULL, handleRequestThread, sd_current);
        }
    }
    //pthread_exit(NULL);
    shutdown(sd_current, SHUT_WR);
    close(sd_current);
    if(useSyslog == 1){
        closelog();
    }
    exit(0);
}

void *handleRequestThread(void *arg) {
    int *sd = (int*)arg;
    printf("sd: %d\n", sd);
    handleRequest(sd, rootDir);
}

void loadConfig(){
    char tmpSTR[MAX_PATH_STR] = "";
    FILE* file = fopen("../src/.lab3-config", "r");

    while (fgets(tmpSTR, MAX_PATH_STR, file) != NULL)
    {
        if(strncmp("rootdir", tmpSTR, 6) == 0){
            char* test = strchr(tmpSTR, '\"');
            char linkDir[MAX_PATH_STR] = "";
            memset(rootDir,0,strlen(rootDir));
            strncpy(linkDir, &test[1], (strlen(test) < MAX_PATH_STR) ? strlen(test)-3 : MAX_PATH_STR);
            realpath(linkDir, rootDir);
        }
        if(strncmp("port", tmpSTR, 4) == 0){

            char* test = strchr(tmpSTR, '\"');
            char intChar[MAX_PATH_STR] = "";
            strncpy(intChar, &test[1], (strlen(test) < MAX_PATH_STR) ? strlen(test)-3 : MAX_PATH_STR);
            portnumber = atoi(intChar);
        }
        if(strncmp("requestHandling", tmpSTR, 15) == 0){
            char* test = strchr(tmpSTR, '\"');
            char method[MAX_PATH_STR] = "";
            strncpy(method, &test[1], (strlen(test) < MAX_PATH_STR) ? strlen(test)-3 : MAX_PATH_STR);
            strncpy(requestMethod, method, sizeof(requestMethod));
        }
    }
    fclose(file);
}

void daemonize(){

    struct sigaction sa;
    struct rlimit r1;
    int fd0, fd1, fd2;

    umask(0);
    pid_t pid = fork();

    if(pid != 0){
        exit(0);
    }
    setsid();

    printf("PID: %d\n", getpid());

    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGHUP, &sa, NULL);

    chdir("/");

    if(getrlimit(RLIMIT_NOFILE, &r1)){
        perror(NULL);
    }


    if(r1.rlim_max == RLIM_INFINITY){
        r1.rlim_max = 1024;
    }

    for(int i = 0; i < r1.rlim_max; i++)
        close(i);

    fd0 = open("/dev/null", O_RDWR);
    fd1 = dup(0);
    fd2 = dup(0);
}

void printHelp(){
    printf("Usage: ./webserver [h|p|d|s]\n");
    printf("-h, Print this help menu\n");
    printf("-p <port>, Select which port to listen on\n");
    printf("-d, Run the server as a daemon\n");
    printf("-s [fork|thread], Select request handling method\n");
    exit(0);
}
