# Kompilátor
CC = gcc

# Prepínače pre kompiláciu
CFLAGS = -g -Wall -Wextra -pthread -I.

# Ciele
TARGETS = main server klient

# Zdrojové súbory
SRCS = main.c server.c klient.c balicek_kariet.c pravidla.c sockets-lib/socket.c

# Objektové súbory
OBJS = $(SRCS:.c=.o)

# Hlavné pravidlo
all: $(TARGETS)

# Kompilácia hlavného programu
main: main.o
	$(CC) $(CFLAGS) -o $@ $^

# Kompilácia servera
server: server.o balicek_kariet.o pravidla.o sockets-lib/socket.o
	$(CC) $(CFLAGS) -o $@ $^

# Kompilácia klienta
klient: klient.o balicek_kariet.o sockets-lib/socket.o
	$(CC) $(CFLAGS) -o $@ $^

# Pravidlo pre kompiláciu .c súborov do .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Vyčistenie projektu
clean:
	rm -f $(TARGETS) $(OBJS)

# Pomocné pravidlo pre zobrazenie nápovedy
help:
	@echo "Použitie:"
	@echo "  make        - Skompiluje všetky cieľové programy (main, server, klient)"
	@echo "  make clean  - Odstráni skompilované súbory"
	@echo "  make help   - Zobrazí túto nápovedu"

.PHONY: all clean help