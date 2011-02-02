L4DIR ?= ../..

include $(L4DIR)/mk/Makeconf

all:: lib

lib:
	$(VERBOSE)PWD=$(call absfilename,../uclibc/lib)/libpthread \
	    $(MAKE) -C ../uclibc/lib/libpthread all
	$(VERBOSE)PWD=$(call absfilename,../uclibc/lib)/uclibc \
	  $(MAKE) -C ../uclibc/lib/uclibc PC_FILENAME= SYSTEMS="$(SYSTEMS_ABI)" all


install clean cleanall:
	$(VERBOSE)PWD=$(call absfilename,../uclibc/lib)/libpthread \
	    $(MAKE) -C ../uclibc/lib/libpthread $@
	$(VERBOSE)PWD=$(call absfilename,../uclibc/lib)/uclibc \
	  $(MAKE) -C ../uclibc/lib/uclibc PC_FILENAME= SYSTEMS="$(SYSTEMS_ABI)" $@
