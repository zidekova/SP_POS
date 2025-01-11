#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fcntl.h>
#include "sockets-lib/socket.h"

int hra_bezi = 0;
int klient_sa_ukoncuje = 0;

// Funkcia na prijímanie správ od servera
void *receive_messages(void *arg) {
    int client_socket = *(int *)arg;
    char buffer[256];

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = read(client_socket, buffer, sizeof(buffer));
        if (bytes_received <= 0) {
            printf("Server sa odpojil\n");
            klient_sa_ukoncuje = 1;
            break;
        }
        printf("%s\n", buffer);

        // Ak server oznámil, že hra beží, nastavíme stav hry
        if (strstr(buffer, "Hra začala") != NULL) {
            printf("Návod: (play <farba><hodnota> - zahrať kartu, draw - potiahnuť kartu, exit - ukončiť): \n");
            hra_bezi = 1;
        }

        // Ak server oznámil, že niekto vyhral hru
        if (strstr(buffer, "Výhra!\n") != NULL) {
            hra_bezi = 0; // Ukonči hru
            klient_sa_ukoncuje = 1; // Signalizácia ukončenia
            break; // Ukonči slučku prijímania správ
        }
    }
    printf("Receive messages je ukoncene\n");
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

    // Nastaviť STDIN na neblokujúci režim
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

    // Hlavná slučka klienta
    while (!klient_sa_ukoncuje) {
        char command[256];

        int bytes_read = read(STDIN_FILENO, command, sizeof(command) - 1);
        if (bytes_read > 0) {
            command[bytes_read] = '\0';  // Ukončiť reťazec
            command[strcspn(command, "\n")] = 0; 

            if (!hra_bezi) {
                // Pred spustením hry
                if (is_host) {
                    if (strcmp(command, "start") == 0) {
                        // Príkaz START: Spustiť hru
                        write(client_socket, "START_GAME", strlen("START_GAME"));
                        printf("Príkaz na spustenie hry bol odoslaný.\n");
                    } else if (strcmp(command, "exit") == 0) {
                        // Príkaz EXIT: Ukončiť spojenie
                        write(client_socket, "EXIT", strlen("EXIT"));
                        printf("Ukončujem spojenie...\n");
                        klient_sa_ukoncuje = 1;
                        break;
                    } else {
                        // Poslanie správy všetkým klientom
                        write(client_socket, command, strlen(command));
                        printf("Správa bola odoslaná: %s\n", command);
                    }
                }
            } else {
                // Po spustení hry
                if (strncmp(command, "play ", 5) == 0) {
                    // Príkaz PLAY: Zahrať kartu
                    write(client_socket, command, strlen(command));
                    printf("Príkaz na zahranie karty bol odoslaný.\n");
                } else if (strcmp(command, "draw") == 0) {
                    // Príkaz DRAW: Potiahnuť kartu
                    write(client_socket, "draw", strlen("draw"));
                    printf("Príkaz na potiahnutie karty bol odoslaný.\n");
                } else if (strcmp(command, "exit") == 0) {
                    // Príkaz EXIT: Ukončiť spojenie
                    write(client_socket, "exit", strlen("exit"));
                    printf("Ukončujem spojenie...\n");
                    klient_sa_ukoncuje = 1;
                    break;
                } else {
                    printf("Neznámy príkaz. Skúste znova.\n");
                }
            }
        
        }
    }

    printf("Klient sa ukončuje...\n");
    pthread_join(thread, NULL);
    // Zatvorenie socketu
    active_socket_destroy(client_socket);
    return 0;
}