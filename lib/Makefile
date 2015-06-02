CFLAGS=-shared -Wall -D_POSIX_C_SOURCE=200809L -fPIC -std=c99

debug: CFLAGS+=-g -DDEBUG

all: libhelpers.so libbufio.so

lib%.so: %.c %.h
	$(CC) $(CFLAGS) $< -o $@

debug: all

clean:
	rm -f *.so
