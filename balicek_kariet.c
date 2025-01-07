#include "balicek_kariet.h"
#include <stdlib.h>
#include <time.h>

Balicek vytvor_balicek() {
    Balicek balicek;
    int index = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 7; j <= 14; j++) {
            balicek.karty[index].farba = i;
            balicek.karty[index].hodnota = j;
            index++;
        }
    }
    return balicek;
}

void zamiesaj_balicek(Balicek *balicek) {
    srand(time(NULL));
    for (int i = 0; i < 32; i++) {
        int j = rand() % 32;
        Karta pomoc = balicek->karty[i];
        balicek->karty[i] = balicek->karty[j];
        balicek->karty[j] = pomoc;
    }
}

void rozdaj_karty(Balicek* balicek, Karta *hrac, int pocet_hracov) {
    for (int i = 0; i < pocet_hracov; i++) {
        for (int j = 0; j < 5; j++) {
            hrac[i * 5 + j] = balicek->karty[i * 5 + j];
        }
    }
}
