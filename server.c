#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include "sockets-lib/socket.h"
#include "balicek_kariet.h"

#define SOCKET_PC 9999
#define MAX_POCET_HRACOV 4
#define MAX_POCET_KARIET 5

typedef struct Hrac {
    int sock;
    int player_id;
} Hrac;

Karta ruka[MAX_POCET_HRACOV][5]; 
Karta posledna_karta; 
int aktualny_hrac = 0;

bool game_finished = false;  

void *handle_client(void *arg) {
    Hrac *hrac = (Hrac *)arg;

    Karta ruka_hraca[MAX_POCET_KARIET];
    memcpy(ruka_hraca, ruka[hrac->player_id], sizeof(Karta) * MAX_POCET_KARIET);

    while (1) {
        if (aktualny_hrac == hrac->player_id) {
            printf("Hrac %d je na rade.\n", hrac->player_id + 1);
            printf("Posledna karta: %d-%d\n", posledna_karta.farba, posledna_karta.hodnota);

            // Čaká na výber karty
            Karta nova_karta;
            read(hrac->sock, &nova_karta, sizeof(Karta));
            
            posledna_karta = nova_karta;
            printf("Hrac %d zahral kartu %d-%d.\n", hrac->player_id + 1, nova_karta.farba, nova_karta.hodnota);
            write(hrac->sock, &nova_karta, sizeof(Karta));  // Pošle potvrdenie
            
            // Hráč nemôže položiť kartu, musí ťahať
            // nova_karta = ťahaj_z_balíčka();
            // printf("Hrac %d ťahá kartu: %d-%d\n", hrac->player_id + 1, nova_karta.farba, nova_karta.hodnota);
            // write(hrac->sock, &nova_karta, sizeof(Karta));  // Pošle kartu
        
            // Prejde na ďalšieho hráča
            aktualny_hrac = (aktualny_hrac + 1) % MAX_POCET_HRACOV;
        }
        sleep(1);
    }

    free(arg);
    close(hrac->sock);
    return NULL;
}

void start_game_simulation() {
    vytvor_balicek();
    zamiesaj_balicek();
    posledna_karta = balicek[MAX_POCET_HRACOV - 1];  // Začneme poslednou kartou balíčka

    printf("Server zacal hru...\n");
}

int main() {
    printf("Spustil sa server.\n");

    int fd_passive = passive_socket_init(SOCKET_PC);
    if (fd_passive < 0) {
        fprintf(stderr, "Chyba pri inicializacii servera.\n");
        return 1;
    }

    start_game_simulation();

    pthread_t threads[MAX_POCET_HRACOV];
    for (int i = 0; i < MAX_POCET_HRACOV; i++) {
        int fd_active = passive_socket_wait_for_client(fd_passive);
        if (fd_active < 0) {
            perror("Chyba pri cakani na klienta");
            continue;
        }

        printf("Novy klient sa pripojil.\n");

        Hrac *hrac = malloc(sizeof(Hrac));
        hrac->sock = fd_active;
        hrac->player_id = i;
        memcpy(ruka[i], &balicek[i * MAX_POCET_KARIET], sizeof(Karta) * MAX_POCET_KARIET); 

        if (pthread_create(&threads[i], NULL, handle_client, hrac) != 0) {
            perror("Chyba pri vytvarani vlakna");
            free(hrac);
            continue;
        }
    }

    for (int i = 0; i < MAX_POCET_HRACOV; i++) {
        pthread_join(threads[i], NULL);
    }

    active_socket_destroy(fd_passive);
    return 0;
}