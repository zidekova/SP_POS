#ifndef STRUKTURY_H
#define STRUKTURY_H

#include <pthread.h>

#define MAX_POCET_HRACOV 4
#define ZAC_POCET_KARIET 5
#define POCET_KARIET_V_BALICKU 32

typedef struct Karta {
    char farba;
    char hodnota;
} Karta;

typedef struct Hrac {
    int socket;
    Karta karty_v_ruke[POCET_KARIET_V_BALICKU];
    int pocet_kariet_v_ruke;
    int je_aktivny;
} Hrac;

typedef struct Hra {
    Hrac hraci[MAX_POCET_HRACOV];
    int pocet_hracov;
    int hra_bezi;
    int client_sockets[MAX_POCET_HRACOV];
    pthread_mutex_t mutex_hra;
    int aktualny_hrac;
    Karta karta_na_vrchu;
    Karta kopa[POCET_KARIET_V_BALICKU]; 
    int pocet_kariet_v_kope;
    Karta volne[POCET_KARIET_V_BALICKU];
    int pocet_volnych_kariet;
} Hra;

typedef struct {
    int client_socket;
    Hra *hra;
} ThreadData;

typedef struct {
    int hra_bezi;
    int klient_sa_ukoncuje;
    int client_socket;
    pthread_t receive_thread;
    pthread_t input_thread;
    int is_host;
} KlientStav;

#endif