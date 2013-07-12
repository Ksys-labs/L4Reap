PKGDIR ?= ../
L4DIR ?= $(PKGDIR)/../../

PAX_FILES = anonmap \
	execbss \
	execdata \
	execheap \
	execstack \
	mprotanon \
	mprotbss \
	mprotdata \
	mprotheap \
	mprotstack \
	rettofunc1 \
	rettofunc2

# these are needed by rand* tests. TODO: use L4Re loader
#PAX_STANDALONE = getamap \
#	getheap \
#	getmain \
#	getshlib \
#	getstack

#	mprotdata
#	mprotshbss
#	randamap \
#	randbody \
#	randheap1 \
#	randheap2 \
#	randmain1 \
#	randmain2 \
#	randstack1 \
#	randstack2 \
#	rettofunc2x \
#	shlibbss \
#	shlibdata \
#	shlibtest \
#	shlibtest2 \
#	writetext

TARGET=$(patsubst %,pax_%,$(PAX_FILES) $(PAX_STANDALONE))

LIBS = $(GCCLIB) -lpthread
MODE = shared
WARNINGS = -Wno-unused
REQUIRED_LIBS = libpthread
LDFLAGS = --no-undefined
DEFINES = -DRUNDIR=""

$(foreach target,$(PAX_FILES),$(eval SRC_C_pax_$(target)=$(target).c body.c))
#$(foreach target,$(PAX_STANDALONE),$(eval SRC_C_pax_$(target)=$(target).c))

vpath %.c $(PKGDIR)/contrib/paxtest-0.9.7-pre4/
include $(L4DIR)/mk/prog.mk
