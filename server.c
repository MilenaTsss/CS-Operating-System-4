#define MAX_NUM_OF_GUNS 50
#define RECEIVED_BUFFER_SIZE MAX_NUM_OF_GUNS * sizeof (int)/* Size of receive buffer */

#include "helper.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage:  %s <Server Port> <Field Size> <Number of guns>\n", argv[0]);
        exit(1);
    }

    unsigned short serverPort = atoi(argv[1]);
    int serverSocket = createSocket();
    int field_size = atoi(argv[2]);
    int num_of_guns = atoi(argv[3]);
    int shots[MAX_NUM_OF_GUNS + 1];

    struct sockaddr_in serverAddress;
    setServerAddress(&serverAddress, serverPort);
    bindServer(serverSocket, &serverAddress);

    struct sockaddr_in fromAddr;
    unsigned int fromSize = sizeof(fromAddr);
    struct sockaddr_in clientAddress[2];
    unsigned int clientLen = sizeof(struct sockaddr_in); /* Length of client address data structure */

    for (int i = 0; i < 2; ++i) {
        int id;
        if ((recvfrom(serverSocket, &id, 4, 0,
                      (struct sockaddr *) &fromAddr, &fromSize)) < 0) {
            dieWithError("recvfrom() failed");
        }
        clientAddress[id] = fromAddr;
        printf("Handling client %s with country id %d\n", inet_ntoa(fromAddr.sin_addr), id);
        int buff[] = {field_size, num_of_guns};
        if (sendto(serverSocket, buff, sizeof(buff), 0, (struct sockaddr *)
                   &fromAddr, sizeof(fromAddr)) != sizeof(buff)) {
            dieWithError("sendto() sent a different number of bytes than expected");
        }
    }

    int current_country = 0;
    shots[0] = 1;
    shots[1] = -2;

    printf("Начинаем войну\n");
    for (;;) {
        int receivedMessageSize = shots[0];
        if (receivedMessageSize == -1) {
            break;
        }

        if (sendto(serverSocket, &shots[1], sizeof(int) * receivedMessageSize, 0, (struct sockaddr *)
                   &clientAddress[current_country], clientLen) != sizeof(int) * receivedMessageSize) {
            dieWithError("sendto() sent a different number of bytes than expected");
        }

        sleep(1);
        /* Receive message from client */
        if ((receivedMessageSize = recvfrom(serverSocket, &shots[1], RECEIVED_BUFFER_SIZE, 0,
                      (struct sockaddr *) &fromAddr, &fromSize)) < 0) {
            dieWithError("recvfrom() failed");
        }
        if (clientAddress[current_country].sin_addr.s_addr != fromAddr.sin_addr.s_addr ||
                clientAddress[current_country].sin_port != fromAddr.sin_port ) {
            dieWithError("Error: received a packet from unknown source.\n");
        }
        shots[0] = receivedMessageSize / sizeof(int);
        if (shots[1] == -1) {
            break;
        }
        current_country = 1 - current_country;
    }
    printf("Война закончена\n");
    shots[1] = -10;
    if (sendto(serverSocket, &shots[1], sizeof(int), 0, (struct sockaddr *)
               &clientAddress[0], clientLen) != sizeof(int)) {
        dieWithError("sendto() sent a different number of bytes than expected");
    }
    if (sendto(serverSocket, &shots[1], sizeof(int), 0, (struct sockaddr *)
               &clientAddress[1], clientLen) != sizeof(int)) {
        dieWithError("sendto() sent a different number of bytes than expected");
    }
    sleep(2);
    close(serverSocket);
}