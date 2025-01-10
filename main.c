// main.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h> 

int start_server(int port) {
    pid_t pid = fork();
    if (pid == 0) {
        // Detský proces - spustí server
        char port_str[16];
        snprintf(port_str, sizeof(port_str), "%d", port);
        execl("./server", "server", port_str, (char *)NULL);
        perror("Chyba pri spustení servera");
        exit(1);
    } else if (pid < 0) {
        perror("fork");
        return -1;
    } else {
        // Rodičovský proces - server je spustený v pozadí
        printf("Server bol spustený na porte %d\n", port);
        return 0;
    }
}

void start_client(const char *server_ip, int port, int is_host) {
    pid_t pid = fork();
    if (pid == 0) {
        // Detský proces - spustí klienta
        char port_str[16];
        snprintf(port_str, sizeof(port_str), "%d", port);
        if (is_host) {
            execl("./klient", "klient", server_ip, port_str, "host", (char *)NULL);
        } else {
            execl("./klient", "klient", server_ip, port_str, (char *)NULL);
        }
        perror("execl"); // Ak execl zlyhá
        exit(1);
    } else if (pid < 0) {
        perror("fork");
    } else {
        printf("Klient sa pripojil na server %s:%d\n", server_ip, port);
        sleep(2);
    }
}

int main() {
    int choice;
    int port;

    printf("KARTOVÁ HRA FARAÓN\n--------------------------\n");
    printf("Vyberte si možnosť:\n");
    printf("1. Vytvoriť novú hru\n");
    printf("2. Pripojiť sa k existujúcej hre\n");
    printf("3. Ukončiť program\n");
    printf("Voľba: ");
    scanf("%d", &choice);

    switch (choice) {
        case 1:
            printf("Zadajte port pre server: ");
            scanf("%d", &port);
            start_server(port);
            sleep(2);
            start_client("127.0.0.1", port, 1);
            break;
        case 2:
            printf("Zadajte port servera: ");
            scanf("%d", &port);
            start_client("127.0.0.1", port, 0);
            break;
        case 3:
            printf("Ukončujem program...\n");
            exit(0);
        default:
            printf("Neplatná voľba!\n");
            break;
    }
    
    while (1) {
        sleep(1); // Program beží na pozadí
    }

    return 0;
}
