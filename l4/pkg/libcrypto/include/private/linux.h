/*
 * \brief   Private header for Linux compatibility layer.
 * \date    2005-10-15
 * \author  Carsten Weinhold <weinhold@os.inf.tu-dresden.de>
 */
/*
 * Copyright (C) 2005-2007  Carsten Weinhold <weinhold@os.inf.tu-dresden.de>
 * Technische Universitaet Dresden, Operating Systems Research Group
 *
 * This file is part of the libcrypto package, which is distributed under
 * the  terms  of the  GNU General Public Licence 2.  Please see the
 * COPYING file for details.
 */

#ifndef __TFS_LINUX_H
#define __TFS_LINUX_H

/* ************************************************************** */

#include <stdint.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#ifdef __LIBCRYPTO_INTERNAL__

#define __u32 u32
#define __le32 u32

/* ************************************************************** */

#define __initdata
#define __init

/* ************************************************************** */

/* FIXME: this is only for little-endian systems */

#define le32_to_cpu(x) ((u32)(x))
#define cpu_to_le32(x) ((u32)(x))

/* borrowed from <linux/byteorder/swab.h> from Linux 2.6.11 */
#define ___swab32(x) \
({ \
        __u32 __x = (x); \
        ((__u32)( \
                (((__u32)(__x) & (__u32)0x000000ffUL) << 24) | \
                (((__u32)(__x) & (__u32)0x0000ff00UL) <<  8) | \
                (((__u32)(__x) & (__u32)0x00ff0000UL) >>  8) | \
                (((__u32)(__x) & (__u32)0xff000000UL) >> 24) )); \
})

#define be32_to_cpu(x) ___swab32(x)

/* ************************************************************** */

//#define asmlinkage extern "C" 
#define asmlinkage

/* ************************************************************** */

/* taken from <linux/crypto.h> from Linux 2.6.11 */
#define CRYPTO_TFM_RES_BAD_KEY_LEN      0x00200000

/* ************************************************************** */

/* FIXME: is there a more appropriate value? */
#ifndef EINVAL
#define EINVAL -1
#endif

#endif /* __LIBCRYPTO_INTERNAL__ */

#endif /* __TFS_LINUX_H */

