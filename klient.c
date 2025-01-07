#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "sockets-lib/socket.h"

#define SOCKET_PC 9999

int main() {
    int client_socket = connect_to_server("localhost", SOCKET_PC);
    if (client_socket < 0) {
        fprintf(stderr, "Nepodarilo sa pripojit na server.\n");
        return 1;
    }

    printf("Pripojeny na server. Zadajte spravu:\n");

    char buffer[256];
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        fgets(buffer, sizeof(buffer), stdin);

        // Odstránenie nového riadku
        buffer[strcspn(buffer, "\n")] = '\0';

        // Odoslanie správy serveru
        write(client_socket, buffer, strlen(buffer));

        // Čítanie odpovede zo serveru
        memset(buffer, 0, sizeof(buffer));
        int n = read(client_socket, buffer, sizeof(buffer) - 1);
        if (n <= 0) {
            printf("Server sa odpojil.\n");
            break;
        }
        printf("Odpoved od servera: %s\n", buffer);
    }

    active_socket_destroy(client_socket);
    return 0;
}
