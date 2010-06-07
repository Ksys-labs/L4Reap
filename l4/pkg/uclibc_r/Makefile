L4DIR ?= ../..

include $(L4DIR)/mk/Makeconf

all:: lib

doc:

include:

idl:

examples:

lib:
	$(VERBOSE)PWD=$(call absfilename,../uclibc/lib)/libpthread \
	    $(MAKE) -C ../uclibc/lib/libpthread all
	$(VERBOSE)PWD=$(call absfilename,../uclibc/lib)/uclibc \
	  $(MAKE) -C ../uclibc/lib/uclibc PC_FILENAME= SYSTEMS="x86-l4f arm-l4f amd64-l4f ppc32-l4f" all


install clean cleanall:
	$(VERBOSE)PWD=$(call absfilename,../uclibc/lib)/libpthread \
	    $(MAKE) -C ../uclibc/lib/libpthread $@
	$(VERBOSE)PWD=$(call absfilename,../uclibc/lib)/uclibc \
	  $(MAKE) -C ../uclibc/lib/uclibc PC_FILENAME= SYSTEMS="x86-l4f arm-l4f amd64-l4f ppc32-l4f" $@
