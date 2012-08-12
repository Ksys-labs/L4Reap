/*
 * romain_pagefault.cc --
 *
 *     Implementation of page fault handling.
 *
 * (c) 2011 Björn Döbel <doebel@os.inf.tu-dresden.de>,
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include "../constants.h"
#include "../log"
#include "../exceptions"
#include "../memory"
#include "../emulation"
#include "../app_loading"
#include "../locking.h"
#include "../configuration"

#include "observers.h"

#include <l4/sys/kdebug.h>
#include <l4/util/bitops.h>

extern l4_addr_t __libc_l4_gettime;

static int pf_write  = 0; // statistical counter
static int pf_mapped = 0; // statistical counter
static int pf_kip    = 0; // statistical counter
#define MSG() DEBUGf(Romain::Log::Memory) << "[" << i->id() << "] "

/*****************************************************************
 *                  Page Fault Handling                          *
 *****************************************************************/
DEFINE_EMPTY_STARTUP(PageFaultObserver)

void Romain::PageFaultObserver::status() const
{
	INFO() << "Page faults so far: mapped " << pf_mapped << " write emu "
	       << pf_write << " kip " << pf_kip;
}

Romain::Observer::ObserverReturnVal
Romain::PageFaultObserver::notify(Romain::App_instance *i, Romain::App_thread *t, Romain::App_model *a)
{
	if (!t->vcpu()->is_page_fault_entry())
		return Romain::Observer::Ignored;

	if (t->unhandled_pf()) {
		enter_kdebug("unhandled pf");
	}

	L4vcpu::Vcpu *vcpu = t->vcpu();

	bool write_pf = vcpu->r()->err & 0x2;
	l4_addr_t pfa = vcpu->r()->pfa;

	MSG() << (write_pf ? "\033[31mwrite\033[0m" : "\033[34;1mread\033[0m")
	      << " page fault @ 0x" << std::hex << pfa;

	Romain::Region_map::Base::Node n = a->rm()->find(pfa);
	MSG() << "rm_find(" << std::hex << pfa << ") = " << n;
	if (n) {
		a->rm()->lazy_map_region(n, i->id());

		/*
		 * Lazily establish region handlers for replicas
		 */
		MSG() << "[" << std::hex
		        << n->second.local_region(i->id()).start() << " - "
		        << n->second.local_region(i->id()).end() << "] ==> "
		        << "[" << n->first.start() << " - "
		        << n->first.end() << "]";
		MSG() << "   DS " << std::hex <<  n->second.memory(i->id()).cap();

		if (write_pf && (always_readonly() ||
		                (n->second.writable() == Romain::Region_handler::Read_only_emulate_write))) {
			++pf_write;
			Romain::WriteEmulator(vcpu, Romain::AppModelAddressTranslator(a, i)).emulate();
			// writes are emulated, no need to map here at all
			// -> XXX actually, we could also map() here, because if the client
			//        writes to memory, it's most probably going to read this
			//        data at a later point in time, too

			/*
			 * For emulated operations single-stepping won't work, because
			 * the real instruction is never executed in the target AS. Therefore,
			 * we need to inject a "virtual" INT1 into the manager here.
			 */
			if (t->vcpu()->r()->flags & TrapFlag)
				t->set_pending_trap(1);
		} else {
			++pf_mapped;

			l4_addr_t offset_in_region = l4_trunc_page(pfa - n->first.start());
			MSG() << "offs in region: " << std::hex << offset_in_region;
			
			// set flags properly, only check ro(), because else we'd already ended
			// up in the emulation branch above
			unsigned pageflags           = n->second.is_ro() ? L4_FPAGE_RO : L4_FPAGE_RW;
			unsigned map_size            = L4_PAGESIZE;
			unsigned size_left_in_region = n->first.end() - (n->first.start() + offset_in_region);

#if 0
#define MAX_MAP_SHIFT 0
			for (unsigned x = 1; x < MAX_MAP_SHIFT; ++x) {
				if (size_left_in_region >= (1 << (L4_PAGESHIFT + x)))
					map_size *= 2;
			}
#undef MAX_MAP_SHIFT
#endif

			i->map(n->second.local_region(i->id()).start() + offset_in_region, // local addr
				   n->first.start() + offset_in_region,                        // remote addr
				   pageflags, map_size);
		}

	} else if ((a->prog_info()->kip <= pfa) && (pfa < a->prog_info()->kip + L4_PAGESIZE)) {
	    ++pf_kip;
	    MSG() << "KIP access ";
	    /* XXX ??? */
		MSG() << std::hex << __libc_l4_gettime << " " <<  *(l4_addr_t*)__libc_l4_gettime;
	    t->set_unhandled_pf();
	} else {
		ERROR() << "Unhandled page fault @ address 0x" << std::hex << pfa
		        << " PC @ 0x" << vcpu->r()->ip;
		t->set_unhandled_pf();
	}

	/*
	 * XXX: For now, tell the manager to let all instances perform their fault handling
	 *      independently. In fact, we could however also handle all faults upon first
	 *      occurrence and return Replicatable here.
	 */
	MSG() << "Page faults so far: mapped " << pf_mapped << " write emu " << pf_write << " kip " << pf_kip;
	return Romain::Observer::Finished;
}


Romain::PageFaultObserver::PageFaultObserver()
{
	char const *ro = ConfigStringValue("general:page_fault_handling"); 

	if (ro && (strcmp(ro, "ro") == 0)) {
		_readonly = true;
	} else {
		_readonly = false;
	}
}
