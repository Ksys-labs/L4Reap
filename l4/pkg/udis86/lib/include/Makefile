PKGDIR    = ../..
L4DIR    ?= $(PKGDIR)/../..

CONTRIB_HEADERS = y
PKGNAME         = libudis86
INCSRC_DIR      = $(OBJ_DIR)
TARGET          = udis86.h \
                  libudis86/itab.h \
                  libudis86/extern.h \
		  libudis86/types.h

CONTRIB_PATH    = $(SRC_DIR)/../contrib

include $(L4DIR)/mk/include.mk

headers:: $(OBJ_DIR)/itab.h $(OBJ_DIR)/itab.c
	$(VERBOSE)mkdir -p $(OBJ_DIR)/libudis86
	$(VERBOSE)cp $(SRC_DIR)/../contrib/udis86.h $(OBJ_DIR)
	$(VERBOSE)cp $(SRC_DIR)/../contrib/libudis86/extern.h $(OBJ_DIR)/libudis86
	$(VERBOSE)cp $(SRC_DIR)/../contrib/libudis86/types.h $(OBJ_DIR)/libudis86
	$(VERBOSE)cp $(OBJ_DIR)/itab.h $(OBJ_DIR)/libudis86

$(OBJ_DIR)/itab.h $(OBJ_DIR)/itab.c:
	cd $(OBJ_DIR) && PYTHONPATH=$(CONTRIB_PATH)/scripts python \
	  $(CONTRIB_PATH)/libudis86/itab.py \
	  $(CONTRIB_PATH)/docs/x86/optable.xml
