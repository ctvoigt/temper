# Makefile for temperature reader
CC=gcc
CFLAGS=-Wall
LIBS=-lusb -largtable2

all: temper

temper: temper.c pcsensor.c pcsensor.h
	$(CC) $(CFLAGS) temper.c pcsensor.c -o temper $(LIBS)

clean:
	rm temper

group:
	groupadd temper 

install:
	install -o nobody -g temper -m 2555 temper /usr/local/bin
	@echo
	@echo "Remember to install rules:"
	@echo "    make rules-install"

uninstall:
	rm -f /usr/local/bin/temper
	rm -f  /etc/udev/rules.d/99-temper.rules
	@echo
	@echo "Remember to reload udev rules:"
	@echo "udevcontrol reload_rules"
	
rules-install:	# must be superuser to do this
	cp 99-temper.rules /etc/udev/rules.d 
	udevadm trigger --action=change
	@echo
	@echo "Check if rules are correctly reloded, try:"
	@echo "udevcontrol reload_rules"
	@echo "If rules cause trouble, reboot."

