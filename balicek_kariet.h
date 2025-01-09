#ifndef BALICEK_KARIET_H
#define BALICEK_KARIET_H

#include "struktury.h"

void inicializuj_balicek(Karta balicek[]);
void zamiesaj_balicek(Karta balicek[], int pocet_kariet);
void rozdaj_karty_hracom(Karta balicek[], Hrac hraci[], int pocet_hracov, Karta *karta_na_vrchu);
void potiahnut_kartu(Karta balicek[], int *pocet_kariet, Hrac *hrac);

#endif