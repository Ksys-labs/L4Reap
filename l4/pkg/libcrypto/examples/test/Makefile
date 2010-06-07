PKGDIR		?= ../../
L4DIR		?= $(PKGDIR)/../..

TARGET		= crypto_test_ref crypto_test_linux crypto_test_oslo \
		  crypto_test_openssl
REQUIRES_LIBS   = log
SRC_C		= test.c
SYSTEMS		= x86-l4f

# list additional library paths and libraries here
LIBS		= -lcrypto_pad -lcrypto_rsaref2 -lcrypto_modes
LIBS_crypto_test_ref     = -lcrypto_sha1_ref -lcrypto_aes_linux_586.o
LIBS_crypto_test_linux   = -lcrypto_sha1_linux -lcrypto_aes_linux.o
LIBS_crypto_test_oslo    = -lcrypto_sha1_oslo -lcrypto_aes_linux.o
LIBS_crypto_test_openssl = -lcrypto_sha1_openssl -lcrypto_aes_openssl

include $(L4DIR)/mk/prog.mk
