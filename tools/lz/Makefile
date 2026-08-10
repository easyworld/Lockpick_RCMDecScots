NATIVE_CC ?= gcc

ifeq (, $(shell which $(NATIVE_CC) 2>/dev/null))
$(error "Native GCC is missing. Please install it first. If it's path is custom, set it with export NATIVE_CC=<path to native gcc toolchain>")
endif

.PHONY: all clean

all: lz77 check_overlap
	@echo > /dev/null

clean:
	@rm -f lz77 check_overlap

lz77: lz.c lz77.c
	@$(NATIVE_CC) -o $@ lz.c lz77.c

check_overlap: lz.c check_overlap.c
	@$(NATIVE_CC) -o $@ lz.c check_overlap.c
