PKGDIR		?= ../../../
L4DIR		?= $(PKGDIR)/../..

TARGET		= ts_calibr
SYSTEMS		= arm-l4f
REQUIRES_LIBS	= libdisko

PRIVATE_INCDIR	+= $(PKGDIR_ABS)/../disko/include $(PKGDIR_ABS)/../libsigc++/include $(PKGDIR_ABS)/../libxml2/include

# disko defines
DEFINES		=  -D__HAVE_PNG__ -D__HAVE_DL__ -D__L4_RE__ -D__HAVE_L4_FB__ -D__HAVE_PF_ALL__ -DMMSLOGGER_STDOUT_ONLY

SRC_CC = init.cc tscalibr.cc

LDFLAGS  	=  --warn-common --no-undefined

vpath %.cpp $(SRC_DIR)/

include $(L4DIR)/mk/prog.mk
