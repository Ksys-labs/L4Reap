PKGDIR        ?= ../..
L4DIR         ?= $(PKGDIR)/../..

SYSTEMS        = x86-l4f arm-l4f

REQUIRES_LIBS  = l4util l4re_c l4re_c-util shmc shmc_ringbuf ankh ankh_lwip \
				 libc_be_fs_noop libc_support_misc libc_be_sig

MODE_USE_C = y
L4_MULTITHREADED = y

TARGET         = wget_clnt

PRIVATE_INCDIR += $(PKGDIR)/lib/lwip/contrib/src/include \
				  $(PKGDIR)/lib/lwip/contrib/src/include/ipv4 \
				  $(PKGDIR)/lib/lwip \
				  $(SRC_DIR)/contrib/src

CFLAGS += -DHAVE_CONFIG_H -Wno-sign-compare -Wno-unused

vpath %.c $(SRC_DIR)/contrib/src

SRC_C = \
		connect.c \
		convert.c \
		cookies.c \
		ftp.c \
		ftp-basic.c \
		ftp-ls.c \
		hash.c \
		html-parse.c \
		html-url.c \
		host.c \
		http.c \
		init.c \
		l4re_helpers.c \
		log.c \
		main.c \
		netrc.c \
		progress.c \
		ptimer.c \
		recur.c \
		res.c \
		retr.c \
		safe-ctype.c \
		spider.c \
		url.c \
		utils.c \
		version.c \
		xmalloc.c

include $(L4DIR)/mk/prog.mk
