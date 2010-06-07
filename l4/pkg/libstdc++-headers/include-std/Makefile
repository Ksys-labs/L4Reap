PKGDIR ?= 	..
L4DIR  ?= 	$(PKGDIR)/../..

STDCXX_PKG_DIR ?= $(PKGDIR_ABS)/../libstdc++-v3

CONTRIB_HEADERS = y
PKGNAME         = libstdc++-v3
INCSRC_DIR      = $(STDCXX_CONTRIB_DIR)/include
TARGET          = $(shell (cd $(INCSRC_DIR) && find . -type f -not -name 'Make*' -print))

LIBSUPC_H := exception_defines.h exception new typeinfo cxxabi.h cxxabi-forced.h

ALLOCATOR_H        := config/allocator/new_allocator_base.h
CLOCALE_H          := config/locale/generic/c_locale.h
ATOMIC_WORD_H      := atomic_word.h
CPU_DEFINES_H      := cpu_defines.h
TIME_MEMBERS_H     := config/locale/generic/time_members.h
MESSAGES_MEMBERS_H := config/locale/generic/messages_members.h
BASIC_FILE_H       := config/io/basic_file_stdio.h
CSTDIO_H           := config/io/c_io_stdio.h
CMATH_TCC          := include/c_std/cmath.tcc

CTYPE_HEADERS := config/os/generic/*
# FIX BID:

include $(L4DIR)/mk/include.mk
include $(STDCXX_PKG_DIR)/contrib.inc

ifeq ("$(STDCXX_0x)", "y")
  LIBSUPC_H += initializer_list exception_ptr.h nested_exception.h
endif

STDCXX_CSTD_HEADERS := \
  cassert cctype cerrno cfloat ciso646 climits clocale cmath csetjmp csignal \
  cstdarg cstddef cstdio cstdlib cstring ctime cwchar cwctype cstdint


STDCXX_CSTD_HEADERS += $(patsubst std_%.h,%,$(notdir $(wildcard $(INCSRC_DIR)/std/*)))

STDCXX_STD_DIRS     := std c_std c_global


INSTALL_INCDIR_X = $(INSTALLDIR_LOCAL)/$(INSTALL_INC_PREFIX)
SRCDIR_X         = $(STDCXX_CONTRIB_DIR)


install_header = $(LN) -sf $(SRCDIR_X)/$(strip $(1)) $(INSTALL_INCDIR_X)/$(strip $(2))

install_config_header = $(VERBOSE)$(call install_header, $(1), bits/$(strip $(2)))

cpu_header = $(word 1,$(wildcard $(SRCDIR_X)/$(CPU_H_arch)/$(strip $(1)) $(SRCDIR_X)/$(CPU_H_generic)/$(strip $(1))))

install_cpu_header = $(VERBOSE)$(if $(call cpu_header,$(1)),$(LN) -sf $(call cpu_header,$(1)) $(INSTALL_INCDIR_X)/bits/$(strip $(1)))

LIBSUPC_HI = $(addprefix $(INSTALL_INCDIR_X)/, $(LIBSUPC_H))
STDCXX_CSTD_HEADERS_ABS = $(addprefix $(INSTALL_INCDIR_X)/, $(STDCXX_CSTD_HEADERS))


std_header_list = $(strip $(1)) std_$(strip $(1)).h

choose_std_header = $(word 1, $(wildcard $(foreach DIR,$(STDCXX_STD_DIRS),$(addprefix $(INCSRC_DIR)/$(DIR)/, $(call std_header_list, $(1))))))

$(STDCXX_CSTD_HEADERS_ABS): %: $(INSTALL_INCDIR_X)
	@$(INSTALL_LINK_MESSAGE)
	$(VERBOSE)$(LN) -sf $(call choose_std_header,$(notdir $@)) $@

$(LIBSUPC_HI): %: $(INSTALL_INCDIR_X)
	@$(INSTALL_LINK_MESSAGE)
	$(VERBOSE)$(call install_header, libsupc++/$(notdir $@), $(notdir $@))

$(INSTALL_INCDIR_X):
	@$(INSTALL_LINK_MESSAGE) $(INSTALL_INCDIR_X)
	$(VERBOSE)$(INSTALL) -d $(INSTALL_INCDIR_X)

all:: $(INSTALL_INCDIR_X) $(LIBSUPC_HI) $(STDCXX_CSTD_HEADERS_ABS)
	@$(INSTALL_LINK_MESSAGE)
	$(call install_config_header, $(ALLOCATOR_H),     c++allocator.h)
	$(call install_config_header, $(CLOCALE_H),       c++locale.h)
	$(call install_cpu_header, atomic_word.h)
	$(call install_cpu_header, cpu_defines.h)
	$(call install_cpu_header, cxxabi_tweaks.h)
	$(call install_config_header, $(CTYPE_HEADERS),   .)
	$(call install_config_header, $(TIME_MEMBERS_H),  time_members.h)
	$(call install_config_header, $(MESSAGES_MEMBERS_H),  messages_members.h)
	$(call install_config_header, $(BASIC_FILE_H), basic_file.h)
	$(call install_config_header, $(CSTDIO_H), c++io.h)
	$(call install_config_header, $(CMATH_TCC), cmath.tcc)
