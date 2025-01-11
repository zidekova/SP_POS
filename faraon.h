#ifndef FARAON_H
#define FARAON_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h> 

int start_server(int port);
int start_client(const char *server_ip, int port, int is_host);

#endif