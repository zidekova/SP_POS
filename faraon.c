// faraon.c
#include "faraon.h"

int start_server(int port) {
    pid_t pid = fork();
    if (pid == 0) {
        char port_str[16];
        snprintf(port_str, sizeof(port_str), "%d", port);
        execl("./server", "server", port_str, (char *)NULL);
        perror("Chyba pri spustení servera");
        exit(1);
    } else if (pid < 0) {
        perror("fork");
        return -1;
    } else {
        printf("Server bol spustený na porte %d\n", port);
        return pid; 
    }
    return 0;
}

int start_client(const char *server_ip, int port, int is_host) {
    pid_t pid = fork();
    if (pid == 0) {
        char port_str[16];
        snprintf(port_str, sizeof(port_str), "%d", port);
        if (is_host) {
            execl("./klient", "klient", server_ip, port_str, "host", (char *)NULL);
        } else {
            execl("./klient", "klient", server_ip, port_str, (char *)NULL);
        }
        perror("execl");
        exit(1);
    } else if (pid < 0) {
        perror("fork");
    } else {
        printf("Klient sa pripojil na server %s:%d\n", server_ip, port);
        return pid; 
    }
    return 0;
}

int main() {
    int choice;
    int port;
    pid_t server_pid = -1; 

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
            server_pid = start_server(port);
            sleep(2);
            start_client("localhost", port, 1);
            break;
        case 2:
            printf("Zadajte port servera: ");
            scanf("%d", &port);

            pid_t client_pid = start_client("localhost", port, 0);

            if (client_pid > 0) {
                int status;
                waitpid(client_pid, &status, 0);
                printf("Klient bol ukončený. Ukončujem program...\n");
            } else {
                printf("Chyba pri spustení klienta.\n");
            }
            break;
        case 3:
            printf("Ukončujem program...\n");
            exit(0);
        default:
            printf("Neplatná voľba!\n");
            break;
    }
    
    if (server_pid != -1) {
        int status;
        waitpid(server_pid, &status, 0);
        printf("Server bol ukončený. Ukončujem program...\n");
        exit(0);
    }

    return 0;
}
