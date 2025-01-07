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

#define BUFFER_SIZE 1024

void *receive_messages(void *arg) {
    int client_socket = *(int *)arg;
    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = read(client_socket, buffer, BUFFER_SIZE);
        if (bytes_received <= 0) {
            printf("Server sa odpojil\n");
            break;
        }
        printf("Prijatá správa: %s\n", buffer);
    }

    return NULL;
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

    // Vlákno na prijímanie správ od servera
    pthread_t thread;
    pthread_create(&thread, NULL, receive_messages, &client_socket);

    // Hlavné vlákno na posielanie správ na server
    char buffer[BUFFER_SIZE];
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        fgets(buffer, BUFFER_SIZE, stdin);
        write(client_socket, buffer, strlen(buffer));
    }

    close(client_socket);
    return 0;
}