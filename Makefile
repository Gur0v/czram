CC = clang
CFLAGS = -Wall -Wextra -O3 -march=native -pipe -fstack-protector-strong -fPIE -pie -flto=thin
PREFIX ?= /usr

all: czram

czram: czram.c
	$(CC) $(CFLAGS) -o czram czram.c

install: czram
	cp czram $(DESTDIR)$(PREFIX)/bin/

clean:
	rm -f czram

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/czram

.PHONY: all install clean uninstall

