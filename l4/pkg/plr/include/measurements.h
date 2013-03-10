#pragma once

/*
 * measurements.h --
 *
 *     Event logging infrastructure
 *
 * (c) 2012-2013 Björn Döbel <doebel@os.inf.tu-dresden.de>,
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include <cassert>
#include <climits>
//#include <utility>

#include <l4/util/atomic.h>
#include <l4/util/rdtsc.h>
#include <l4/sys/kdebug.h>

/*
 * Namespace containing classes regarding measurements and
 * sensors etc.
 *
 * XXX: C compatibility! Don't use C++ features except inside __cplusplus parts -> I want to
 *      use this header file from C files, too
 */
#ifdef __cplusplus
namespace Measurements
{
#endif

	enum EventTypes {
		Invalid = 0,
		Syscall = 1,
		Pagefault = 2,
		Swifi = 3,
		Foo = 4,
		Trap = 5,
		Thread_start = 6,
		Thread_stop  = 7,
		Locking = 8,
		SHMLocking = 9,
	};

	struct __attribute__((packed))
	SensorHead {
		l4_uint64_t   tsc;   // TSC: stop
		l4_uint32_t   vcpu;  // vcpu ptr
		unsigned char type;  // event type
	};


	struct __attribute__((packed))
	PagefaultEvent
	{
		char rw;
		l4_addr_t address;
		l4_addr_t localbase;
		l4_addr_t remotebase;
	};

	struct __attribute__((packed))
	TrapEvent
	{
		char        start;
		l4_addr_t   trapaddr;
		l4_uint32_t trapno;
	};


	struct __attribute__((packed))
	SyscallEvent
	{
		l4_addr_t   eip;
		l4_uint32_t label;
	};


	struct __attribute__((packed))
	FooEvent
	{
		unsigned start;
	};


	struct __attribute__((packed))
	LockEvent
	{
		enum LockEvents {
			lock,
			unlock,
			mtx_lock,
			mtx_unlock
		};
		unsigned eventType;
		unsigned lockPtr;
	};


	struct __attribute__((packed))
	SHMLockEvent
	{
		unsigned lockid;
		unsigned epoch;
		unsigned owner; // current owner
		unsigned type; // 1 -> init
			           // 2 -> lock_enter
					   // 3 -> lock_exit
			           // 4 -> unlock_enter
					   // 5 -> unlock_exit
	};


	struct __attribute__((packed))
	BarnesEvent
	{
		unsigned ptr;
		unsigned num;
		unsigned type;
	};


	struct __attribute__((packed))
	GenericEvent
	{
		struct SensorHead header;
		union {
			struct PagefaultEvent pf;
			struct SyscallEvent sys;
			struct FooEvent     foo;
			struct TrapEvent    trap;
			struct LockEvent    lock;
			struct SHMLockEvent shml;
			struct BarnesEvent  barnes;
			char         pad[19];
		} data;
	};


	/*
	 * Event buffer
	 * 
	 * An event buffer holds events of the GenericEvent type. The class does not
	 * allocate memory. Instead the underlying buffer needs to be specified using
	 * the set_buffer() function.
	 * 
	 * Once the buffer is valid, users obtain an element using the next() method
	 * and fill it appropriately.
	 * 
	 * The buffer is managed as a ring buffer and may overflow, in which case the
	 * oldest elements get overwritten. The index variable is increased monotonically,
	 * so users may determine whether the buffer has already overflown by checking
	 * if index > size. If so, (index mod size) points to the oldest element.
	 *
	 * The whole buffer can be dumped using the dump() method. This will produce a UU-encoded
	 * version of the zipped buffer content.
	 */
	struct __attribute__((packed))
	EventBuf
	{
		struct GenericEvent*  buffer;
		unsigned       index;
		unsigned       size;
		unsigned       sharedTSC;
		l4_uint64_t   *timestamp;
		char _end[];

#ifdef __cplusplus

		/**
		 * Create event buffer
		 *
		 * sharableTSC -> allow the TSC value to be located in a way that we can share
		 *                this value among different address spaces (e.g., have replicas
		 *                log events themselves using this TSC). This requires the timestamp
		 *                value to be placed on a dedicated page.
		 */
		EventBuf(bool sharableTSC = false)
			: buffer(0), index(0), size(0), sharedTSC(sharableTSC ? 1 : 0)
		{
		  static_assert(sizeof(SensorHead) == 13, "Sensor head not 13 bytes large!");
		  static_assert(sizeof(GenericEvent) == 32, "GenericEvent larger than 24 bytes!");
		  //static_assert((l4_umword_t)((EventBuf const *)0)->_end < sizeof(GenericEvent), "head too large?");

		  if (!sharableTSC) {
		  	timestamp = new l4_uint64_t();
		  }

		  static unsigned char dummyBuffer[32];
		  set_buffer(dummyBuffer, 32);
		}


		~EventBuf()
		{
			enter_kdebug("~EventBuf");
			if (!sharedTSC) {
				delete timestamp;
			}
		}


		void set_buffer(unsigned char *buf, unsigned size_in_bytes)
		{
			buffer = reinterpret_cast<GenericEvent*>(buf);
			size   = size_in_bytes / sizeof(GenericEvent);
		}


		void set_tsc_buffer(l4_uint64_t *buf)
		{
			timestamp = buf;
		}


		static void launchTimerThread(l4_addr_t timerAddress, unsigned CPU);

		l4_uint64_t getTime(bool local=false)
		{
			if (local) {
				return l4_rdtsc();
			} else {
				return *timestamp;
			}
		}

		/*
		 * Get the next buffer entry.
		 *
		 * Safe against concurrent calls by using atomic increment on the
		 * counter.  Concurrent accesses may lead to events not being properly
		 * ordered, though.
		 */
		GenericEvent* next()
		{
			unsigned val = l4util_inc32_res(&index) - 1;
			val %= size;
			return &buffer[val];
		}


		unsigned oldest() const
		{
			if (index < size) {
				return 0;
			}
			else {
				return index % size;
			}
		}
#endif /* C++ */
	};

#ifdef __cplusplus
}

extern "C"
{
#endif
	void *evbuf_get_address(void);
	l4_uint64_t evbuf_get_time(void *eb, unsigned is_local);
	struct GenericEvent* evbuf_next(void *eb);
#ifdef __cplusplus
}
#endif
