#ifndef KLIENT_H
#define KLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fcntl.h>
#include "sockets-lib/socket.h"
#include "struktury.h"

void *receive_messages(void *arg);
void *handle_user_input(void *arg);

#endif