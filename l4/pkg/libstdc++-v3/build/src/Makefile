PKGDIR ?= 	../..
L4DIR  ?= 	$(PKGDIR)/../..

TARGET         ?= libstdc++.a libstdc++.so
CONTRIB_INCDIR  = libstdc++-v3
PC_FILENAME    ?= libstdc++
STDCXX_PKG_DIR ?= $(PKGDIR_ABS)
REQUIRES_LIBS  ?= libsupc++

STDCXX_SRC_SUBDIRS := src config/io config/locale/generic

SRC_CC := \
  allocator-inst.cc \
  bitmap_allocator.cc \
  codecvt.cc \
  compatibility.cc \
  complex_io.cc \
  concept-inst.cc \
  ctype.cc \
  debug.cc \
  debug_list.cc \
  ext-inst.cc \
  fstream-inst.cc \
  functexcept.cc \
  globals_io.cc \
  ios.cc \
  ios_failure.cc \
  ios_init.cc \
  ios-inst.cc \
  ios_locale.cc \
  iostream-inst.cc \
  istream.cc \
  istream-inst.cc \
  limits.cc \
  locale.cc \
  locale_facets.cc \
  locale_init.cc \
  locale-inst.cc \
  localename.cc \
  misc-inst.cc \
  mt_allocator.cc \
  ostream-inst.cc \
  pool_allocator.cc \
  sstream-inst.cc \
  stdexcept.cc \
  streambuf.cc \
  streambuf-inst.cc \
  string-inst.cc \
  tree.cc \
  valarray-inst.cc \
  wlocale-inst.cc \
  wstring-inst.cc \
  basic_file_stdio.cc \
  c_locale.cc \
  ctype_members.cc \
  time_members.cc \
  monetary_members.cc \
  messages_members.cc \
  numeric_members.cc \
  codecvt_members.cc \
  collate_members.cc \


include $(L4DIR)/mk/lib.mk
include $(STDCXX_PKG_DIR)/contrib.inc
include $(STDCXX_PKG_DIR)/build/src/Makefile-$(STDCXX_CONTRIB_VERSION)

.general.d: $(STDCXX_PKG_DIR)/build/src/Makefile-$(STDCXX_CONTRIB_VERSION)

vpath %.cc $(OBJ_DIR)
vpath %.cc $(STDCXX_CONTRIB_DIR)/src
vpath %.cc $(STDCXX_CONTRIB_DIR)/config/io
vpath %.cc $(STDCXX_CONTRIB_DIR)/config/locale/generic

