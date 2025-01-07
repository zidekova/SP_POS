CC = gcc
CFLAGS = -Wall -pthread

all: server klient

server: server.c balicek_kariet.c sockets-lib/socket.c
	$(CC) $(CFLAGS) -o server server.c balicek_kariet.c sockets-lib/socket.c

klient: klient.c balicek_kariet.c sockets-lib/socket.c
	$(CC) $(CFLAGS) -o klient klient.c balicek_kariet.c sockets-lib/socket.c

clean:
	rm -f server klient
