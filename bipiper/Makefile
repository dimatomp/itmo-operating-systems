CFLAGS=-D_POSIX_C_SOURCE=200112L -L../lib -lhelpers -lbufio -I../lib -std=c99

all:
	gcc $(CFLAGS) -o bipiper bipiper.c

debug:
	gcc $(CFLAGS) -g -o bipiper bipiper.c
