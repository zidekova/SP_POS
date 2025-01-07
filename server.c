#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "sockets-lib/socket.h"

#define SOCKET_PC 9999

void *handle_client(void *arg) {
    int client_socket = *(int *)arg;
    free(arg);

    char buffer[256];
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int n = read(client_socket, buffer, sizeof(buffer) - 1);
        if (n <= 0) {
            printf("Klient sa odpojil.\n");
            break;
        }
        printf("Sprava od klienta: %s\n", buffer);

        // Odošli odpoveď klientovi
        write(client_socket, "Server prijal spravu", 20);
    }

    close(client_socket);
    return NULL;
}

int main() {
    int fd_passive = passive_socket_init(SOCKET_PC); // Inicializácia pasívneho soketu
    if (fd_passive < 0) {
        fprintf(stderr, "Chyba pri inicializacii servera.\n");
        return 1;
    }

    printf("Server beží na porte %d. Čakám na pripojenie klientov...\n", SOCKET_PC);

    while (1) {
        int fd_active = passive_socket_wait_for_client(fd_passive); // Čakáme na pripojenie klienta
        if (fd_active < 0) {
            perror("Chyba pri čakaní na klienta");
            continue; // Ak došlo k chybe, pokračujeme v čakaní na ďalšieho klienta
        }

        printf("Nový klient sa pripojil.\n");

        // Dynamicky alokujeme pamäť pre soket klienta
        int *client_socket = malloc(sizeof(int));
        *client_socket = fd_active;

        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_client, client_socket) != 0) {
            perror("Chyba pri vytváraní vlákna");
            free(client_socket); // Uvoľníme alokovanú pamäť v prípade chyby
            continue;
        }

        pthread_detach(thread); // Odpojíme vlákno, aby sa automaticky vyčistilo po dokončení
    }

    active_socket_destroy(fd_passive); // Uzatvoríme pasívny soket
    return 0;
}
