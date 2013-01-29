#pragma once

#include <l4/util/atomic.h>

enum {
	mutex_init_id   = 0,
	mutex_lock_id   = 1,
	mutex_unlock_id = 2,
	pt_lock_id      = 3,
	pt_unlock_id    = 4,
	pt_max_wrappers = 5,

	lock_entry_free = 0,
	lock_unlocking  = 0xAAAAAAAA,
	lock_unowned    = 0xFFFFFFFF,

	NUM_TRAMPOLINES = 32,
	TRAMPOLINE_SIZE = 32,
	NUM_LOCKS       = 64,
	LOCK_INFO_ADDR  = 0xA000,
};

typedef struct {
  unsigned char trampolines[NUM_TRAMPOLINES * TRAMPOLINE_SIZE];
  struct {
	  volatile l4_addr_t lockdesc;            // corresponding pthread_mutex_t ptr
	  volatile l4_addr_t owner;               // lock owner
	  volatile l4_addr_t owner_epoch;         // lock holder's epoch
	  volatile l4_addr_t wait_count; // count how many threads wait to acquire this lock
	  volatile l4_addr_t acq_count;           // count how many threads acquired this lock
	  volatile l4_addr_t wake_count;          // count how many threads should be unlocked
	  volatile l4_uint32_t lock;     // internal: lock for this row
  } locks[NUM_LOCKS];
  volatile l4_umword_t replica_count; // number of replicas running a.t.m.
} lock_info;

/* Compile-time assertion: lock_info must fit into a page) */
char __lock_info_size_valid[!!(sizeof(lock_info) <= L4_PAGESIZE)-1];

lock_info* get_lock_info(void);
lock_info* get_lock_info(void) {
	return (lock_info*)LOCK_INFO_ADDR;
}


static inline void lock_li(volatile lock_info *li, unsigned idx)
{
	asm volatile ("" : : : "memory");
	while (!l4util_cmpxchg32(&li->locks[idx].lock, 0, 1)) {
		asm volatile ("ud2" : : : "eax", "memory");
	}
}

static inline void unlock_li(volatile lock_info* li, unsigned idx)
{
	asm volatile ("" : : : "eax", "memory");
	li->locks[idx].lock = 0;
	asm volatile ("" ::: "memory");
}


static char const *
__attribute__((used))
lockID_to_str(unsigned id)
{
	switch(id) {
		case mutex_init_id:   return "mutex_init";
		case mutex_lock_id:   return "mutex_lock";
		case mutex_unlock_id: return "mutex_unlock";
		case pt_lock_id:      return "__pthread_lock";
		case pt_unlock_id:    return "__pthread_unlock";
		default: return "???";
	}
}
