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

// Funkcia na prijímanie správ od servera
void *receive_messages(void *arg) {
    int client_socket = *(int *)arg;
    char buffer[256];

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = read(client_socket, buffer, sizeof(buffer));
        if (bytes_received <= 0) {
            printf("Server sa odpojil\n");
            break;
        }
        printf("Správa od servera: %s\n", buffer);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Použitie: %s <server_ip> <port> [host]\n", argv[0]);
        exit(1);
    }

    const char *server_ip = argv[1];
    int port = atoi(argv[2]);
    int is_host = (argc > 3 && strcmp(argv[3], "host") == 0);

    // Pripojenie na server
    int client_socket = connect_to_server(server_ip, port);
    if (client_socket < 0) {
        fprintf(stderr, "Chyba pri pripájaní na server\n");
        exit(1);
    }

    printf("Si pripojený na server %s:%d\n", server_ip, port);

    // Vytvorenie vlákna na prijímanie správ od servera
    pthread_t thread;
    pthread_create(&thread, NULL, receive_messages, &client_socket);
    pthread_detach(thread);

    // Ak je klient hostiteľ, môže poslať príkaz na spustenie hry
    if (is_host) {
        printf("Si host hry. Môžeš zadať príkaz na spustenie hry.\n");
        while (1) {
            char command[256];
            printf("Zadajte príkaz (start - spustiť hru, exit - ukončiť, správa - poslať správu všetkým): \n");
            fgets(command, sizeof(command), stdin);
            command[strcspn(command, "\n")] = 0;  // Odstránenie nového riadku

            if (strcmp(command, "start") == 0) {
                write(client_socket, "START_GAME", strlen("START_GAME"));
                printf("Príkaz na spustenie hry bol odoslaný.\n");
            } else if (strcmp(command, "exit") == 0) {
                break;
            } else {
                // Poslanie správy všetkým klientom
                write(client_socket, command, strlen(command));
                printf("Správa bola odoslaná: %s\n", command);
            }
        }
    } else {
        // Ak nie je hostiteľ, klient čaká na správy od servera
        while (1) {
            sleep(1);
        }
    }

    // Zatvorenie socketu
    active_socket_destroy(client_socket);
    return 0;
}
