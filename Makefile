CC = gcc

CFLAGS = -g -Wall -Wextra -pthread -I.

TARGETS = faraon server klient

SRCS = faraon.c server.c klient.c balicek_kariet.c pravidla.c sockets-lib/socket.c

all: $(TARGETS)

faraon: faraon.c
	$(CC) $(CFLAGS) -o $@ faraon.c

server: server.c balicek_kariet.c pravidla.c sockets-lib/socket.c
	$(CC) $(CFLAGS) -o $@ server.c balicek_kariet.c pravidla.c sockets-lib/socket.c

klient: klient.c balicek_kariet.c sockets-lib/socket.c
	$(CC) $(CFLAGS) -o $@ klient.c balicek_kariet.c sockets-lib/socket.c

clean:
	rm -f $(TARGETS)

help:
	@echo "Použitie:"
	@echo "  make        - Skompiluje všetky cieľové programy (faraon, server, klient)"
	@echo "  make clean  - Odstráni skompilované súbory"
	@echo "  make help   - Zobrazí túto nápovedu"

.PHONY: all clean help