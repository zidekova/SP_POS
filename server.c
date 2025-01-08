#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>  
#include <pthread.h>
#include <time.h>
#include "sockets-lib/socket.h"

#define MAX_HRA_COV 10  // Maximálny počet hráčov

int pocet_hracov = 0;
int hra_bezi = 0;
int client_sockets[MAX_HRA_COV];  // Zoznam socketov pripojených klientov
pthread_mutex_t mutex_hra = PTHREAD_MUTEX_INITIALIZER;

void posli_spravu(int socket, const char* sprava) {
    if (write(socket, sprava, strlen(sprava)) < 0) {
        perror("Chyba pri odosielaní správy");
    }
}

void posli_vsetkym(const char* sprava) {
    pthread_mutex_lock(&mutex_hra);
    printf("Posielam správu všetkým klientom: %s\n", sprava);

    for (int i = 0; i < MAX_HRA_COV; i++) {
        if (client_sockets[i] != -1) {
            if (write(client_sockets[i], sprava, strlen(sprava)) < 0) {
                perror("Chyba pri odosielaní správy klientovi");
            }
        }
    }
    pthread_mutex_unlock(&mutex_hra);
}

void *handle_client(void *arg) {
    int client_socket = *(int *)arg;
    char buffer[256];

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = read(client_socket, buffer, sizeof(buffer));
        if (bytes_received <= 0) {
            printf("Klient sa odpojil. Socket: %d\n", client_socket);
            close(client_socket);

            pthread_mutex_lock(&mutex_hra);
            for (int i = 0; i < MAX_HRA_COV; i++) {
                if (client_sockets[i] == client_socket) {
                    client_sockets[i] = -1;
                    pocet_hracov--;
                    printf("Počet prihlásených hráčov: %d\n", pocet_hracov);
                    break;
                }
            }
            pthread_mutex_unlock(&mutex_hra);
            break;
        }

        printf("Prijatá správa od klienta: %s\n", buffer);

        // Poslať správu všetkým ostatným klientom
        posli_vsetkym(buffer);
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

    // Inicializácia zoznamu socketov
    for (int i = 0; i < MAX_HRA_COV; i++) {
        client_sockets[i] = -1;
    }

    printf("Server počúva na porte %d...\n", port);

    while (1) {
        int client_socket = passive_socket_wait_for_client(server_socket);
        if (client_socket < 0) {
            perror("Chyba pri prijímaní klienta");
            continue;
        }

        pthread_mutex_lock(&mutex_hra);
        if (pocet_hracov < MAX_HRA_COV) {
            for (int i = 0; i < MAX_HRA_COV; i++) {
                if (client_sockets[i] == -1) {
                    client_sockets[i] = client_socket;
                    break;
                }
            }
            pocet_hracov++;
            printf("Počet prihlásených hráčov: %d\n", pocet_hracov);
        } else {
            printf("Maximálny počet hráčov dosiahnutý.\n");
            close(client_socket);
        }
        pthread_mutex_unlock(&mutex_hra);

        pthread_t thread;
        pthread_create(&thread, NULL, handle_client, &client_socket);
        pthread_detach(thread);
    }

    passive_socket_destroy(server_socket);
    return 0;
}

