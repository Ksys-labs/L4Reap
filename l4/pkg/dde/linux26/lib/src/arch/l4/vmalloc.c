/******************************************************************************
 * Bjoern Doebel <doebel@tudos.org>                                           *
 *                                                                            *
 * (c) 2005 - 2007 Technische Universitaet Dresden                            *
 * This file is part of DROPS, which is distributed under the terms of the    *
 * GNU General Public License 2. Please see the COPYING file for details.     *
 ******************************************************************************/

/*
 * \brief    vmalloc implementation
 * \author   Bjoern Doebel
 * \date     2007-07-30
 */

/* Linux */
#include <linux/vmalloc.h>

/* DDEKit */
#include <l4/dde/ddekit/memory.h>
#include <l4/dde/ddekit/lock.h>

void *vmalloc(unsigned long size)
{
	return ddekit_simple_malloc(size);
}

void vfree(const void *addr)
{
	ddekit_simple_free((void*)addr);
}
