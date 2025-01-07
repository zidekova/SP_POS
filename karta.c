#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef enum {
    SRDCE = '♥', 
    LIST = '♠', 
    GULA = '♦',
    ZALUD = '♣'
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

char get_farba(Karta karta) {
    return karta.farba;
}

char get_hodnota(Karta karta) {
    return karta.hodnota;
}

char* get_karta(Karta karta) {
    char *karta_str[3];
    karta_str[0] = karta.farba;
    karta_str[1] = karta.hodnota;
    karta_str[2] = '\0';
    return karta_str;
}

