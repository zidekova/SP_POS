#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include "sockets-lib/socket.h"
#include "balicek_kariet.h"
#include "pravidla.h"

void posli_spravu(int socket, const char* sprava);
void posli_vsetkym(Hra *hra, const char* sprava);
void posli_info_o_karte_na_vrchu(Hra *hra);
void posli_info_o_poradi(Hra *hra);
void vypis_hracove_karty(Hrac* hrac);
void odstran_hraca_z_pola(Hra *hra, int client_socket);
void *handle_client(void *arg);

#endif
