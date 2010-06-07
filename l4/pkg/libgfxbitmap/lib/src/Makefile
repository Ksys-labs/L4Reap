PKGDIR  ?= ../..
L4DIR   ?= $(PKGDIR)/../..

TARGET   = libgfxbitmap.a 
SRC_C    = font.c bitmap.c

OBJS    += fontfile.o

include $(L4DIR)/mk/lib.mk

%.o: %.psf
	@$(COMP_MESSAGE)
	$(VERBOSE)$(LD) -r --oformat $(OFORMAT) -o $@ -b binary $^

fontfile.psf: $(SRC_DIR)/lat0-14.psf
	@ln -s $? $@
