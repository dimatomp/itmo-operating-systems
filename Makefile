TASKS=cat revwords filter bufcat simplesh filesender bipiper
.PHONY: $(TASKS)

export CFLAGS=-std=c99 -I../lib -L../lib -lhelpers -lbufio -Wall

all: $(TASKS)

libraries:
	$(MAKE) -C lib

$(TASKS): BINARIES=$@
bipiper: BINARIES=forking
bipiper: CFLAGS+=-D_POSIX_C_SOURCE=200112L

$(TASKS):
	@$(MAKE) --no-print-directory -C $@ $(BINARIES)
