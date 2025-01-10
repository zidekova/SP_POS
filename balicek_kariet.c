#include "balicek_kariet.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

void inicializuj_balicek(Karta volne[], int *pocet_volnych_kariet) {
    char farby[] = {'S', 'L', 'Z', 'G'}; // Piky, Srdcia, Káry, Kríže
    char hodnoty[] = {'7', '8', '9', '1', 'J', 'Q', 'K', 'A'}; // '1' pre 10

    int index = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 4; j++) {
            volne[index].hodnota = hodnoty[i];
            volne[index].farba = farby[j];
            index++;
        }
    }
    *pocet_volnych_kariet = POCET_KARIET_V_BALICKU; // Nastav počet kariet vo volne
}

void zamiesaj_balicek(Karta volne[], int pocet_volnych_kariet) {
    for (int i = 0; i < pocet_volnych_kariet; i++) {
        int j = rand() % pocet_volnych_kariet;
        Karta pomoc = volne[i];
        volne[i] = volne[j];
        volne[j] = pomoc;
    }
}

void rozdaj_karty_hracom(Hra *hra) {
    // Kontrola, či je dostatok kariet
    if (hra->pocet_hracov * ZAC_POCET_KARIET > hra->pocet_volnych_kariet) {
        printf("Chyba: Nie je dostatok kariet v balíčku pre všetkých hráčov!\n");
        return;
    }

    int pocet_kariet = hra->pocet_volnych_kariet - 1; // Indexy idú od 0

    // Rozdaj karty hráčom
    for (int i = 0; i < hra->pocet_hracov; i++) {
        for (int j = 0; j < ZAC_POCET_KARIET; j++) {
            if (pocet_kariet >= 0) {  // Kontrola, aby sme neprekročili hranice poľa
                hra->hraci[i].karty_v_ruke[j] = hra->volne[pocet_kariet];
                pocet_kariet--;
                hra->hraci[i].pocet_kariet_v_ruke++;
            } else {
                printf("Chyba: Nie je dostatok kariet v balíčku!\n");
                return;  // Ukončenie funkcie, ak nie je dostatok kariet
            }
        }
    }

    // Nastav kartu na vrchu balíčka a skontroluj, či sa nezhoduje s kartami hráčov
    hra->karta_na_vrchu = hra->volne[pocet_kariet];
    pocet_kariet--;

    // Kontrola, či karta na vrchu nie je rovnaká ako karty hráčov
    int je_duplikat = 1;
    while (je_duplikat) {
        je_duplikat = 0;
        for (int i = 0; i < hra->pocet_hracov; i++) {
            for (int j = 0; j < hra->hraci[i].pocet_kariet_v_ruke; j++) {
                if (hra->karta_na_vrchu.farba == hra->hraci[i].karty_v_ruke[j].farba &&
                    hra->karta_na_vrchu.hodnota == hra->hraci[i].karty_v_ruke[j].hodnota) {
                    je_duplikat = 1;
                    break;
                }
            }
            if (je_duplikat) break;
        }
        if (je_duplikat) {
            hra->karta_na_vrchu = hra->volne[pocet_kariet];
            pocet_kariet--;
        }
    }

    // Aktualizuj počet kariet vo volne
    hra->pocet_volnych_kariet = pocet_kariet + 1;
}

void potiahnut_kartu(Hra *hra, Hrac *hrac) {
    if (hra->pocet_volnych_kariet <= 0) {
        printf("Balíček je prázdny!\n");
        return;
    }

    // Pridaj kartu hráčovi
    hrac->karty_v_ruke[hrac->pocet_kariet_v_ruke] = hra->volne[hra->pocet_volnych_kariet - 1];
    hrac->pocet_kariet_v_ruke++;
    hra->pocet_volnych_kariet--;
}

void odhod_kartu_do_kopy(Hra *hra, Hrac *hrac, int index_karty) {
    if (index_karty < 0 || index_karty >= hrac->pocet_kariet_v_ruke) {
        printf("Neplatný index karty!\n");
        return;
    }

    // Pridaj kartu do kopy
    hra->kopa[hra->pocet_kariet_v_kope] = hrac->karty_v_ruke[index_karty];
    hra->pocet_kariet_v_kope++;

    // Odstráň kartu z ruky hráča
    for (int i = index_karty; i < hrac->pocet_kariet_v_ruke - 1; i++) {
        hrac->karty_v_ruke[i] = hrac->karty_v_ruke[i + 1];
    }
    hrac->pocet_kariet_v_ruke--;

    printf("Hráč %d odhodil kartu: %c%c\n", hra->aktualny_hrac + 1, hra->kopa[hra->pocet_kariet_v_kope - 1].farba, hra->kopa[hra->pocet_kariet_v_kope - 1].hodnota);
}

void presun_karty_z_kopy_do_volnych(Hra *hra) {
    // Kontrola, či je kopa prázdna
    if (hra->pocet_kariet_v_kope <= 1) {
        printf("Kopa neobsahuje dostatok kariet na presun.\n");
        return;
    }

    // Presuň karty z kopy do volných kariet (okrem karty na vrchu kopy)
    for (int i = 0; i < hra->pocet_kariet_v_kope - 1; i++) {
        hra->volne[hra->pocet_volnych_kariet] = hra->kopa[i];
        hra->pocet_volnych_kariet++;
    }

    // Posledná karta v kope (karta na vrchu) ostáva v kope
    hra->kopa[0] = hra->kopa[hra->pocet_kariet_v_kope - 1];
    hra->pocet_kariet_v_kope = 1; // V kope zostáva len karta na vrchu

    printf("Karty z kopy boli presunuté do volných kariet (okrem karty na vrchu).\n");
}