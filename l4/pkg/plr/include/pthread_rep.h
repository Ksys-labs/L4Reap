#pragma once

#include <l4/sys/types.h>
#include <l4/util/atomic.h>

struct spinlock
{
	unsigned int value;
};


typedef struct spinlock spinlock_t;


#define fence() asm volatile ("" ::: "memory");


inline static unsigned int
fas_uint(volatile unsigned int *lock, unsigned int val)
{
	__asm__ __volatile__("xchgl %0, %1"
	     : "+m" (*lock), "+q" (val)
	     :
	     : "memory");
	return val;
}


#ifdef PT_SOLO
#include <l4/util/util.h>
static inline void
stall(void)
{
	l4_thread_yield();
}
#else
static inline void
stall(void)
{
	asm volatile ("ud2" ::: "memory");
}
#endif


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
	  volatile l4_addr_t lockdesc;    // corresponding pthread_mutex_t ptr
	  volatile l4_addr_t owner;       // lock owner
	  volatile l4_addr_t owner_epoch; // lock holder's epoch
	  volatile l4_addr_t wait_count;  // count how many threads wait to acquire this lock
	  volatile l4_addr_t acq_count;   // count how many threads acquired this lock
	  volatile l4_addr_t wake_count;  // count how many threads should be unlocked
	  volatile spinlock_t lock;       // internal: lock for this row
  } locks[NUM_LOCKS];
  volatile l4_umword_t replica_count; // number of replicas running a.t.m.
} lock_info;

/* Compile-time assertion: lock_info must fit into a page) */
static char __lock_info_size_valid[!!(sizeof(lock_info) <= L4_PAGESIZE)-1];

static inline lock_info* get_lock_info(void)
{
	static lock_info* __lip_address = (lock_info*)LOCK_INFO_ADDR;
#ifdef PT_SOLO
	static lock_info lip;
	static int init = 0;
	if (!init) {
		init = 1;
		lip.replica_count = 1;
		memset(&lip.trampolines, 0, NUM_TRAMPOLINES * TRAMPOLINE_SIZE);
		lip.locks[0].owner    = 0xDEADBEEF;
		lip.locks[0].lockdesc = 0xFAFAFAFA;
	}
	__lip_address = &lip;
#endif
	return __lip_address;
}


static inline void lock_li(volatile lock_info *li, unsigned idx)
{
	volatile spinlock_t *lock = &li->locks[idx].lock;
	while (fas_uint(&lock->value, 1) == 1) {
		stall();
	}
	fence();
}

static inline void unlock_li(volatile lock_info* li, unsigned idx)
{
	fence();
	li->locks[idx].lock.value = 0;
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
