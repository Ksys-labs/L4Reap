/*
 * \brief   Private header Linux SHA1 functions.
 * \date    2006-07-26
 * \author  Carsten Weinhold <weinhold@os.inf.tu-dresden.de>
 */
/*
 * Copyright (C) 2006  Carsten Weinhold <weinhold@os.inf.tu-dresden.de>
 * Technische Universitaet Dresden, Operating Systems Research Group
 *
 * This file is part of the libcrypto package, which is distributed under
 * the  terms  of the  GNU General Public Licence 2.  Please see the
 * COPYING file for details.
 */

#ifndef __CRYPTO_SHA1_LINUX_H
#define __CRYPTO_SHA1_LINUX_H

#include "linux.h"

struct sha1_ctx {
        u64 count;
        u32 state[5];
        u8 buffer[64];
};

#endif /* __CRYPTO_SHA1_LINUX_H */

