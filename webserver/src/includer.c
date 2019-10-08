#include "../include/includer.h"
useSyslog = 0;

void closeConnection(int sd){
    shutdown(sd, SHUT_WR);
    close(sd);
}
