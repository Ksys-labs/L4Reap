# vim:set ft=make:

vpath %.c $(SRC_DIR)/../contrib/src $(SRC_DIR)/../l4
vpath %.h $(SRC_DIR)/../contrib/src

%.o: %.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS)  $<

%: %.o
	$(CC) $(LDFLAGS) $< $(LOADLIBES) $(LDLIBS)
	

include $(SRC_DIR)/../contrib/src/Makefile


MYCFLAGS=-I$(SRC_DIR)/../contrib/src $(L4_DEFINES) -nostdinc -fno-stack-protector $(L4_INCLUDES)
