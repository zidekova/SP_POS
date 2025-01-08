#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "sockets-lib/socket.h"
#include "struktury.h"

#define BUFFER_SIZE 1024

void vypis_karty(char* karty_v_ruke, int pocet_kariet_v_ruke) {
    printf("Tvoje karty:\n");
    for (int i = 0; i < pocet_kariet_v_ruke; i++) {
        printf("%d: %c%c\n", i + 1, karty_v_ruke[2 * i], karty_v_ruke[2 * i + 1]);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Použitie: %s <server_ip> <port>\n", argv[0]);
        exit(1);
    }

    const char *server_ip = argv[1];
    int port = atoi(argv[2]);

    int client_socket = connect_to_server(server_ip, port);
    if (client_socket < 0) {
        fprintf(stderr, "Chyba pri pripájaní na server\n");
        exit(1);
    }

    printf("Pripojený na server %s:%d\n", server_ip, port);

    // Tu bude logika pre komunikáciu so serverom
    char karty_v_ruke[BUFFER_SIZE];
    int pocet_kariet_v_ruke = 0;

    // Prijatie počiatočných kariet od servera
    for (int i = 0; i < ZAC_POCET_KARIET; i++) {
        char karta[3];
        int bytes_received = read(client_socket, karta, sizeof(karta));
        if (bytes_received > 0) {
            karty_v_ruke[2 * pocet_kariet_v_ruke] = karta[0];
            karty_v_ruke[2 * pocet_kariet_v_ruke + 1] = karta[1];
            pocet_kariet_v_ruke++;
        }
    }

    // Hlavná slučka klienta
    while (1) {
        vypis_karty(karty_v_ruke, pocet_kariet_v_ruke);
        printf("Vyber kartu (1-%d) alebo potiahni kartu (0): ", pocet_kariet_v_ruke);
        int vyber;
        scanf("%d", &vyber);

        if (vyber == 0) {
            // Potiahnutie karty
            write(client_socket, "TAHANIE", 7);
            char nova_karta[3];
            int bytes_received = read(client_socket, nova_karta, sizeof(nova_karta));
            if (bytes_received > 0) {
                karty_v_ruke[2 * pocet_kariet_v_ruke] = nova_karta[0];
                karty_v_ruke[2 * pocet_kariet_v_ruke + 1] = nova_karta[1];
                pocet_kariet_v_ruke++;
                printf("Potiahol si kartu: %c%c\n", nova_karta[0], nova_karta[1]);
            }
        } else if (vyber >= 1 && vyber <= pocet_kariet_v_ruke) {
            // Zahranie karty
            char karta[3];
            karta[0] = karty_v_ruke[2 * (vyber - 1)];
            karta[1] = karty_v_ruke[2 * (vyber - 1) + 1];
            karta[2] = '\0';
            write(client_socket, karta, sizeof(karta));

            // Odstránenie karty z ruky
            for (int i = vyber - 1; i < pocet_kariet_v_ruke - 1; i++) {
                karty_v_ruke[2 * i] = karty_v_ruke[2 * (i + 1)];
                karty_v_ruke[2 * i + 1] = karty_v_ruke[2 * (i + 1) + 1];
            }
            pocet_kariet_v_ruke--;
        } else {
            printf("Neplatná voľba!\n");
        }
    }


    close(client_socket);
    return 0;
}