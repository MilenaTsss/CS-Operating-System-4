//
// Created by milena on 22.05.2023.
//

#include <stdio.h>  /* for perror(), printf(), fprintf() */
#include <stdlib.h> /* for exit(), atoi() */
#include <string.h> /* for memset() */
#include <unistd.h> /* for close() */

#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */


void dieWithError(char *message) {
    perror(message);
    exit(1);
}

int createSocket() {
    int sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        dieWithError("socket() failed");
    }
    return sock;
}

void setServerAddressWithIP(struct sockaddr_in* serverAddress, char* serverIP, unsigned short serverPort) {
    memset(serverAddress, 0, sizeof(*serverAddress));    /* Zero out structure */
    serverAddress->sin_family = AF_INET;                  /* Internet address family */
    serverAddress->sin_addr.s_addr = inet_addr(serverIP); /* Server IP address */
    serverAddress->sin_port = htons(serverPort);          /* Server port */
}

void setServerAddress(struct sockaddr_in* serverAddress, unsigned short serverPort) {
    memset(serverAddress, 0, sizeof(*serverAddress));    /* Zero out structure */
    serverAddress->sin_family = AF_INET;                  /* Internet address family */
    serverAddress->sin_addr.s_addr = htonl(INADDR_ANY); /* Server IP address */
    serverAddress->sin_port = htons(serverPort);          /* Server port */
}

void bindServer(int sock, struct sockaddr_in* serverAddress) {
    if (bind(sock, (struct sockaddr *) serverAddress, sizeof(*serverAddress)) < 0) {
        dieWithError("bind() failed");
    }
}
