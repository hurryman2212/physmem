SRCS = $(wildcard *.c)
SRCS := $(filter-out pget64.c, $(SRCS))
SRCS := $(filter-out pset64.c, $(SRCS))
SRCS := $(filter-out setpte.c, $(SRCS))

PROGS = $(patsubst %.c,%,$(SRCS))
PGET64-PROGS = pget8 pget16 pget32 pget64
PSET64-PROGS = pset8 pset16 pset32 pset64
SETPTE-PROGS = setpgd setp4d setpud setpmd setpte
ALL-PROGS = $(PROGS) $(PGET64-PROGS) $(PSET64-PROGS) $(SETPTE-PROGS)

CFLAGS += -std=gnu11 -D_GNU_SOURCE -Wall -Wextra -Werror -pedantic -Wno-unused-function

all: $(PROGS) pget64 pset64 setpte
%: %.c
	$(CC) $(CFLAGS) -o $@ $< -lx86linuxextra

pget64: pget64.c
	$(CC) $(CFLAGS) -D_GET8 -o pget8 $^ -lx86linuxextra
	$(CC) $(CFLAGS) -D_GET16 -o pget16 $^ -lx86linuxextra
	$(CC) $(CFLAGS) -D_GET32 -o pget32 $^ -lx86linuxextra
	$(CC) $(CFLAGS) -o $@ $^ -lx86linuxextra
pset64: pset64.c
	$(CC) $(CFLAGS) -D_SET8 -o pset8 $^ -lx86linuxextra
	$(CC) $(CFLAGS) -D_SET16 -o pset16 $^ -lx86linuxextra
	$(CC) $(CFLAGS) -D_SET32 -o pset32 $^ -lx86linuxextra
	$(CC) $(CFLAGS) -o $@ $^ -lx86linuxextra
setpte: setpte.c
	$(CC) $(CFLAGS) -D_PGD -o setpgd $^ -lm -lx86linuxextra
	$(CC) $(CFLAGS) -D_P4D -o setp4d $^ -lm -lx86linuxextra
	$(CC) $(CFLAGS) -D_PUD -o setpud $^ -lm -lx86linuxextra
	$(CC) $(CFLAGS) -D_PMD -o setpmd $^ -lm -lx86linuxextra
	$(CC) $(CFLAGS) -o $@ $^ -lm -lx86linuxextra

install: all
	cp $(ALL-PROGS) /usr/local/bin/
uninstall:
	rm -f $(ALL-PROGS:%=/usr/local/bin/%)

clean:
	rm -f $(ALL-PROGS)
