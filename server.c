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

#define MAX_PLAYERS 10
#define BUFFER_SIZE 1024

typedef struct {
    int socket;
    struct sockaddr_in address;
} Client;

Client clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void *handle_client(void *arg) {
    int client_socket = *(int *)arg;
    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = read(client_socket, buffer, BUFFER_SIZE);
        if (bytes_received <= 0) {
            // Klient sa odpojil
            printf("Klient sa odpojil\n");
            close(client_socket);
            pthread_mutex_lock(&clients_mutex);
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].socket == client_socket) {
                    clients[i].socket = 0;
                    break;
                }
            }
            pthread_mutex_unlock(&clients_mutex);
            break;
        }

        // Spracovanie správy od klienta (napr. pohyb hráča)
        printf("Prijatá správa: %s\n", buffer);

        // Poslanie správy všetkým klientom
        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].socket != 0) {
                write(clients[i].socket, buffer, strlen(buffer));
            }
        }
        pthread_mutex_unlock(&clients_mutex);
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Použitie: %s <port>\n", argv[0]);
        exit(1);
    }

    int port = atoi(argv[1]);
    int server_socket = passive_socket_init(port);
    if (server_socket < 0) {
        fprintf(stderr, "Chyba pri inicializácii servera\n");
        exit(1);
    }

    // Inicializácia poľa klientov
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].socket = 0;
    }

    printf("Server počúva na porte %d...\n", port);

    while (1) {
        struct sockaddr_in client_address;
        socklen_t client_len = sizeof(client_address);
        int client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_len);
        if (client_socket < 0) {
            perror("Chyba pri prijímaní klienta");
            continue;
        }

        // Pridanie nového klienta do poľa
        pthread_mutex_lock(&clients_mutex);
        int added = 0;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].socket == 0) {
                clients[i].socket = client_socket;
                clients[i].address = client_address;
                printf("Nový klient pripojený: %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
                added = 1;
                break;
            }
        }
        pthread_mutex_unlock(&clients_mutex);

        if (!added) {
            fprintf(stderr, "Nie je možné pripojiť ďalšieho klienta (plný počet klientov)\n");
            close(client_socket);
            continue;
        }

        // Vytvorenie vlákna pre klienta
        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_client, &client_socket) != 0) {
            perror("Chyba pri vytváraní vlákna");
            close(client_socket);
        }
        pthread_detach(thread);
    }

    close(server_socket);
    return 0;
}