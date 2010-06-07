/*
 * \brief   Private header Linux AES functions.
 * \date    2006-07-26
 * \author  Carsten Weinhold <weinhold@os.inf.tu-dresden.de>
 */
/*
 * Copyright (C) 2006-2007  Carsten Weinhold <weinhold@os.inf.tu-dresden.de>
 * Technische Universitaet Dresden, Operating Systems Research Group
 *
 * This file is part of the libcrypto package, which is distributed under
 * the  terms  of the  GNU General Public Licence 2.  Please see the
 * COPYING file for details.
 */

#ifndef __CRYPTO_AES_LINUX_H
#define __CRYPTO_AES_LINUX_H

#include "linux.h"

/* AES C implementation */

struct aes_c_ctx {
	int key_length;
	u32 E[60];
	u32 D[60];
};

/* AES i586 ASM implementation */

#define AES_MIN_KEY_SIZE	16
#define AES_MAX_KEY_SIZE	32
#define AES_BLOCK_SIZE		16
#define AES_KS_LENGTH		4 * AES_BLOCK_SIZE
#define RC_LENGTH		29

struct aes_586_ctx {
	u32 ekey[AES_KS_LENGTH];
	u32 rounds;
	u32 dkey[AES_KS_LENGTH];
};
#endif /* __CRYPTO_AES_LINUX_H */

