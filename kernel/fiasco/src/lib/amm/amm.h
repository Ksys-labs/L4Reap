/*
 * Copyright (c) 1996, 1998 University of Utah and the Flux Group.
 * All rights reserved.
 * 
 * This file is part of the Flux OSKit.  The OSKit is free software, also known
 * as "open source;" you can redistribute it and/or modify it under the terms
 * of the GNU General Public License (GPL), version 2, as published by the Free
 * Software Foundation (FSF).  To explore alternate licensing terms, contact
 * the University of Utah at csl-dist@cs.utah.edu or +1-801-585-3271.
 * 
 * The OSKit is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GPL for more details.  You should have
 * received a copy of the GPL along with the OSKit; see the file COPYING.  If
 * not, write to the FSF, 59 Temple Place #330, Boston, MA 02111-1307, USA.
 */

/*
 * Address Map Management library.
 */
#ifndef _OSKIT_AMM_H_
#define _OSKIT_AMM_H_

#include <assert.h>
#include "types.h"

typedef Address vm_offset_t;
typedef size_t vm_size_t;

#ifdef __cplusplus
extern "C" {
#endif

/* #define STATS */

#ifdef STATS
struct amm_stats {
	unsigned lookups;
	unsigned hits;
	unsigned entriesscanned;
};
#endif

typedef struct amm_entry {
	struct amm_entry *next;		/* must be first */
	vm_offset_t	 start;
	vm_offset_t	 end;
	int		 flags;
} amm_entry_t;

typedef amm_entry_t Amm_entry;

#define AMM_MINADDR	((vm_offset_t)0)	/* min valid address */
#define AMM_MAXADDR	((vm_offset_t)~0)	/* max valid address + 1 */

/* values used for simple versions of the interface */
#define AMM_FREE	(0)
#define AMM_RESERVED	~((unsigned)(~0) >> 1)
#define AMM_ALLOCATED	(AMM_RESERVED >> 1)

typedef struct amm {
	struct amm_entry *nodes;
	struct amm_entry **hint;
	struct amm_entry *(*alloc)(struct amm *amm,
				   vm_offset_t addr,
				   vm_size_t size,
				   int flags);
	void		 (*free)(struct amm *amm,
				 struct amm_entry *entry);
	int		 (*split)(struct amm *amm,
				  struct amm_entry *entry,
				  vm_offset_t addr,
				  struct amm_entry **headp,
				  struct amm_entry **tailp);
	int		 (*join)(struct amm *amm,
				 struct amm_entry *head,
				 struct amm_entry *tail,
				 struct amm_entry **new_entry);
#ifdef STATS
	struct amm_stats stats;
#endif
} Amm;

/* amm_find attributes */
#define AMM_FORWARD	0	/* search forward for entry */
//#define AMM_BACKWARD	1	/* search backward for entry */
#define AMM_FIRSTFIT	0	/* use first fit (default) */
//#define AMM_BESTFIT	2	/* use best fit */
#define AMM_EXACT_ADDR	4	/* match address exactly */

#define amm_entry_start(e)	(e)->start
#define amm_entry_end(e)	(e)->end
#define amm_entry_size(e)	((e)->end - (e)->start)
#define amm_entry_flags(e)	(e)->flags

/* Generic interfaces */

void amm_init_gen(struct amm *amm, int flags, struct amm_entry *entry,
		  struct amm_entry *(*af)(struct amm *, vm_offset_t,
					  vm_size_t, int),
		  void (*ff)(struct amm *, struct amm_entry *),
		  int (*sf)(struct amm *, struct amm_entry *, vm_offset_t,
			    struct amm_entry **, struct amm_entry **),
		  int (*jf)(struct amm *, struct amm_entry *,
			    struct amm_entry *, struct amm_entry **));

void amm_destroy(struct amm *amm);

struct amm_entry *amm_find_addr(struct amm *amm, vm_offset_t addr);

struct amm_entry *amm_find_gen(struct amm *amm, vm_offset_t *addrp,
			       vm_size_t size, int flags, int flagmask,
			       unsigned int align_bits, vm_offset_t align_off,
			       int find_flags);

int amm_modify(struct amm *amm, vm_offset_t addr, vm_size_t size,
	       int nflags, struct amm_entry *nentry);

amm_entry_t *amm_select(struct amm *amm, vm_offset_t addr, vm_size_t size);

int
amm_iterate_gen(struct amm *amm,
		int (*ifunc)(struct amm *, struct amm_entry *, void *),
		void *arg, vm_offset_t addr, vm_size_t size,
		int flags, int flagmask);

void amm_dump(struct amm *amm);

/* Simple interfaces */

void amm_init(struct amm *amm, vm_offset_t low, vm_offset_t high);

int amm_allocate(struct amm *amm, vm_offset_t *addr, vm_size_t size, int prot);

int amm_deallocate(struct amm *amm, vm_offset_t addr, vm_size_t size);

int amm_reserve(struct amm *amm, vm_offset_t addr, vm_size_t size);

int amm_protect(struct amm *amm, vm_offset_t addr, vm_size_t size, int prot);

int amm_iterate(struct amm *amm,
		int (*ifunc)(struct amm *, struct amm_entry *, void *),
		void *arg);

int amm_split(struct amm *amm, struct amm_entry **pentry,
              struct amm_entry *entry, vm_offset_t addr,
              struct amm_entry **head, struct amm_entry **tail);

int amm_join(struct amm *amm, struct amm_entry **pentry,
             struct amm_entry *head, struct amm_entry *tail,
             struct amm_entry **new_entry);                       

struct amm_entry *amm_alloc_entry(struct amm *amm, vm_offset_t addr,
                                  vm_size_t size, int flags);     

void amm_free_entry(struct amm *amm, struct amm_entry *entry);

#ifdef __cplusplus
}
#endif

#endif	/* _OSKIT_AMM_H_ */

