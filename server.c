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

#define MAX_HRA_COV 10  // Maximálny počet hráčov

Hrac hraci[MAX_POCET_HRACOV];
int pocet_hracov = 0;
int hra_bezi = 0;
int client_sockets[MAX_HRA_COV];  // Zoznam socketov pripojených klientov
pthread_mutex_t mutex_hra = PTHREAD_MUTEX_INITIALIZER;
int aktualny_hrac = 0;  // Index aktuálneho hráča v poradí
Karta karta_na_vrchu;

void posli_spravu(int socket, const char* sprava) {
    if (write(socket, sprava, strlen(sprava)) < 0) {
        perror("Chyba pri odosielaní správy");
    }
}

void posli_vsetkym(const char* sprava) {
    pthread_mutex_lock(&mutex_hra);
    printf("Posielam správu všetkým klientom: %s\n", sprava);

    for (int i = 0; i < pocet_hracov; i++) {
        if (hraci[i].je_aktivny && hraci[i].socket != -1) {
            if (write(hraci[i].socket, sprava, strlen(sprava)) < 0) {
                perror("Chyba pri odosielaní správy klientovi");
            }
        }
    }
    pthread_mutex_unlock(&mutex_hra);
}

void posli_info_o_karte_na_vrchu() {
    char sprava[50];
    snprintf(sprava, sizeof(sprava), "Karta na vrchu: %c%c\n", karta_na_vrchu.farba, karta_na_vrchu.hodnota);
    posli_vsetkym(sprava);
}

void posli_info_o_poradi() {
    char sprava[50];
    snprintf(sprava, sizeof(sprava), "Na ťahu je hráč %d\n", aktualny_hrac + 1);
    posli_vsetkym(sprava);
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
    int client_socket = *(int *)arg;
    char buffer[256];

    // Balíček kariet a počet kariet v balíčku
    //Karta balicek[POCET_KARIET_V_BALICKU];
    //int pocet_kariet_v_balicku = POCET_KARIET_V_BALICKU;

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = read(client_socket, buffer, sizeof(buffer));
        if (bytes_received <= 0) {
            printf("Klient sa odpojil. Socket: %d\n", client_socket);
            close(client_socket);

            // inicializacia pola hracov
            pthread_mutex_lock(&mutex_hra);
            for (int i = 0; i < pocet_hracov; i++) {
                if (hraci[i].socket == client_socket) {
                    hraci[i].je_aktivny = 0;
                    hraci[i].socket = -1;
                    pocet_hracov--;
                    printf("Počet prihlásených hráčov: %d\n", pocet_hracov);
                    break;
                }
            }
            pthread_mutex_unlock(&mutex_hra);
            break;
        }

        //printf("Prijatá správa od klienta: %s\n", buffer);

        if (strcmp(buffer, "START_GAME") == 0) {
            if (pocet_hracov < 2 || pocet_hracov > 4) {
                posli_spravu(client_socket, "Hra nemôže začať. Počet hráčov musí byť medzi 2 a 4.\n");
                continue;
            }
            
            // Oznám všetkým hráčom, že hra začala
            posli_vsetkym("Hra začala\n");

            Karta balicek[POCET_KARIET_V_BALICKU];
            inicializuj_balicek(balicek);
            zamiesaj_balicek(balicek, POCET_KARIET_V_BALICKU);
            rozdaj_karty_hracom(balicek, hraci, pocet_hracov, &karta_na_vrchu);

            posli_info_o_karte_na_vrchu();

            aktualny_hrac = 0;
            posli_info_o_poradi();

            for (int i = 0; i < pocet_hracov; i++) {
                vypis_hracove_karty(&hraci[i]);
            }
        }

        if (strncmp(buffer, "play ", 5) == 0) {
            char farba = buffer[5];
            char hodnota = buffer[6];

            // Aktualizuj kartu na vrchu
            karta_na_vrchu.farba = farba;
            karta_na_vrchu.hodnota = hodnota;

            // Odstráň kartu z ruky hráča
            for (int i = 0; i < hraci[aktualny_hrac].pocet_kariet_v_ruke; i++) {
                if (hraci[aktualny_hrac].karty_v_ruke[i].farba == farba &&
                    hraci[aktualny_hrac].karty_v_ruke[i].hodnota == hodnota) {
                    // Posuň karty v poli doľava
                    for (int j = i; j < hraci[aktualny_hrac].pocet_kariet_v_ruke - 1; j++) {
                        hraci[aktualny_hrac].karty_v_ruke[j] = hraci[aktualny_hrac].karty_v_ruke[j + 1];
                    }
                    hraci[aktualny_hrac].pocet_kariet_v_ruke--;
                    break;
                }
            }

            // Oznám všetkým hráčom novú kartu na vrchu
            char sprava[50];
            printf("Nova karta na vrchu je: %c%c\n", karta_na_vrchu.farba, karta_na_vrchu.hodnota);
            snprintf(sprava, sizeof(sprava), "Karta na vrchu: %c%c\n", karta_na_vrchu.farba, karta_na_vrchu.hodnota);
            posli_vsetkym(sprava);

            // Prepni na ďalšieho hráča
            aktualny_hrac = (aktualny_hrac + 1) % pocet_hracov;
            snprintf(sprava, sizeof(sprava), "Na ťahu je hráč %d\n", aktualny_hrac + 1);
            posli_vsetkym(sprava);

            // Potvrdenie klientovi
            posli_spravu(client_socket, "Karta bola úspešne zahraná.\n");
            
            // Po aktualizácii ťahu
            snprintf(sprava, sizeof(sprava), "Si na rade\n");
            posli_spravu(hraci[aktualny_hrac].socket, sprava);
            vypis_hracove_karty(&hraci[aktualny_hrac]);
        }

        /*if (strncmp(buffer, "PLAY ", 5) == 0) {
            printf("Správa začína na 'PLAY '. Spracovávam...\n");
            char farba = buffer[5];
            char hodnota = buffer[6];

            pthread_mutex_lock(&mutex_hra);

            // Kontrola, či hráč môže zahrať kartu
            if (farba == karta_na_vrchu.farba || hodnota == karta_na_vrchu.hodnota) {
                // Aktualizuj kartu na vrchu
                karta_na_vrchu.farba = farba;
                karta_na_vrchu.hodnota = hodnota;

                // Odstráň kartu z ruky hráča
                int karta_najdena = 0;
                for (int i = 0; i < hraci[aktualny_hrac].pocet_kariet_v_ruke; i++) {
                    if (hraci[aktualny_hrac].karty_v_ruke[i].farba == farba &&
                        hraci[aktualny_hrac].karty_v_ruke[i].hodnota == hodnota) {
                        // Posuň karty v poli doľava
                        for (int j = i; j < hraci[aktualny_hrac].pocet_kariet_v_ruke - 1; j++) {
                            hraci[aktualny_hrac].karty_v_ruke[j] = hraci[aktualny_hrac].karty_v_ruke[j + 1];
                        }
                        hraci[aktualny_hrac].pocet_kariet_v_ruke--;
                        karta_najdena = 1;
                        break;
                    }
                }
                
                if (karta_najdena) {
                    // Oznám všetkým hráčom novú kartu na vrchu
                    posli_vsetkym("Karta na vrchu: %c%c\n", karta_na_vrchu.farba, karta_na_vrchu.hodnota);

                    // Prepni na ďalšieho hráča
                    aktualny_hrac = (aktualny_hrac + 1) % pocet_hracov;
                    posli_vsetkym("Na ťahu je hráč %d\n", aktualny_hrac + 1);

                    // Potvrdenie klientovi
                    posli_spravu(client_socket, "Karta bola úspešne zahraná.\n");
                } else {
                    posli_spravu(client_socket, "Chyba: Karta sa nenašla v tvojej ruke!\n");
                }
            } else {
                posli_spravu(client_socket, "Nemôžeš zahrať túto kartu!\n");
            }

            pthread_mutex_unlock(&mutex_hra);
        } else {
            printf("Správa nezačína na 'PLAY '. Obsah: %s\n", buffer);
        }

        if (strcmp(buffer, "DRAW") == 0) {
            pthread_mutex_lock(&mutex_hra);

            if (pocet_kariet_v_balicku > 0) {
                Karta potiahnuta_karta = balicek[pocet_kariet_v_balicku - 1];
                hraci[aktualny_hrac].karty_v_ruke[hraci[aktualny_hrac].pocet_kariet_v_ruke] = potiahnuta_karta;
                hraci[aktualny_hrac].pocet_kariet_v_ruke++;
                pocet_kariet_v_balicku--;

                char sprava[50];
                snprintf(sprava, sizeof(sprava), "Potiahol si kartu: %c%c\n", potiahnuta_karta.farba, potiahnuta_karta.hodnota);
                posli_spravu(client_socket, sprava);

                snprintf(sprava, sizeof(sprava), "Hráč %d si potiahol kartu.\n", aktualny_hrac + 1);
                posli_vsetkym(sprava);

                prepni_na_dalsieho_hraca();
            } else {
                posli_spravu(client_socket, "Balíček je prázdny!\n");
            }

            pthread_mutex_unlock(&mutex_hra);
        }*/

        if (strcmp(buffer, "exit") == 0) {
            printf("Klient požiadal o ukončenie spojenia. Socket: %d\n", client_socket);
            close(client_socket);

            pthread_mutex_lock(&mutex_hra);
            for (int i = 0; i < pocet_hracov; i++) {
                if (hraci[i].socket == client_socket) {
                    hraci[i].je_aktivny = 0;
                    hraci[i].socket = -1;
                    pocet_hracov--;
                    printf("Počet prihlásených hráčov: %d\n", pocet_hracov);
                    break;
                }
            }
            pthread_mutex_unlock(&mutex_hra);
            break;
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Použitie: %s <port>\n", argv[0]);
        exit(1);
    }

    for (int i = 0; i < MAX_POCET_HRACOV; i++) {
        hraci[i].socket = -1;
        hraci[i].pocet_kariet_v_ruke = 0;
        hraci[i].je_aktivny = 0;
    }

    int port = atoi(argv[1]);
    int server_socket = passive_socket_init(port);
    if (server_socket < 0) {
        fprintf(stderr, "Chyba pri inicializácii servera\n");
        exit(1);
    }

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
                    hraci[i].socket = client_socket;
                    hraci[i].je_aktivny = 1;
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