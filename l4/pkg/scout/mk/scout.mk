MODE		?= shared
OBJS		+= $(addsuffix .bin.o, $(notdir $(SRC_BIN)))

REQUIRES_LIBS	+= scout

SRC_CC		= doc.cc
vpath %.txt $(SRC_DIR)

include $(L4DIR)/mk/prog.mk

$(GENERAL_D_LOC): $(SCOUTDIR)/mk/scout.mk

%.bin.o: %
	@$(GEN_MESSAGE)
	$(VERBOSE)(TARGETDIR=$$PWD && cd $(dir $<) && $(OBJCOPY) -I binary -B $(BFD_ARCH_$(ARCH)) -O $(OFORMAT) $(notdir $<) $$TARGETDIR/$@)

doc.cc: $(SRC_TXT)
	@$(GEN_MESSAGE)
	$(VERBOSE)$(GOSH) --style $(SCOUTDIR)/mk/scout.gosh $< >$@ || rm $@

