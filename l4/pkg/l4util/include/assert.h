/*****************************************************************************/
/**
 * \file
 * \brief   Some useful assert-style macros.
 *
 * \date    09/2009
 * \author  Bjoern Doebel <doebel@tudos.org>
 *
 * (c) 2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

/*****************************************************************************/
#pragma once

#if !DEBUG

#define DO_NOTHING              do {} while (0)
#define ASSERT_VALID(c)         DO_NOTHING
#define ASSERT_EQUAL(a,b)       DO_NOTHING
#define ASSERT_NOT_EQUAL(a,b)   DO_NOTHING
#define ASSERT_LOWER_EQ(a,b)    DO_NOTHING
#define ASSERT_GREATER_EQ(a,b)  DO_NOTHING
#define ASSERT_BETWEEN(a,b,c)   DO_NOTHING
#define ASSERT_IPC_OK(i)        DO_NOTHING
#define ASSERT_OK(e)            DO_NOTHING
#define ASSERT_NOT_NULL(p)      DO_NOTHING
#ifndef assert
#define assert(cond)            DO_NOTHING
#endif

#else // DEBUG 

#include <stdio.h>
#include <assert.h>
#include <l4/sys/kdebug.h>

#define ASSERT_VALID(cap) \
	do { \
		if (l4_is_invalid_cap(cap)) { \
			printf("%s: Cap invalid.\n", __func__); \
			assert(l4_is_invalid_cap(cap)); \
		} \
	} while (0)


#define ASSERT_EQUAL(a, b) \
	do { \
		if ((a) != (b)) { \
			printf("%s: "#a" (%lx) != "#b" (%lx)\n", __func__, (unsigned long)(a), (unsigned long)(b)); \
			assert((a)==(b)); \
		} \
	} while (0)


#define ASSERT_NOT_EQUAL(a, b) \
	do { \
		if ((a) == (b)) { \
			printf("%s: "#a" (%lx) == "#b" (%lx)\n", __func__, (unsigned long)(a), (unsigned long)(b)); \
			assert((a)!=(b)); \
		} \
	} while (0)


#define ASSERT_LOWER_EQ(val, max) \
	do { \
		if ((val) > (max)) { \
			printf("%s: "#val" (%lx) > "#max" (%lx)\n", __func__, (unsigned long)(val), (unsigned long)(max)); \
			assert((val)<=(max)); \
		} \
	} while (0)


#define ASSERT_GREATER_EQ(val, min) \
	do { \
		if ((val) < (min)) { \
			printf("%s: "#val" (%lx) < "#min" (%lx)\n", __func__, (unsigned long)(val), (unsigned long)(min)); \
			assert((val)>=(min)); \
		} \
	} while (0)


#define ASSERT_BETWEEN(val, min, max) \
	ASSERT_LOWER_EQ((val), (max)); \
    ASSERT_GREATER_EQ((val), (min));


#define ASSERT_IPC_OK(msgtag) \
	do { \
		if (l4_ipc_error((msgtag), l4_utcb())) { \
			printf("%s: IPC Error: %lx\n", __func__, l4_ipc_error(msgtag)); \
			assert(!l4_ipc_error(msgtag)); \
		} \
	} while (0)

#define ASSERT_OK(val)         ASSERT_EQUAL((val), 0)
#define ASSERT_NOT_NULL(ptr)   ASSERT_NOT_EQUAL((ptr), NULL)

#endif // DEBUG
