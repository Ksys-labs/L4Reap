PKGDIR          ?= ../..
L4DIR           ?= $(PKGDIR)/../..

TARGET           = l4dope
REQUIRES_LIBS    = input libio
L4_MULTITHREADED = y

include $(L4DIR)/mk/Makeconf

PRIVATE_LIBDIR   = $(PKGDIR_OBJ)/server/common/OBJ-$(SYSTEM) \
                   $(PKGDIR_OBJ)/server/gfx/OBJ-$(SYSTEM)

MODE_USE_C       = y
EXTRA_LIBS      += -ldope_common -ldope_gfx

PRIVATE_INCDIR   = $(PKGDIR_ABS)/server/include

#CFLAGS_donsched-client.c = -ffunction-sections

SRC_C            = messenger.c vscr_server.c  \
                   startup.c   input.c        \
                   thread.c    timer.c        sharedmem.c

#SRC_C            += donsched.c

SRC_CC           = scrdrv_gen.cc server.cc

vpath messenger.c $(PKGDIR_ABS)/server/common

#CLIENTIDL        = dopeapp.idl donsched.idl
#SERVERIDL        = dope.idl vscr.idl donsched.idl
#IDL_PKGDIR       = $(PKGDIR_OBJ) $(PKGDIR_OBJ)/server

include $(L4DIR)/mk/prog.mk

ifdef USE_RT_MON
LIBS_ALL        += -lrt_mon_client
CFLAGS          += -DUSE_RT_MON=1
endif
