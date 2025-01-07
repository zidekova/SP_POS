CC = gcc
CFLAGS = -Wall -pthread

all: main server klient

main: main.c
	$(CC) $(CFLAGS) -o main main.c

server: server.c balicek_kariet.c sockets-lib/socket.c
	$(CC) $(CFLAGS) -o server server.c balicek_kariet.c sockets-lib/socket.c

klient: klient.c balicek_kariet.c sockets-lib/socket.c
	$(CC) $(CFLAGS) -o klient klient.c balicek_kariet.c sockets-lib/socket.c

clean:
	rm -f main server klient