CFLAGS = -O3 -Wall -ggdb -DSTACK_MAGIC_DEBUG
LDFLAGS =

all: coropx

coropx: $(wildcard *.c) $(wildcard *.cxx) $(wildcard *.s) Makefile
	clang \
		-x c $(wildcard *.c) \
		-x c++ $(wildcard *.cxx) \
		-x assembler $(wildcard *.s) \
		-o $@ $(CFLAGS) $(LDFLAGS)
