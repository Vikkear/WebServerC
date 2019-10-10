#include "../include/includer.h"

// Closes the connection to the socket
void closeConnection(int sd){
    shutdown(sd, SHUT_WR);
    close(sd);
}
