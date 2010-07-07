PKGDIR  ?= ../..
L4DIR   ?= $(PKGDIR)/../..

TARGET   = libgfxbitmap.a 
SRC_C    = font.c bitmap.c

FONTS    = lat0-12.psf lat0-14.psf lat0-16.psf vgafont.psf
OBJS    += $(FONTS:.psf=.o)

include $(L4DIR)/mk/lib.mk

%.o: %.psf
	@$(COMP_MESSAGE)
	$(VERBOSE)$(LD) -r --oformat $(OFORMAT) -o $@ -b binary $^

%.psf: $(SRC_DIR)/%.psf
	@ln -s $? $@
