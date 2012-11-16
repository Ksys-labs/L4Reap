PKGDIR		?= ../..
L4DIR		?= $(PKGDIR)/../..

TARGET		 = ex_svmtest ex_vmxtest
SYSTEMS		 = x86-l4f amd64-l4f
SRC_C_ex_svmtest = svmtest.c
SRC_C_ex_vmxtest = vmxtest.c
REQUIRES_LIBS    = libvcpu l4re_c-util libpthread

include $(L4DIR)/mk/prog.mk
