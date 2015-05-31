all: cat revwords filter bufcat simplesh

cat: libhelpers.so
	$(MAKE) -C cat

revwords: libhelpers.so
	$(MAKE) -C revwords

filter: libhelpers.so
	$(MAKE) -C filter

bufcat: libbufio.so
	$(MAKE) -C bufcat

simplesh: libhelpers.so libbufio.so
	$(MAKE) -C simplesh

libhelpers.so:
	$(MAKE) -C lib libhelpers

libbufio.so:
	$(MAKE) -C lib libbufio
