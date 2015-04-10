# Makefile for temperature reader
CC=gcc
CFLAGS=-Wall

all: temper

temper: temper.c pcsensor.c pcsensor.h
	$(CC) $(CFLAGS) temper.c pcsensor.c -o temper -lusb

clean:
	rm temper

group:
	groupadd temper 

install:
	install temper /usr/local/bin

uninstall:
	rm -f /usr/local/bin/temper
	
rules-install:	# must be superuser to do this
	cp 99-temper.rules /etc/udev/rules.d 
	udevadm trigger --action=change 
