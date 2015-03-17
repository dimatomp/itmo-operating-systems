all: cat revwords

cat: libhelpers.so
	$(MAKE) -C cat

revwords: libhelpers.so
	$(MAKE) -C revwords

libhelpers.so:
	$(MAKE) -C lib
