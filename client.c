#include "helper.h"
#include "war.h"

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "<Server IP> <Server Port> <Country number>\n");
        exit(1);
    }
    srand(time(NULL));

    int sock = createSocket();
    char *serverIP = argv[1];
    unsigned short serverPort = atoi(argv[2]);
    int id = atoi(argv[3]);

    struct sockaddr_in serverAddress;
    setServerAddressWithIP(&serverAddress, serverIP, serverPort);
    if (sendto(sock, &id, sizeof(int), 0, (struct sockaddr *)
               &serverAddress, sizeof(serverAddress)) != sizeof(int)) {
        dieWithError("sendto() sent a different number of bytes than expected");
    }

    struct sockaddr_in fromAddr;
    unsigned int fromSize = sizeof(fromAddr);
    int buff[2];
    int number;
    if ((recvfrom(sock, buff, sizeof(buff), 0,
                               (struct sockaddr *) &fromAddr, &fromSize)) < 0) {
        dieWithError("recvfrom() failed");
    }
    if (serverAddress.sin_addr.s_addr != fromAddr.sin_addr.s_addr) {
        dieWithError("Error: received a packet from unknown source.\n");
    }
    field_size = buff[0];
    num_of_guns = buff[1];
    printf("Field size = %d\nNumber of guns = %d\n", field_size, num_of_guns);

    own_field = malloc(field_size * field_size * sizeof (point_t));
    foreign_field = malloc(field_size * field_size * sizeof (point_t));
    fill_field(own_field);
    simple_fill_field(foreign_field);

    int receivedBuffer[num_of_guns];
    int shots[num_of_guns];

    if ((number = recvfrom(sock, receivedBuffer, sizeof(receivedBuffer), 0,
                  (struct sockaddr *) &fromAddr, &fromSize)) < 0) {
        dieWithError("recvfrom() failed");
    }
    if (serverAddress.sin_addr.s_addr != fromAddr.sin_addr.s_addr) {
        dieWithError("Error: received a packet from unknown source.\n");
    }
    for (;number > 0;) {
        if (number == 4 && receivedBuffer[0] == -10) {
            break;
        }
        printf("%d\n", number);
        if (receivedBuffer[0] >= 0) {
            printf("Попадание в: ");
            for (int i = 0; i < number / 4; ++i) {
                printf("%d ", receivedBuffer[i]);
                if (own_field[receivedBuffer[i]].type == ALIVE_GUN) {
                    own_field[receivedBuffer[i]].type = DEAD_GUN;
                }
                if (own_field[receivedBuffer[i]].type == EMPTY_POINT) {
                    own_field[receivedBuffer[i]].type = USED_POINT;
                }
            }
            printf("\n");
        }
        if (!check_status()) {
            shots[0] = -1;
            printf("Проигрыш\n");
            if (sendto(sock, shots, sizeof(int), 0, (struct sockaddr *)
                       &serverAddress, sizeof(serverAddress)) != sizeof(int)) {
                dieWithError("sendto() sent a different number of bytes than expected");
            }
            break;
        }
        printf("Нападаем\n");
        int *p = shots;
        number = generate_targets(p);
        printf("Нападение совершено\n");
        if (sendto(sock, shots, sizeof(int) * number, 0, (struct sockaddr *)
                   &serverAddress, sizeof(serverAddress)) != sizeof(int) * number) {
            dieWithError("sendto() sent a different number of bytes than expected");
        }

        sleep(1);

        if ((number = recvfrom(sock, receivedBuffer, sizeof(receivedBuffer), 0,
                               (struct sockaddr *) &fromAddr, &fromSize)) < 0) {
            dieWithError("recvfrom() failed");
        }
        if (serverAddress.sin_addr.s_addr != fromAddr.sin_addr.s_addr) {
            dieWithError("Error: received a packet from unknown source.\n");
        }
    }

    printf("\n"); /* Print a final linefeed */

    if (check_status()) {
        printf("Выигрыш\n");
    }

    close(sock);
    free(own_field);
    free(foreign_field);

    exit(0);
}
