#include "pravidla.h"
#include "struktury.h"
#include <stdio.h>

// Teraz môžeme pristupovať k balicek, pretože je deklarovaný v struktury.h

int je_platny_tah(Karta vrchna_karta, Karta aktualna_karta) {
    return (aktualna_karta.farba == vrchna_karta.farba || aktualna_karta.hodnota == vrchna_karta.hodnota);
}

void spracuj_specialnu_kartu(Karta aktualna_karta, Hrac hraci[], int pocet_hracov, int *aktualny_hrac) {
    if (aktualna_karta.hodnota == 'A') {
        // Eso: Nasledujúci hráč vynechá ťah
        *aktualny_hrac = (*aktualny_hrac + 2) % pocet_hracov;
    } else if (aktualna_karta.hodnota == '7') {
        // 7: Nasledujúci hráč potiahne 3 karty
        int dalsi_hrac = (*aktualny_hrac + 1) % pocet_hracov;
        for (int i = 0; i < 3; i++) {
            if (hraci[dalsi_hrac].pocet_kariet_v_ruke < ZAC_POCET_KARIET) {
                int index_karty = POCET_KARIET_V_BALICKU - 1;
                hraci[dalsi_hrac].karty_v_ruke[hraci[dalsi_hrac].pocet_kariet_v_ruke++] = balicek[index_karty];
            }
        }
    } else if (aktualna_karta.hodnota == 'Q') {
        // Menič: Hráč si vyberá farbu
        printf("Vyber farbu (S, L, Z, G): ");
        char vybrana_farba;
        scanf(" %c", &vybrana_farba);
        aktualna_karta.farba = vybrana_farba;
    }
}

int kontrola_vyhry(Hrac hrac) {
    return (hrac.pocet_kariet_v_ruke == 0);
}