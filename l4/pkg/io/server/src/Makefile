PKGDIR		?= ../..
L4DIR		?= $(PKGDIR)/../..

include $(L4DIR)/mk/prog.mk

ifneq ($(filter regen_files,$(MAKECMDGOALS)),)

BISON ?= bison
RAGEL ?= ragel

SRC_YY := $(SRC_DIR)/cfg_parser.yy
CC_YY  := $(SRC_YY:%.yy=%.tab.cc) $(SRC_YY:%.yy=%.tab.hh)
# generate scanner in the source dir, to prevent a hard dependency to ragel
CC_RL  := $(SRC_DIR)/cfg_scan.cc

regen_files:: $(CC_RL) $(CC_YY)

$(CC_RL): %.cc: %.rl $(SRC_DIR)/Makefile
	@$(GEN_MESSAGE)
	$(VERBOSE)$(RAGEL) -o $@ $<
	$(VERBOSE)perl -p -i -e 's/(\#line\s+\d+\s+")\/.*\/(.+")$$/$$1$$2/' $@

$(SRC_DIR)/%.tab.hh $(SRC_DIR)/%.tab.cc: $(SRC_DIR)/%.yy $(SRC_DIR)/Makefile
	@$(GEN_MESSAGE)
	$(VERBOSE)$(BISON) $<
	$(VERBOSE)perl -p -i -e 's/(\#line\s+\d+\s+")\/.*\/(.+")$$/$$1$$2/' \
	  $(patsubst %.yy,%.tab.cc,$<) $(patsubst %.yy,%.tab.hh,$<)

.PHONY: regen_files
endif
