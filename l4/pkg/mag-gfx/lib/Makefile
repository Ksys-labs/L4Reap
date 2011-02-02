PKGDIR		?= ..
L4DIR		?= $(PKGDIR)/../..

TARGET		= libmag-gfx.a libmag-gfx.so
SRC_CC		= canvas.cc factory.cc
SRC_CC_x86-l4f   := blit-x86.cc
SRC_CC_amd64-l4f := blit.cc
SRC_CC_arm-l4f   := blit.cc
SRC_CC_ppc32-l4f := blit.cc
SRC_CC_sparc-l4f := blit.cc

include $(L4DIR)/mk/lib.mk
