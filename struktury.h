#ifndef STRUKTURY_H
#define STRUKTURY_H

#define MAX_POCET_HRACOV 4
#define ZAC_POCET_KARIET 5
#define POCET_KARIET_V_BALICKU 32

typedef struct Karta {
    char farba;
    char hodnota;
} Karta;

typedef struct Hrac {
    int socket;
    Karta karty_v_ruke[ZAC_POCET_KARIET];
    int pocet_kariet_v_ruke;
    int je_aktivny;
} Hrac;

extern Karta balicek[POCET_KARIET_V_BALICKU];

#endif