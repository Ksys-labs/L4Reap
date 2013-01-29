/*
 * memory.cc --
 *
 * Memory management implementation
 *
 * (c) 2011-2013 Björn Döbel <doebel@os.inf.tu-dresden.de>,
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "memory"
#include "app_loading"
#include "locking.h"
#include <l4/sys/kdebug.h>

#define MSG() DEBUGf(Romain::Log::Memory)

int Romain::Region_ops::map(Region_handler const *h, l4_addr_t addr,
                            L4Re::Util::Region const &r, bool writable,
                            l4_umword_t *result)
{
	(void)h;
	(void)addr;
	(void)r;
	(void)writable;
	(void)result;
	enter_kdebug("ops::map");
	return -1;
}

void Romain::Region_ops::unmap(Region_handler const *h, l4_addr_t vaddr,
                               l4_addr_t offs, unsigned long size)
{
	(void)h;
	(void)vaddr;
	(void)offs;
	(void)size;
	enter_kdebug("ops::unmap");
}


void Romain::Region_ops::free(Region_handler const *h, l4_addr_t start, unsigned long size)
{
	(void)h;
	(void)start;
	(void)size;
	enter_kdebug("ops::free");
}


void Romain::Region_ops::take(Region_handler const* h)
{
	(void)h;
	enter_kdebug("ops::take");
}


void Romain::Region_ops::release(Region_handler const* h)
{
	(void)h;
	enter_kdebug("ops::release");
}


Romain::Region_map::Region_map()
	: Base(0,0), _active_instance(Invalid_inst)
{
	l4_kernel_info_t *kip = l4re_kip();
	MSG() << "kip found at 0x" << std::hex << kip
	      << " having " << L4::Kip::Mem_desc::count(kip)
	      << " entries.";

	L4::Kip::Mem_desc *d = L4::Kip::Mem_desc::first(kip);
	unsigned long count  = L4::Kip::Mem_desc::count(kip);

	for (L4::Kip::Mem_desc *desc = d; desc < d + count; ++desc) {

		if (!desc->is_virtual())
			continue;

		l4_addr_t s = desc->start() == 0 ? 0x1000 : desc->start();
		l4_addr_t e = desc->end();
		MSG() << "virtual range: [" << std::hex << s << "-" << e << "] ";

		switch(desc->type()) {
			case L4::Kip::Mem_desc::Conventional:
				set_limits(s,e);
				break;
			default:
				WARN() << "type " << desc->type() << "???";
				break;
		}

	}
}


void *Romain::Region_map::attach_locally(void* addr, unsigned long size,
                                         Romain::Region_handler *hdlr,
                                         unsigned flags, unsigned char align)
{
	long r; (void)align; (void)addr;
	Romain::Region_handler::Dataspace const *ds = &hdlr->memory(0);
	L4Re::Dataspace::Stats dsstat;
	(*ds)->info(&dsstat);

#if 0
	MSG() << "attaching DS " << std::hex << ds->cap()
	      << ", addr = "     << addr
	      << ", flags = "    << flags << " (" << dsstat.flags << ")"
	      << ", size = "     << size;
#endif

	if (!ds->is_valid()) {
		enter_kdebug("attaching invalid cap");
	}

	/*
	 * Here's what the variables below mean in the original DS. If the DS is
	 * read-only, we still want a writable version, because some users, such as
	 * GDB, want to modify ro data (e.g., GDB breakpoints). Therefore, we then
	 * create a copy of the original DS and adapt the variables afterwards.
	 *
	 * 1      2    3    4                      5
	 * +------+----+----+----------------------+
	 * |      |'..'|..'.|                      |       Original DS
	 * |      |.''.|''.'|                      |
	 * +------+----+----+----------------------+
	 *        \         \
	 *         \         \
	 *          \         \
	 *           \         \
	 *           +---------+
	 *           |''.''.''.|                           New DS
	 *           |..'..'..'|
	 *           +---------+
	 *           6         7
	 *
	 * 1 .. original DS start
	 * 2 .. page base of region (page_base)
	 * 3 .. offset within DS    (hdlr()->offset)
	 * 4 .. region end          (hdlr()->offset + size)
	 * 5 .. original DS end
	 * 6 .. new DS start
	 * 7 .. new DS end
	 *
	 * offset_in_page = $3 - $2
	 * eff_size       = size + offset_in_page rounded to page size
	 *
	 * After copy:
	 *    page_base      = 0
	 *    hdlr()->offset = 0
	 *
	 */

	l4_addr_t page_base      = l4_trunc_page(hdlr->offset());
	l4_addr_t offset_in_page = hdlr->offset() - page_base;
	unsigned eff_size        = l4_round_page(size + offset_in_page);

#if 1
	MSG() << "  page_base " << (void*)page_base << ", offset " << std::hex << offset_in_page
	      << ", eff_size " << eff_size << ", hdlr->offset " << hdlr->offset();
#endif

	hdlr->offset(page_base /*hdlr->offset() - offset_in_page*/);

	/*
	 * If the dataspace we attach was originally not mapped writable,
	 * make a writable copy here, so that fault handlers such as GDB
	 * can instrument this memory even if the client should not get
	 * it writable.
	 */
	if (!(dsstat.flags & L4Re::Dataspace::Map_rw)) {
		//MSG() << "Copying to rw dataspace.";
		L4::Cap<L4Re::Dataspace> mem;

		Romain::Region_map::allocate_ds(&mem, eff_size);

		mem->copy_in(0, *ds, page_base, hdlr->offset() + size);
		ds        = &mem; // taking pointer to stack?? XXX
		flags    &= ~L4Re::Rm::Read_only;
		page_base = 0;
		/*
		 * XXX: If we copied the dataspace above, what happens to the
		 *      original DS that was passed in? Shoudln't it be released?
		 *
		 *      No. It might still be attached elsewhere. Perhaps decrement
		 *      some refcnt? XXX check
		 */
	}

	/*
	 * flags contains the replica's flags setting. If the replica asks to attach into
	 * a previously reserved region, then this is only valid for the replica's
	 * rm::attach() call, but not for attaching our local version/copy of the DS.
	 */
	flags &= ~L4Re::Rm::In_area;

	l4_addr_t a = 0;
	r = L4Re::Env::env()->rm()->attach(&a, eff_size,
	                                   L4Re::Rm::Search_addr | flags,
	                                   *ds, page_base);
	_check(r != 0, "attach error");

	Romain::Region reg(a, a+eff_size - 1);
	reg.touch_rw();

	hdlr->set_local_region(_active_instance, reg);
	return (void*)(a + offset_in_page);
}

void *Romain::Region_map::attach(void* addr, unsigned long size,
                                 Romain::Region_handler const &hdlr,
                                 unsigned flags, unsigned char align, bool shared)
{
	void *ret = 0;
	Romain::Region_handler _handler(hdlr);

	_handler.shared(shared);

	/* Only attach locally, if this hasn't been done beforehand yet. */
	if (!_handler.local_region(_active_instance).start()) {
		void* a = attach_locally(addr, size, &_handler, flags, align);
		_check(a == 0, "Error in local attach");
	}
	ret = Base::attach(addr, size, _handler, flags, align);

	MSG() << "new mapping (" << _active_instance << ") "
	      << "[" << std::hex << _handler.local_region(_active_instance).start()
	      << " - " << _handler.local_region(_active_instance).end()
	      << "] -> " << ret;
	//enter_kdebug("after attach");

	return ret;
}



#if 0
int Romain::Region_map::detach(void* addr, unsigned long size, unsigned flags,
                            L4Re::Util::Region* reg, Romain::Region_handler* h)
{
	MSG() << "addr " << std::hex << l4_addr_t(addr) << " " << size
	      << " " << flags;
	MSG() << "active instance: " << _active_instance;
	enter_kdebug("detach");
	return -1;
}
#endif

bool Romain::Region_map::lazy_map_region(Romain::Region_map::Base::Node &n, unsigned inst)
{
	/* already mapped? */
	if (n->second.local_region(inst).start())
		return false;

	DEBUGf(Romain::Log::Memory) << "start " <<  n->second.local_region(inst).start();
	DEBUGf(Romain::Log::Memory) << "replica without yet established mapping.";
	DEBUGf(Romain::Log::Memory) << "ro: " << n->second.is_ro();
	DEBUGf(Romain::Log::Memory) << "shared: " << (n->second.shared() ? "true" : "false");

	/*
	 * As we found a node, we know there exists at least one replica
	 * that already has this mapping. Therefore, we go and look for
	 * one of those mappings to start from.
	 */
	int existing = find_existing_region(n);
	_check(existing < 0, "no mapping???");
	_check(existing > Romain::MAX_REPLICAS, "no mapping???");

	Romain::Rm_guard(this, inst);
	Romain::Region_handler *rh = const_cast<Romain::Region_handler*>(&n->second);

	/*
	 * Case 1: region is read-only -> we share the mapping from
	 *         the first node, because it was already established.
	 */
	
	if (n->second.is_ro() or rh->shared()) {
		/*
		 * XXX: Why is setting local_region and memory split up?
		 */
		rh->set_local_region(inst, n->second.local_region(existing));
		rh->memory(inst, n->second.memory(existing));
	} else {
		DEBUGf(Romain::Log::Memory) << "Copying existing mapping.";
		bool b = copy_existing_mapping(*rh, existing, inst);
		_check(!b, "error creating rw copy");
	}
	return true;
}
