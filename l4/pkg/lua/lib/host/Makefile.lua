# vim:set ft=make:

B:=$(SRC_DIR)/../contrib/src

vpath %.c $(B)
vpath %.h $(B)

%.o: %.c
	$(CC) -c $(CFLAGS) $<

include $(B)/Makefile
