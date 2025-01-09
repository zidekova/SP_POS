#include "balicek_kariet.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

void inicializuj_balicek(Karta balicek[]) {
    char farby[] = {'S', 'L', 'Z', 'G'}; // Piky, Srdcia, Káry, Kríže
    char hodnoty[] = {'7', '8', '9', '1', 'J', 'Q', 'K', 'A'}; // '1' pre 10

    int index = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 4; j++) {
            balicek[index].hodnota = hodnoty[i];
            balicek[index].farba = farby[j];
            index++;
        }
    }
}

void zamiesaj_balicek(Karta balicek[], int pocet_kariet) {
    srand(time(NULL));
    for (int i = 0; i < pocet_kariet; i++) {
        int j = rand() % pocet_kariet;
        Karta pomoc = balicek[i];
        balicek[i] = balicek[j];
        balicek[j] = pomoc;
    }
}

void rozdaj_karty_hracom(Karta balicek[], Hrac hraci[], int pocet_hracov, Karta *karta_na_vrchu) {
    // Kontrola, či je dostatok kariet
    if (pocet_hracov * ZAC_POCET_KARIET > POCET_KARIET_V_BALICKU) {
        printf("Chyba: Nie je dostatok kariet v balíčku pre všetkých hráčov!\n");
        return;
    }

    int pocet_kariet = POCET_KARIET_V_BALICKU - 1; // Indexy idú od 0

    // Rozdaj karty hráčom
    for (int i = 0; i < pocet_hracov; i++) {
        for (int j = 0; j < ZAC_POCET_KARIET; j++) {
            if (pocet_kariet >= 0) {  // Kontrola, aby sme neprekročili hranice poľa
                hraci[i].karty_v_ruke[j] = balicek[pocet_kariet];
                pocet_kariet--;
                hraci[i].pocet_kariet_v_ruke++;
            } else {
                printf("Chyba: Nie je dostatok kariet v balíčku!\n");
                return;  // Ukončenie funkcie, ak nie je dostatok kariet
            }
        }
    }

    // Logovanie rozdelených kariet
    printf("Rozdávam karty hráčom...\n");
    for (int i = 0; i < pocet_hracov; i++) {
        printf("Hráč %d dostáva karty: ", i);
        for (int j = 0; j < ZAC_POCET_KARIET; j++) {
            printf("%c%c ", hraci[i].karty_v_ruke[j].farba, hraci[i].karty_v_ruke[j].hodnota);
        }
        printf("\n");
    }
    printf("Karty boli úspešne rozdelené.\n");

    // Nastav kartu na vrchu balíčka a skontroluj, či sa nezhoduje s kartami hráčov
    *karta_na_vrchu = balicek[pocet_kariet];
    pocet_kariet--;

    // Kontrola, či karta na vrchu nie je rovnaká ako karty hráčov
    int je_duplikat = 1;
    while (je_duplikat) {
        je_duplikat = 0;
        for (int i = 0; i < pocet_hracov; i++) {
            for (int j = 0; j < hraci[i].pocet_kariet_v_ruke; j++) {
                if (karta_na_vrchu->farba == hraci[i].karty_v_ruke[j].farba &&
                    karta_na_vrchu->hodnota == hraci[i].karty_v_ruke[j].hodnota) {
                    je_duplikat = 1;
                    break;
                }
            }
            if (je_duplikat) break;
        }
        if (je_duplikat) {
            *karta_na_vrchu = balicek[pocet_kariet];
            pocet_kariet--;
        }
    }

    printf("Karta na vrchu: %c%c\n", karta_na_vrchu->farba, karta_na_vrchu->hodnota);
}

void potiahnut_kartu(Karta balicek[], int *pocet_kariet, Hrac *hrac) {
    if (*pocet_kariet <= 0) {
        printf("Balíček je prázdny!\n");
        return;
    }

    // Pridaj kartu hráčovi
    hrac->karty_v_ruke[hrac->pocet_kariet_v_ruke] = balicek[*pocet_kariet - 1];
    hrac->pocet_kariet_v_ruke++;
    (*pocet_kariet)--;

    printf("Hráč %d si potiahol kartu: %c%c\n", hrac->socket, balicek[*pocet_kariet].farba, balicek[*pocet_kariet].hodnota);
}