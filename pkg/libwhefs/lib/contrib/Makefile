#!/usr/bin/make -f
include config.make

.PHONY: src
app: src
app src: 
	$(MAKE) -C $@ $(MAKECMDGOALS)

clean: clean-app clean-src
distclean: distclean-app distclean-src

all: src app

.PHONY:
AMAL_GEN := $(TOP_SRCDIR)/createAmalgamation.sh
AMAL_CFLAGS := -std=c99 -c -pedantic -Wall -Wimplicit-function-declaration
amal amalgamation: 
	bash $(AMAL_GEN)
	@which gcc &>/dev/null || exit 0; \
		echo "Compiling amalgamation..."; \
		gcc $(AMAL_CFLAGS) whefs_amalgamation.c || exit $$?
	@rm -f whefs_amalgamation.o; echo "It compiles!"
