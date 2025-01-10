#include "pravidla.h"

// Teraz môžeme pristupovať k balicek, pretože je deklarovaný v struktury.h

int je_platny_tah(Karta vrchna_karta, Karta aktualna_karta) {
    return (aktualna_karta.farba == vrchna_karta.farba || aktualna_karta.hodnota == vrchna_karta.hodnota || aktualna_karta.hodnota == 'Q');
}

void je_sedmicka(Karta aktualna_karta, Hra *hra) {
    if (aktualna_karta.hodnota == '7') {
        // 7: Nasledujúci hráč potiahne 3 karty
        for (int i = 0; i < 3; i++) {
            if (hra->pocet_volnych_kariet > 0) {
                potiahnut_kartu(hra, &hra->hraci[(hra->aktualny_hrac + 1) % hra->pocet_hracov]);   
            }
        }
        printf("Hráč %d potiahol 3 karty.\n", hra->aktualny_hrac + 2);
    }
}

void je_eso(Karta aktualna_karta, Hra *hra) {
    if (aktualna_karta.hodnota == 'A') {
        // Eso: Nasledujúci hráč vynechá ťah
        hra->aktualny_hrac = (hra->aktualny_hrac + 1) % hra->pocet_hracov;
        printf("Hráč %d vynecháva ťah.\n", (hra->aktualny_hrac + 1));
    }
}

void je_menic(Karta aktualna_karta, Hra *hra) {
    if (aktualna_karta.hodnota == 'Q') {
        // Menic: Farba sa vyberá náhodne
        char farby[] = {'S', 'L', 'Z', 'G'}; 
        int index = rand() % 4; 
        char vybrana_farba = farby[index]; 

        // Nastav novú farbu na vrchu balíčka
        hra->karta_na_vrchu.farba = vybrana_farba;
        hra->karta_na_vrchu.hodnota = ' ';

        printf("Farba novej karty je: %c\n", vybrana_farba);
    }
}

int kontrola_vyhry(Hrac *hrac) {
    return (hrac->pocet_kariet_v_ruke == 0);
}