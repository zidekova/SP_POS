#include "balicek_kariet.h"
#include <stdlib.h>
#include <time.h>

void inicializuj_balicek(Karta balicek[]) {
    char farby[] = {'S', 'L', 'Z', 'G'}; // Piky, Srdcia, Káry, Kríže
    char hodnoty[] = {'7', '8', '9', '1', 'J', 'Q', 'K', 'A'}; // '1' pre 10

    int index = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 4; j++) {
            balicek[index].hodnota = hodnoty[i];
            balicek[index].farba = farby[j];
            index++;
        }
    }
}

void zamiesaj_balicek(Karta balicek[], int pocet_kariet) {
    srand(time(NULL));
    for (int i = 0; i < pocet_kariet; i++) {
        int j = rand() % pocet_kariet;
        Karta pomoc = balicek[i];
        balicek[i] = balicek[j];
        balicek[j] = pomoc;
    }
}

void rozdaj_karty_hracom(Karta balicek[], Hrac hraci[], int pocet_hracov, int zac_pocet_kariet) {
    int pocet_kariet = POCET_KARIET_V_BALICKU - 1; // Indexy idú od 0
    for (int i = 0; i < pocet_hracov; i++) {
        for (int j = 0; j < zac_pocet_kariet; j++) {
            hraci[i].karty_v_ruke[j] = balicek[pocet_kariet];
            pocet_kariet--;
            hraci[i].pocet_kariet_v_ruke++;
        }
    }
}