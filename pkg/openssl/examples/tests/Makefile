PKGDIR		?= ../../
L4DIR		?= $(PKGDIR)/../..

TARGET		= bntest ectest ecdsatest ecdhtest ideatest md2test md4test md5test hmactest \
		wp_test rc2test rc4test rc5test destest shatest sha1test sha256t sha512t mdc2test \
		rmdtest randtest dhtest enginetest bftest casttest ssltest exptest dsatest \
		rsa_test evp_test igetest jpaketest asn1test dummytest

LIBS		= $(GCCLIB)
MODE            = shared
REQUIRES_LIBS	= libcrypto libssl


PRIVATE_INCDIR	= $(PKGDIR)/include $(PKGDIR)/lib/contrib/ $(PKGDIR)/lib/contrib/crypto
DEFINES		= -DDSO_DLFCN -DHAVE_DLFCN_H
WARNINGS	= -Wno-unused

LDFLAGS		=  --warn-common --no-undefined
#--no-allow-shlib-undefined

SRC_C_bntest = test/bntest.c
SRC_C_ectest = test/ectest.c
SRC_C_ecdsatest = test/ecdsatest.c
SRC_C_ecdhtest = test/ecdhtest.c
SRC_C_ideatest = test/ideatest.c
SRC_C_md2test = test/md2test.c
SRC_C_md4test = test/md4test.c
SRC_C_md5test = test/md5test.c
SRC_C_hmactest = test/hmactest.c
SRC_C_wp_test = test/wp_test.c
SRC_C_rc2test = test/rc2test.c
SRC_C_rc4test = test/rc4test.c
SRC_C_rc5test = test/rc5test.c
SRC_C_destest = test/destest.c
SRC_C_shatest = test/shatest.c
SRC_C_sha1test = test/sha1test.c
SRC_C_sha256t = test/sha256t.c
SRC_C_sha512t = test/sha512t.c
SRC_C_mdc2test = test/mdc2test.c
SRC_C_rmdtest = test/rmdtest.c
SRC_C_randtest = test/randtest.c
SRC_C_dhtest = test/dhtest.c
SRC_C_enginetest = test/enginetest.c
SRC_C_bftest = test/bftest.c
SRC_C_casttest = test/casttest.c
SRC_C_ssltest = test/ssltest.c
SRC_C_exptest = test/exptest.c
SRC_C_dsatest = test/dsatest.c
SRC_C_rsa_test = test/rsa_test.c
SRC_C_evp_test = test/evp_test.c
SRC_C_igetest = test/igetest.c
SRC_C_jpaketest = test/jpaketest.c
SRC_C_asn1test = test/asn1test.c
SRC_C_dummytest = test/dummytest.c

vpath %.c $(SRC_DIR)/../../lib/contrib/

include $(L4DIR)/mk/prog.mk
