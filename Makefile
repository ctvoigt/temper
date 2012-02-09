all:	pcsensor

CFLAGS = -O2 -Wall -fPIC -c *.c

pcsensor:	pcsensor.c pcsensor_wrap.c
	${CC} -DUNIT_TEST -o $@ $^ -lusb

clean:
	rm -f pcsensor *.o

rules-install:			# must be superuser to do this
	cp 99-tempsensor.rules /etc/udev/rules.d