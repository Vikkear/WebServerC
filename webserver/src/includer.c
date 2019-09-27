#include "../include/includer.h"

void closeConnection(int sd){
    shutdown(sd, SHUT_WR);
    close(sd);
    exit(0);
}
