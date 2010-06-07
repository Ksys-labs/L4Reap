# directories we need to know
PKGDIR  = .
L4DIR  ?= $(PKGDIR)/../..

# include subdir role
include $(L4DIR)/mk/subdir.mk

include: virt_dep

.PHONY: virt_dep
virt_dep:
	$(VERBOSE)PWD=$(PWD)/lib/uclibc $(MAKE) -C lib/uclibc include
