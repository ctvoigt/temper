# Makefile for temperature reader
CC=gcc
CFLAGS=-Wall

all: temper

temper: temper.c pcsensor.c pcsensor.h
	$(CC) $(CFLAGS) temper.c pcsensor.c -o temper -lusb

clean:
	rm temper

install: all
	install -o nobody -g temper -m 2555 temper /usr/local/bin
	cp 60-temper.rules /etc/udev/rules.d
	@echo
	@echo "Remember to reload udev rules:"
	@echo "    udevcontrol reload_rules"

uninstall:
	-rm -rf /usr/local/bin/temper /etc/udev/rules.d/60-temper.rules
	@echo
	@echo "Remember to reload udev rules:"
	@echo "    udevcontrol reload_rules"
