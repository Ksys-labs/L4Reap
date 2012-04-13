PKGDIR		?= ../../../
L4DIR		?= $(PKGDIR)/../..

PC_FILENAME	= libssl
TARGET		= libssl.so libssl.a

LIBS		= $(GCCLIB)
REQUIRES_LIBS	= libcrypto


PRIVATE_INCDIR	= $(PKGDIR)/include $(PKGDIR)/lib/contrib/ $(PKGDIR)/lib/contrib/crypto
DEFINES		= -DDSO_DLFCN -DHAVE_DLFCN_H
WARNINGS	= -Wno-unused

LDFLAGS		=  --warn-common --no-undefined
#--no-allow-shlib-undefined

SRC_C =  ssl/s2_meth.c \
	ssl/s2_srvr.c \
	ssl/s2_clnt.c \
	ssl/s2_lib.c \
	ssl/s2_enc.c \
	ssl/s2_pkt.c \
	ssl/s3_meth.c \
	ssl/s3_srvr.c \
	ssl/s3_clnt.c \
	ssl/s3_lib.c \
	ssl/s3_enc.c \
	ssl/s3_pkt.c \
	ssl/s3_both.c \
	ssl/s23_meth.c \
	ssl/s23_srvr.c \
	ssl/s23_clnt.c \
	ssl/s23_lib.c \
	ssl/s23_pkt.c \
	ssl/t1_meth.c \
	ssl/t1_srvr.c \
	ssl/t1_clnt.c \
	ssl/t1_lib.c \
	ssl/t1_enc.c \
	ssl/d1_meth.c \
	ssl/d1_srvr.c \
	ssl/d1_clnt.c \
	ssl/d1_lib.c \
	ssl/d1_pkt.c \
	ssl/d1_both.c \
	ssl/d1_enc.c \
	ssl/ssl_lib.c \
	ssl/ssl_err2.c \
	ssl/ssl_cert.c \
	ssl/ssl_sess.c \
	ssl/ssl_ciph.c \
	ssl/ssl_stat.c \
	ssl/ssl_rsa.c \
	ssl/ssl_asn1.c \
	ssl/ssl_txt.c \
	ssl/ssl_algs.c \
	ssl/bio_ssl.c \
	ssl/ssl_err.c \
	ssl/kssl.c \
	ssl/t1_reneg.c

vpath %.c $(SRC_DIR)/../../contrib/

include $(L4DIR)/mk/lib.mk
