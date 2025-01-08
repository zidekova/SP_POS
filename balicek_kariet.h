#ifndef BALICEK_KARIET_H
#define BALICEK_KARIET_H

#include "struktury.h"

void inicializuj_balicek(Karta balicek[]);
void zamiesaj_balicek(Karta balicek[], int pocet_kariet);
void rozdaj_karty_hracom(Karta balicek[], Hrac hraci[], int pocet_hracov, int zac_pocet_kariet);

#endif