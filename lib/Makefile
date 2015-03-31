CFLAGS=-shared -Wall -fPIC -std=c99

all: libhelpers libbufio

libhelpers:
	gcc $(CFLAGS) -o libhelpers.so helpers.c

libbufio:
	gcc $(CFLAGS) -o libbufio.so bufio.c

debug:
	$(MAKE) --no-print-directory CFLAGS="$(CFLAGS) -g -DDEBUG" all
