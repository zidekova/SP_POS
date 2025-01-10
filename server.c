// server.c
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
#include "balicek_kariet.h"
#include "pravidla.h"

void posli_spravu(int socket, const char* sprava) {
    if (write(socket, sprava, strlen(sprava)) < 0) {
        perror("Chyba pri odosielaní správy");
    }
}

void posli_vsetkym(Hra *hra, const char* sprava) {
    //pthread_mutex_lock(&hra->mutex_hra);

    for (int i = 0; i < hra->pocet_hracov; i++) {
        if (hra->hraci[i].je_aktivny && hra->hraci[i].socket != -1) {
            if (write(hra->hraci[i].socket, sprava, strlen(sprava)) < 0) {
                perror("Chyba pri odosielaní správy klientovi");
            }
        }
    }
    //pthread_mutex_unlock(&hra->mutex_hra);
}

void posli_info_o_karte_na_vrchu(Hra *hra) {
    char sprava[50];
    snprintf(sprava, sizeof(sprava), "Karta na vrchu: %c%c\n", hra->karta_na_vrchu.farba, hra->karta_na_vrchu.hodnota);
    posli_vsetkym(hra, sprava);
}

void posli_info_o_poradi(Hra *hra) {
    char sprava[50];
    snprintf(sprava, sizeof(sprava), "Na ťahu je hráč %d\n", hra->aktualny_hrac + 1);
    posli_vsetkym(hra, sprava);
}

void vypis_hracove_karty(Hrac* hrac)
{
    if (hrac->je_aktivny) {
        char sprava[256];
        snprintf(sprava, sizeof(sprava), "Tvoje karty: ");
        for (int j = 0; j < hrac->pocet_kariet_v_ruke; j++) {
            char karta_str[10];
            snprintf(karta_str, sizeof(karta_str), "%c%c ", hrac->karty_v_ruke[j].farba, hrac->karty_v_ruke[j].hodnota);
            strcat(sprava, karta_str);
        }
        posli_spravu(hrac->socket, sprava);
    }
    
}

void *handle_client(void *arg) {
    ClientData *client_data = (ClientData *)arg;
    int client_socket = client_data->client_socket;
    Hra *hra = client_data->hra;
    char buffer[256];

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = read(client_socket, buffer, sizeof(buffer));

        if (bytes_received <= 0) {
            printf("Klient sa odpojil. Socket: %d\n", client_socket);
            close(client_socket);

            // inicializacia pola hracov
            pthread_mutex_lock(&hra->mutex_hra);
            for (int i = 0; i < hra->pocet_hracov; i++) {
                if (hra->hraci[i].socket == client_socket) {
                    hra->hraci[i].je_aktivny = 0;
                    hra->hraci[i].socket = -1;
                    hra->pocet_hracov--;
                    printf("Počet prihlásených hráčov: %d\n", hra->pocet_hracov);
                    break;
                }
            }
            pthread_mutex_unlock(&hra->mutex_hra);
            pthread_exit(NULL);  
            break;
        }

        if (strcmp(buffer, "START_GAME") == 0) {
            if (hra->pocet_hracov < 2 || hra->pocet_hracov > 4) {
                posli_spravu(client_socket, "Hra nemôže začať. Počet hráčov musí byť medzi 2 a 4.\n");
                continue;
            }
            
            // Oznám všetkým hráčom, že hra začala
            posli_vsetkym(hra, "Hra začala\n");

            inicializuj_balicek(hra->volne, &hra->pocet_volnych_kariet);
            zamiesaj_balicek(hra->volne, hra->pocet_volnych_kariet);
            rozdaj_karty_hracom(hra);

            posli_info_o_karte_na_vrchu(hra);

            hra->aktualny_hrac = 0;
            posli_info_o_poradi(hra);

            for (int i = 0; i < hra->pocet_hracov; i++) {
                vypis_hracove_karty(&hra->hraci[i]);
            }
        }

        if (strncmp(buffer, "play ", 5) == 0) {
            char farba = buffer[5];
            char hodnota = buffer[6];

            pthread_mutex_lock(&hra->mutex_hra);

            // Skontroluj, či je ťah platný
            Karta hracova_karta = {farba, hodnota};
            if (je_platny_tah(hra->karta_na_vrchu, hracova_karta)) {
                // Odhoď kartu do kopy
                int index_karty = -1;
                for (int i = 0; i < hra->hraci[hra->aktualny_hrac].pocet_kariet_v_ruke; i++) {
                    if (hra->hraci[hra->aktualny_hrac].karty_v_ruke[i].farba == farba &&
                        hra->hraci[hra->aktualny_hrac].karty_v_ruke[i].hodnota == hodnota) {
                        index_karty = i;
                        break;
                    }
                }

                if (index_karty != -1) {
                    odhod_kartu_do_kopy(hra, &hra->hraci[hra->aktualny_hrac], index_karty);
                    hra->karta_na_vrchu = hracova_karta;
                    
                    // Spracuj špeciálnu kartu
                    je_sedmicka(hracova_karta, hra);
                    je_eso(hracova_karta, hra);
                    je_menic(hracova_karta, hra);

                    // Kontrola, či hráč vyhral
                    if (kontrola_vyhry(&hra->hraci[hra->aktualny_hrac])) {
                        char sprava[100];
                        printf("VYHRA!!!!!!!!\n");
                        //snprintf(sprava, sizeof(sprava), "Hráč %d vyhral hru!\n", hra->hraci[hra->aktualny_hrac].socket);
                        posli_vsetkym(hra, "Hráč vyhral hru!\n");
                        hra->hra_bezi = 0; // Ukonči hru
                        pthread_mutex_unlock(&hra->mutex_hra);
                        break;
                    }

                    // Prepni na ďalšieho hráča
                    hra->aktualny_hrac = (hra->aktualny_hrac + 1) % hra->pocet_hracov;
                }
            } else {
                posli_spravu(client_socket, "Neplatný ťah!\n");
            }
            
            pthread_mutex_unlock(&hra->mutex_hra);

            // Oznám všetkým hráčom novú kartu na vrchu
            char sprava[50];
            posli_info_o_karte_na_vrchu(hra);

            // Prepni na ďalšieho hráča
            posli_info_o_poradi(hra);
            
            // Po aktualizácii ťahu
            snprintf(sprava, sizeof(sprava), "Si na rade\n");
            posli_spravu(hra->hraci[hra->aktualny_hrac].socket, sprava);
            vypis_hracove_karty(&hra->hraci[hra->aktualny_hrac]);
        }

        if (strcmp(buffer, "draw") == 0) {
            pthread_mutex_lock(&hra->mutex_hra);

            if (hra->pocet_volnych_kariet <= 0) {
                // Ak sú voľné karty prázdne, presuň karty z kopy
                presun_karty_z_kopy_do_volnych(hra);
            }

            // Potiahni kartu z balíčka
            potiahnut_kartu(hra, &hra->hraci[hra->aktualny_hrac]);

            pthread_mutex_unlock(&hra->mutex_hra);

            // Oznám hráčovi, že si potiahol kartu
            char sprava[50];
            snprintf(sprava, sizeof(sprava), "Potiahol si si kartu: %c%c\n",
                     hra->hraci[hra->aktualny_hrac].karty_v_ruke[hra->hraci[hra->aktualny_hrac].pocet_kariet_v_ruke - 1].farba,
                     hra->hraci[hra->aktualny_hrac].karty_v_ruke[hra->hraci[hra->aktualny_hrac].pocet_kariet_v_ruke - 1].hodnota);
            posli_spravu(hra->hraci[hra->aktualny_hrac].socket, sprava);

            // Prepni na ďalšieho hráča
            hra->aktualny_hrac = (hra->aktualny_hrac + 1) % hra->pocet_hracov;

            // Oznám všetkým hráčom novú kartu na vrchu
            posli_info_o_karte_na_vrchu(hra);

            // Prepni na ďalšieho hráča
            posli_info_o_poradi(hra);
            
            // Po aktualizácii ťahu
            snprintf(sprava, sizeof(sprava), "Si na rade\n");
            posli_spravu(hra->hraci[hra->aktualny_hrac].socket, sprava);
            vypis_hracove_karty(&hra->hraci[hra->aktualny_hrac]);
        }

        if (strcmp(buffer, "exit") == 0) {
            printf("Klient požiadal o ukončenie spojenia. Socket: %d\n", client_socket);
            close(client_socket);

            pthread_mutex_lock(&hra->mutex_hra);
            for (int i = 0; i < hra->pocet_hracov; i++) {
                if (hra->hraci[i].socket == client_socket) {
                    hra->hraci[i].je_aktivny = 0;
                    hra->hraci[i].socket = -1;
                    hra->pocet_hracov--;
                    printf("Počet prihlásených hráčov: %d\n", hra->pocet_hracov);
                    break;
                }
            }
            pthread_mutex_unlock(&hra->mutex_hra);
            break;
        }
    }

    printf("Ukončujem vláknové spojenie pre socket: %d\n", client_socket);
    free(client_data);
    return NULL;
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    if (argc < 2) {
        fprintf(stderr, "Použitie: %s <port>\n", argv[0]);
        exit(1);
    }

    Hra hra = {
        .pocet_hracov = 0,
        .hra_bezi = 0,
        .mutex_hra = PTHREAD_MUTEX_INITIALIZER,
        .aktualny_hrac = 0
    };

    for (int i = 0; i < MAX_POCET_HRACOV; i++) {
        hra.hraci[i].socket = -1;
        hra.hraci[i].pocet_kariet_v_ruke = 0;
        hra.hraci[i].je_aktivny = 0;
    }

    int port = atoi(argv[1]);
    int server_socket = passive_socket_init(port);
    if (server_socket < 0) {
        fprintf(stderr, "Chyba pri inicializácii servera\n");
        exit(1);
    }

    for (int i = 0; i < MAX_POCET_HRACOV; i++) {
        hra.client_sockets[i] = -1;
    }

    printf("Server počúva na porte %d...\n", port);

    while (1) {
        int client_socket = passive_socket_wait_for_client(server_socket);
        if (client_socket < 0) {
            perror("Chyba pri prijímaní klienta");
            continue;
        }

        pthread_mutex_lock(&hra.mutex_hra);
        if (hra.pocet_hracov < MAX_POCET_HRACOV) {
            for (int i = 0; i < MAX_POCET_HRACOV; i++) {
                if (hra.client_sockets[i] == -1) {
                    hra.client_sockets[i] = client_socket;
                    hra.hraci[i].socket = client_socket;
                    hra.hraci[i].je_aktivny = 1;
                    break;
                }
            }
            hra.pocet_hracov++;
            printf("Počet prihlásených hráčov: %d\n", hra.pocet_hracov);
        } else {
            printf("Maximálny počet hráčov dosiahnutý.\n");
            close(client_socket);
        }
        pthread_mutex_unlock(&hra.mutex_hra);

        ClientData *client_data = malloc(sizeof(ClientData));
        client_data->client_socket = client_socket;
        client_data->hra = &hra;

        pthread_t thread;
        pthread_create(&thread, NULL, handle_client, client_data);
        pthread_detach(thread);
    }

    printf("Server sa ukončuje...\n");
    passive_socket_destroy(server_socket);
    return 0;
}
