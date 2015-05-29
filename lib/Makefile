CFLAGS=-shared -Wall -D_POSIX_C_SOURCE=200809L -fPIC -std=c99

all: libhelpers libbufio

libhelpers:
	gcc $(CFLAGS) -o libhelpers.so helpers.c

libbufio:
	gcc $(CFLAGS) -o libbufio.so bufio.c

debug:
	$(MAKE) --no-print-directory CFLAGS="$(CFLAGS) -g -DDEBUG" all
