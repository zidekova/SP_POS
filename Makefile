CC = gcc
CFLAGS = -Wall -pthread

all: server klient

server: server.c sockets-lib/socket.c
	$(CC) $(CFLAGS) -o server server.c sockets-lib/socket.c

klient: klient.c sockets-lib/socket.c
	$(CC) $(CFLAGS) -o klient klient.c sockets-lib/socket.c

clean:
	rm -f server klient

