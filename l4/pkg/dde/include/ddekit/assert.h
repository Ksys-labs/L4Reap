#pragma once

#include <l4/dde/ddekit/printf.h>
#include <l4/dde/ddekit/panic.h>

/** \file ddekit/assert.h */

/** Assert that an expression is true and panic if not. 
 * \ingroup DDEKit_util
 */
#define Assert(expr)	do 									\
	{														\
		if (!(expr)) {										\
			ddekit_print("\033[31;1mDDE: Assertion failed: "#expr"\033[0m\n");	\
			ddekit_printf("  File: %s:%d\n",__FILE__,__LINE__); 		\
			ddekit_printf("  Function: %s()\n", __FUNCTION__);	\
			ddekit_panic("Assertion failed.");				\
		}} while (0);

#define assert Assert
