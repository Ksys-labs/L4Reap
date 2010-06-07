#
# $Id$
#
# Makefile for the Linux 2.4 header include package
#

PKGDIR	= ..
L4DIR	?= $(PKGDIR)/../..

INSTALL_INC_PREFIX = linux-26

SUFFIX_amd64 := x86
SUFFIX_x86   := x86
SUFFIX_arm   := arm

include $(L4DIR)/mk/include.mk

ARCH:=$(BUILD_ARCH)

# macro to establish a symlink to a dir if not already existing
MAKELINK = if [ ! $1 -ef $2 ] ; then ln -sf $3 $2 ; fi

all::
	$(VERBOSE)$(call MAKELINK, \
		$(INSTALLDIR_LOCAL)/$(INSTALL_INC_PREFIX)/asm-$(SUFFIX_$(ARCH)),\
		$(INSTALLDIR_LOCAL)/$(INSTALL_INC_PREFIX)/asm, asm-$(SUFFIX_$(ARCH)))

install::
	$(VERBOSE)$(call MAKELINK, \
		$(INSTALLDIR)/$(INSTALL_INC_PREFIX)/asm-$(SUFFIX_$(ARCH)),\
		$(INSTALLDIR)/$(INSTALL_INC_PREFIX)/asm, asm-$(SUFFIX_$(ARCH)))

