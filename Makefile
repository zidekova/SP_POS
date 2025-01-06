CC = gcc
CFLAGS = -Wall -pthread

all: server klient

server: server.c
	$(CC) $(CFLAGS) -o server server.c

klient: klient.c
	$(CC) $(CFLAGS) -o klient klient.c

clean:
	rm -f server klient

