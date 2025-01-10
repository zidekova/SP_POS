#ifndef PRAVIDLA_H
#define PRAVIDLA_H

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "struktury.h"
#include "balicek_kariet.h"

int je_platny_tah(Karta vrchna_karta, Karta aktualna_karta);
void je_sedmicka(Karta aktualna_karta, Hra *hra);
void je_eso(Karta aktualna_karta, Hra *hra);
void je_menic(Karta aktualna_karta, Hra *hra);
int kontrola_vyhry(Hrac *hrac);

#endif