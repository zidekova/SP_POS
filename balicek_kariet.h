#ifndef BALICEK_KARIET_H
#define BALICEK_KARIET_H

#include "struktury.h"

void inicializuj_balicek(Karta volne[], int *pocet_volnych_kariet);
void zamiesaj_balicek(Karta volne[], int pocet_volnych_kariet);
void rozdaj_karty_hracom(Hra *hra);
void potiahnut_kartu(Hra *hra, Hrac *hrac);
void odhod_kartu_do_kopy(Hra *hra, Hrac *hrac, int index_karty);
void presun_karty_z_kopy_do_volnych(Hra *hra);

#endif