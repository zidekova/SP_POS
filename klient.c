#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "sockets-lib/socket.h"
#include "balicek_kariet.h"

#define SOCKET_PC 9999

int main() {
    printf("Spustam server ako samostatny proces na pozadi...\n");
    pid_t pid = fork();
    if (pid == 0) {
        execl("./server", "server", NULL);
        perror("Chyba pri spustani servera");
        exit(1);
    }

    sleep(1);  // Dáme čas serveru na spustenie

    int client_socket = connect_to_server("localhost", SOCKET_PC);
    if (client_socket < 0) {
        fprintf(stderr, "Nepodarilo sa pripojit na server.\n");
        return 1;
    }

    Karta karta;
    while (1) {
        printf("Cakam na kartu od servera...\n");
        read(client_socket, &karta, sizeof(Karta));
        printf("Obdržaná karta: %c-%d\n", karta.farba, karta.hodnota);

        // Vykonaj akciu podľa karty
        printf("Vyber kartu na polozenie: ");
        scanf("%d", (int *)&karta.hodnota);  // Na testovanie
        write(client_socket, &karta, sizeof(Karta));
    }

    active_socket_destroy(client_socket);
    return 0;
}