#ifndef BALICEK_KARIET_H
#define BALICEK_KARIET_H

// Struktúra a definície
typedef enum {
    SRDCE = 'S', 
    LIST = 'L', 
    GULA = 'G',
    ZALUD = 'Z'
} Farba;

typedef enum {
    SEDEM = 7, 
    OSEM = 8, 
    DEVAT = 9, 
    DESAT = 10, 
    DOLNIK = 'J', 
    HORNIK = 'Q', 
    KRAL = 'K', 
    ESO = 'A'
} Hodnota;

typedef struct Karta {
    Farba farba;
    Hodnota hodnota;
} Karta;

typedef struct {
    Karta karty[32];
} Balicek;

// Deklarácie funkcií
Balicek vytvor_balicek();
void zamiesaj_balicek();
void rozdaj_karty(Balicek* balicek, Karta *hrac, int pocet_hracov);

#endif
