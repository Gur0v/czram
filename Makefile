CC = clang
CFLAGS = -Wall -Wextra -O3 -march=native -pipe -fstack-protector-strong -fPIE -pie -flto=thin
PREFIX ?= /usr

all: czram

czram: czram.c
	$(CC) $(CFLAGS) -o czram czram.c

install: czram
	install -Dm755 czram $(DESTDIR)$(PREFIX)/bin/czram
	install -Dm644 dinit.d/czram $(DESTDIR)/etc/dinit.d/czram
	install -Dm644 conf $(DESTDIR)/etc/default/czram

clean:
	rm -f czram

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/czram
	rm -f $(DESTDIR)/etc/dinit.d/czram
	rm -f $(DESTDIR)/etc/default/czram

.PHONY: all install clean uninstall

