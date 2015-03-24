all: cat revwords filter

cat: libhelpers.so
	$(MAKE) -C cat

revwords: libhelpers.so
	$(MAKE) -C revwords

filter: libhelpers.so
	$(MAKE) -C filter

libhelpers.so:
	$(MAKE) -C lib
