#include "klient.h"


// Funkcia na prijímanie správ od servera
void *receive_messages(void *arg) {
    KlientStav *stav = (KlientStav *)arg;
    char buffer[256];

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = read(stav->client_socket, buffer, sizeof(buffer));
        if (bytes_received <= 0) {
            stav->klient_sa_ukoncuje = 1;
            break;
        }
        printf("%s\n", buffer);

        if (strstr(buffer, "Hra začala") != NULL) {
            printf("Návod: (play <farba><hodnota> - zahrať kartu, draw - potiahnuť kartu, exit - ukončiť): \n");
            stav->hra_bezi = 1;
        }

        if (strstr(buffer, "Výhra!\n") != NULL) {
            stav->hra_bezi = 0; 
            stav->klient_sa_ukoncuje = 1; 
            break;
        }
    }
    return NULL;
}

void *handle_user_input(void *arg) {
    KlientStav *stav = (KlientStav *)arg;
    char command[256];

    // Nastavenie STDIN na neblokujúci režim
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

    while (!stav->klient_sa_ukoncuje) {
        memset(command, 0, sizeof(command));
        int bytes_read = read(STDIN_FILENO, command, sizeof(command) - 1);
        if (bytes_read > 0) {
            command[bytes_read] = '\0';
            command[strcspn(command, "\n")] = 0;

            if (!stav->hra_bezi) {
                if (stav->is_host) {
                    printf("Si host! Zadajte príkaz (start - spustiť hru, exit - ukončiť): \n");
                    if (strcmp(command, "start") == 0) {
                        // Príkaz START: Spustiť hru
                        write(stav->client_socket, "START_GAME", strlen("START_GAME"));
                    } else if (strcmp(command, "exit") == 0) {
                        // Príkaz EXIT: Ukončiť spojenie
                        write(stav->client_socket, "EXIT", strlen("EXIT"));
                        printf("Ukončujem spojenie...\n");
                        stav->klient_sa_ukoncuje = 1;
                        break;
                    }
                }
            } else {
                if (strncmp(command, "play ", 5) == 0) {
                    // Príkaz PLAY: Zahrať kartu
                    write(stav->client_socket, command, strlen(command));
                } else if (strcmp(command, "draw") == 0) {
                    // Príkaz DRAW: Potiahnuť kartu
                    write(stav->client_socket, "draw", strlen("draw"));
                } else if (strcmp(command, "exit") == 0) {
                    // Príkaz EXIT: Ukončiť spojenie
                    write(stav->client_socket, "exit", strlen("exit"));
                    printf("Ukončujem spojenie...\n");
                    stav->klient_sa_ukoncuje = 1;
                    break;
                } else {
                    printf("Neznámy príkaz. Skúste znova.\n");
                }
            }
        } 
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

    KlientStav stav = {
        .hra_bezi = 0,
        .klient_sa_ukoncuje = 0,
        .client_socket = -1,
        .receive_thread = 0,
        .input_thread = 0,
        .is_host = is_host
    };

    // Pripojenie na server
    stav.client_socket = connect_to_server(server_ip, port);
    if (stav.client_socket < 0) {
        fprintf(stderr, "Chyba pri pripájaní na server\n");
        exit(1);
    }

    printf("Si pripojený na server %s:%d\n", server_ip, port);

    pthread_create(&stav.receive_thread, NULL, receive_messages, &stav);
    pthread_create(&stav.input_thread, NULL, handle_user_input, &stav);

    pthread_join(stav.receive_thread, NULL);
    pthread_join(stav.input_thread, NULL);
    
    active_socket_destroy(stav.client_socket);
    return 0;
}