#include "balicek_kariet.h"
#include <stdlib.h>
#include <time.h>

// Definícia balíčka
Karta balicek[32];

void vytvor_balicek() {
    int index = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 7; j <= 14; j++) {
            balicek[index].farba = i;
            balicek[index].hodnota = j;
            index++;
        }
    }
}

void zamiesaj_balicek() {
    srand(time(NULL));
    for (int i = 0; i < 32; i++) {
        int random_index = rand() % 32;
        Karta pomoc = balicek[i];
        balicek[i] = balicek[random_index];
        balicek[random_index] = pomoc;
    }
}

void rozdaj_karty(Karta *hrac, int pocet_hracov) {
    for (int i = 0; i < pocet_hracov; i++) {
        for (int j = 0; j < 5; j++) {
            hrac[i * 5 + j] = balicek[i * 5 + j];
        }
    }
}
