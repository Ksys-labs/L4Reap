/*
 * pagefault.cc --
 *
 *     Implementation of page fault handling.
 *
 * (c) 2011-2012 Björn Döbel <doebel@os.inf.tu-dresden.de>,
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
#include "syscalls_handler.h"

#include "observers.h"

#include <l4/sys/kdebug.h>
#include <l4/util/bitops.h>

extern l4_addr_t __libc_l4_gettime;

static int pf_write  = 0; // statistical counter
static int pf_mapped = 0; // statistical counter
static int pf_kip    = 0; // statistical counter

#define MSG() DEBUGf(Romain::Log::Memory) << "[" << i->id() << "] "
#define MSGt(t) DEBUGf(Romain::Log::Faults) << "[" << t->vcpu() << "] "

/*****************************************************************
 *                  Page Fault Handling                          *
 *****************************************************************/
DEFINE_EMPTY_STARTUP(PageFaultObserver)

void Romain::PageFaultObserver::status() const
{
	INFO() << "[ pf ] Page faults so far: mapped " << pf_mapped << " write emu "
	       << pf_write << " kip " << pf_kip;
}


/*
 * Prepare for a mapping by finding the largest possible alignment to map a
 * flexpage that contains the address at (start + offset) from master to replica.
 */
l4_umword_t Romain::PageFaultObserver::fit_alignment(Romain::Region const* local,
													 L4Re::Util::Region const * remote,
								  					 l4_umword_t offset,
								  					 Romain::App_thread *t)
{
	MSGt(t) << "offs in region: " << std::hex << offset;
	
	l4_addr_t localbase  = local->start() + offset;
	l4_addr_t remotebase = remote->start() + offset;
	MSGt(t) << std::hex << localbase  << "("  << l4util_bsf(localbase) << ") <-> "
	        << remotebase << "(" << l4util_bsf(remotebase) << ")";

	/*
	 * The maximum possible alignment is the minimum of the zero bits in both
	 * addresses.
	 */
	l4_umword_t align = std::min(l4util_bsf(localbase), l4util_bsf(remotebase));

	/*
	 * The maximum alignment might lead to a mapping that is larger than the
	 * region itself. Therefore we shrink the mapping until it fits the region.
	 */
	for (; align > 12; --align) {
		l4_umword_t mask    = (1 << align) - 1;
		l4_addr_t newlocal  = localbase - (localbase & mask);

		if ((newlocal >= local->start()) and
			(newlocal + (1<<align) - 1 <= local->end())) {
			break;
		}
	}

	return align;

	/* XXX: Use this if you want to test the single-page-mapping version. */
	//return L4_PAGESHIFT;
}

Romain::Observer::ObserverReturnVal
Romain::PageFaultObserver::notify(Romain::App_instance *i, Romain::App_thread *t,
                                  Romain::Thread_group* tg, Romain::App_model *a)
{
	if (!t->vcpu()->is_page_fault_entry())
		return Romain::Observer::Ignored;

	if (t->unhandled_pf()) {
		enter_kdebug("unhandled pf");
	}

	L4vcpu::Vcpu *vcpu = t->vcpu();

	bool write_pf = vcpu->r()->err & 0x2;
	l4_addr_t pfa = vcpu->r()->pfa;

	Measurements::GenericEvent *ev = Romain::_the_instance_manager->logbuf()->next();
	ev->header.tsc         = Romain::_the_instance_manager->logbuf()->getTime(Log::logLocalTSC);
	ev->header.vcpu        = (l4_uint32_t)t->vcpu();
	ev->header.type        = Measurements::Pagefault;
	ev->data.pf.address    = pfa;
	ev->data.pf.rw         = write_pf ? 1 : 0;
	ev->data.pf.localbase  = 0;
	ev->data.pf.remotebase = 0;

	MSGt(t) << (write_pf ? RED "write" NOCOLOR : BOLD_BLUE "read" NOCOLOR)
	      << " page fault @ 0x" << std::hex << pfa;

	Romain::Region_map::Base::Node n = a->rm()->find(pfa);
	MSGt(t) << "rm_find(" << std::hex << pfa << ") = " << n;
	if (n) {
		a->rm()->lazy_map_region(n, i->id());

		/*
		 * Lazily establish region handlers for replicas
		 */
		MSGt(t) << "[" << std::hex
		        << n->second.local_region(i->id()).start() << " - "
		        << n->second.local_region(i->id()).end() << "] ==> "
		        << "[" << n->first.start() << " - "
		        << n->first.end() << "]";
		MSGt(t) << "   DS " << std::hex <<  n->second.memory(i->id()).cap();

		if (write_pf && (always_readonly() ||
		                (n->second.writable() == Romain::Region_handler::Read_only_emulate_write))) {
			++pf_write;
			/* XXX: can we use a static object here instead? */
			AppModelAddressTranslator *aat = new AppModelAddressTranslator(a, i);
			Romain::WriteEmulator(vcpu, aat).emulate();
			delete aat;
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

			Romain::Region_handler const * rh   = &n->second;
			Romain::Region const * localregion  = &rh->local_region(i->id());
			L4Re::Util::Region const * remote   = &n->first;

			/*
			 * We try to map the largest possible flexpage to reduce the
			 * total number of mappings.
			 */
			l4_addr_t offset_in_region = l4_trunc_page(pfa - remote->start());
			l4_umword_t align = fit_alignment(localregion, remote, offset_in_region, t);
			MSGt(t) << "fitting align " << align;

			/* Calculate the map base addresses for the given alignment */
			l4_addr_t localbase = localregion->start() + offset_in_region;
			localbase  = localbase  - (localbase  & ((1 << align) - 1));

			l4_addr_t remotebase = remote->start() + offset_in_region;
			remotebase = remotebase - (remotebase & ((1 << align) - 1));

			MSGt(t) << std::hex << "map: " << localbase << " -> "
			        << remotebase << " size " << (1 << align);

			//enter_kdebug("pf");
			
			// set flags properly, only check ro(), because else we'd already ended
			// up in the emulation branch above
			unsigned pageflags           = rh->is_ro() ? L4_FPAGE_RO : L4_FPAGE_RW;

			ev->data.pf.localbase  = localregion->start() + offset_in_region;
			ev->data.pf.remotebase = remote->start() + offset_in_region;

			i->map_aligned(localbase, remotebase, align, pageflags);
		}

	} else if ((a->prog_info()->kip <= pfa) && (pfa < a->prog_info()->kip + L4_PAGESIZE)) {
	    ++pf_kip;
	    MSGt(t) << "KIP access ";
	    /* XXX ??? */
		MSGt(t) << std::hex << __libc_l4_gettime << " " <<  *(l4_addr_t*)__libc_l4_gettime;
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
	MSGt(t) << "Page faults so far: mapped " << pf_mapped << " write emu " << pf_write << " kip " << pf_kip;

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
