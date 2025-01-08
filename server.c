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
#include "struktury.h"

Karta balicek[POCET_KARIET_V_BALICKU];
Karta kopa[POCET_KARIET_V_BALICKU];
int vrch_kopy = -1;

Hrac hraci[MAX_POCET_HRACOV];
int pocet_hracov = 0;
int aktualny_hrac = 0;
pthread_mutex_t mutex_hra = PTHREAD_MUTEX_INITIALIZER;

void posli_spravu(int socket, const char* sprava) {
    write(socket, sprava, strlen(sprava));
}

void posli_karty_hracom() {
    for (int i = 0; i < pocet_hracov; i++) {
        for (int j = 0; j < ZAC_POCET_KARIET; j++) {
            char karta[3];
            karta[0] = hraci[i].karty_v_ruke[j].hodnota;
            karta[1] = hraci[i].karty_v_ruke[j].farba;
            karta[2] = '\0';
            write(hraci[i].socket, karta, sizeof(karta));
        }
    }
}

void *handle_client(void *arg) {
    int client_socket = *(int *)arg;
    char buffer[10];

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = read(client_socket, buffer, sizeof(buffer));
        if (bytes_received <= 0) {
            // Klient sa odpojil
            printf("Klient sa odpojil\n");
            close(client_socket);
            break;
        }

        // Spracovanie ťahu
        Karta aktualna_karta;
        aktualna_karta.farba = buffer[0];
        aktualna_karta.hodnota = buffer[1];
        pthread_mutex_lock(&mutex_hra);
        if (je_platny_tah(kopa[vrch_kopy], aktualna_karta)) {
            kopa[++vrch_kopy] = aktualna_karta;
            spracuj_specialnu_kartu(aktualna_karta, hraci, pocet_hracov, &aktualny_hrac);

            // Odstránenie karty z ruky hráča
            for (int i = 0; i < hraci[aktualny_hrac].pocet_kariet_v_ruke; i++) {
                if (hraci[aktualny_hrac].karty_v_ruke[i].hodnota == aktualna_karta.hodnota &&
                    hraci[aktualny_hrac].karty_v_ruke[i].farba == aktualna_karta.farba) {
                    for (int j = i; j < hraci[aktualny_hrac].pocet_kariet_v_ruke - 1; j++) {
                        hraci[aktualny_hrac].karty_v_ruke[j] = hraci[aktualny_hrac].karty_v_ruke[j + 1];
                    }
                    hraci[aktualny_hrac].pocet_kariet_v_ruke--;
                    break;
                }
            }

            // Kontrola výhry
            if (kontrola_vyhry(hraci[aktualny_hrac])) {
                char sprava[100];
                snprintf(sprava, sizeof(sprava), "Hráč %d vyhral!\n", aktualny_hrac + 1);
                for (int i = 0; i < pocet_hracov; i++) {
                    posli_spravu(hraci[i].socket, sprava);
                }
                exit(0);
            }
        } else {
            posli_spravu(client_socket, "Neplatný ťah!\n");
        }
        pthread_mutex_unlock(&mutex_hra);
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

    inicializuj_balicek(balicek);
    zamiesaj_balicek(balicek, POCET_KARIET_V_BALICKU);

    printf("Server počúva na porte %d...\n", port);

    while (pocet_hracov < 1) { // OPRAVIT
        struct sockaddr_in client_address;
        socklen_t client_len = sizeof(client_address);
        int client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_len);
        if (client_socket < 0) {
            perror("Chyba pri prijímaní klienta");
            continue;
        }

        hraci[pocet_hracov].socket = client_socket;
        hraci[pocet_hracov].pocet_kariet_v_ruke = 0;
        hraci[pocet_hracov].je_aktivny = 1;
        pocet_hracov++;

        pthread_t thread;
        pthread_create(&thread, NULL, handle_client, &client_socket);
        pthread_detach(thread);
    }

    rozdaj_karty_hracom(balicek, hraci, pocet_hracov, ZAC_POCET_KARIET);
    posli_karty_hracom();

    // ????????
    int pocet_kariet = POCET_KARIET_V_BALICKU;
    kopa[++vrch_kopy] = balicek[--pocet_kariet];

    // Hlavná herná slučka
    /*while (1) {
        sleep(1); // Simulácia herného cyklu
    }*/

    close(server_socket);
    return 0;
}