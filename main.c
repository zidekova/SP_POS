#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void start_server(int port) {
    pid_t pid = fork();
    if (pid == 0) {
        // Detský proces - spustí server
        char port_str[16];
        snprintf(port_str, sizeof(port_str), "%d", port);
        execl("./server", "server", port_str, (char *)NULL);
        perror("execl"); // Ak execl zlyhá
        exit(1);
    } else if (pid < 0) {
        perror("fork");
    } else {
        printf("Server bol spustený na porte %d\n", port);
    }
}

void start_client(const char *server_ip, int port) {
    pid_t pid = fork();
    if (pid == 0) {
        // Detský proces - spustí klienta
        char port_str[16];
        snprintf(port_str, sizeof(port_str), "%d", port);
        execl("./klient", "klient", server_ip, port_str, (char *)NULL);
        perror("execl"); // Ak execl zlyhá
        exit(1);
    } else if (pid < 0) {
        perror("fork");
    } else {
        printf("Klient sa pripojil na server %s:%d\n", server_ip, port);
    }
}

int main() {
    int choice;
    int port;
    char server_ip[16];

    while (1) {
        printf("Vyberte možnosť:\n");
        printf("1. Vytvoriť novú hru (spustiť server)\n");
        printf("2. Pripojiť sa k existujúcej hre (spustiť klienta)\n");
        printf("3. Ukončiť program\n");
        printf("Voľba: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                printf("Zadajte port pre server: ");
                scanf("%d", &port);
                start_server(port);
                break;
            case 2:
                printf("Zadajte IP adresu servera: ");
                scanf("%s", server_ip);
                printf("Zadajte port servera: ");
                scanf("%d", &port);
                start_client(server_ip, port);
                break;
            case 3:
                printf("Ukončujem program...\n");
                exit(0);
            default:
                printf("Neplatná voľba!\n");
                break;
        }
    }

    return 0;
}