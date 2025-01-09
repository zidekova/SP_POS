#ifndef PRAVIDLA_H
#define PRAVIDLA_H

#include "struktury.h"

int je_platny_tah(Karta vrchna_karta, Karta aktualna_karta);
void spracuj_specialnu_kartu(Karta aktualna_karta, Hrac hraci[], int pocet_hracov, int *aktualny_hrac);
int kontrola_vyhry(Hrac hrac);

#endif