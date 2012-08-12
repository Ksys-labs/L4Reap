# vim:set ft=make:

vpath %.c $(SRC_DIR)/../contrib/src $(SRC_DIR)/../l4
vpath %.h $(SRC_DIR)/../contrib/src

%.o: %.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS)  $<

include $(SRC_DIR)/../contrib/src/Makefile
MYCFLAGS=-I$(SRC_DIR)/../contrib/src -DLUA_USE_L4RE $(L4_DEFINES) -nostdinc $(CXXFLAGS_L4_GENERIC) $(L4_INCLUDES)
