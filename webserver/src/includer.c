#include "../include/includer.h"
useSyslog = 0;

// Closes the connection to the socket
void closeConnection(int sd){
    shutdown(sd, SHUT_WR);
    close(sd);
}
