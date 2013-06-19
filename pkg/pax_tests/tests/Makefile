PKGDIR ?= ../
L4DIR ?= $(PKGDIR)/../../

PAX_FILES = anonmap \
	execdata \
	execstack \
	mprotstack \
	rettofunc1 \
	rettofunc1x \
	rettofunc2 \
	shlibbss
	
#	execbss \
#	execheap \
#	getamap \
#	getheap \
#	getmain \
#	getshlib \
#	getstack \
#	mprotanon \
#	mprotbss \
#	mprotdata \
#	mprotheap \
#	mprotshbss \
#	mprotshdata \
#	randamap \
#	randbody \
#	randheap1 \
#	randheap2 \
#	randmain1 \
#	randmain2 \
#	randstack1 \
#	randstack2 \
#	rettofunc2x \
#	shlibtest \
#	shlibdata \
#	shlibtest2 \
#	writetext

TARGET=$(patsubst %,pax_%,$(PAX_FILES))

LIBS = $(GCCLIB) -lpthread
MODE = shared
WARNINGS = -Wno-unused
REQUIRED_LIBS = libpthread
LDFLAGS = --no-undefined
DEFINES = -DRUNDIR=""

$(foreach target,$(PAX_FILES),$(eval SRC_C_pax_$(target)=$(target).c body.c))
SRC_C_pax_getstack=getstack.c
SRC_C_pax_getshlib=getshlib.c

vpath %.c $(PKGDIR)/contrib/paxtest-0.9.7-pre4/
include $(L4DIR)/mk/prog.mk
