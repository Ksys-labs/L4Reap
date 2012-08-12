/*
 * romain_syscalls.cc --
 *
 *     Implementation of Romain syscall handling.
 *
 * (c) 2011 Björn Döbel <doebel@os.inf.tu-dresden.de>,
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include "../log"
#include "../app_loading"
#include "../locking.h"
#include "../manager"

#include "observers.h"

#include <l4/re/util/region_mapping_svr>
#include <l4/sys/segment.h>

#define MSG() DEBUGf(Romain::Log::Faults)

DEFINE_EMPTY_STARTUP(SyscallObserver)

static unsigned long num_syscalls;

namespace Romain {
extern Romain::InstanceManager *_the_instance_manager;
}

void Romain::SyscallObserver::status() const
{
	INFO() << "System call count: " << num_syscalls;
}

Romain::Observer::ObserverReturnVal
Romain::SyscallObserver::notify(Romain::App_instance *i,
                                Romain::App_thread *t,
                                Romain::App_model *a)
{
	Romain::Observer::ObserverReturnVal retval = Romain::Observer::Ignored;

	if (t->vcpu()->r()->trapno != 13) {
		return retval;
	}

	/* SYSENTER / INT30 */
	if (t->vcpu()->r()->ip == 0xEACFF003) {
		++num_syscalls;

		l4_msgtag_t *tag = reinterpret_cast<l4_msgtag_t*>(&t->vcpu()->r()->ax);
		MSG() << "SYSENTER. tag = " << std::hex << tag->label();

		/*
		 * Fiasco-specific:
		 *      EBX is return address
		 *      EBP is return ESP
		 * -> we need to remember these
		 */
		l4_addr_t ebx_pre = t->vcpu()->r()->bx;
		l4_addr_t ebp_pre = t->vcpu()->r()->bp;

		/*
		 * XXX:
		 * This should not only distinguish between the protocol type used
		 * for invocation but also between local and remote objects. Remote
		 * invocations should always be redirected (e.g., ex_reg'ing a remote
		 * thread should be okay) and only local object invocations should be
		 * okay.
		 *
		 * XXX: By the way: How does a remote ex_regs() work if the remote thread
		 *      object is also running in a replicated task?
		 *      -> The remote master will provide an IPC gate that implements the
		 *         thread protocol and intercepts&emulates the ex_regs call.
		 */
		switch(tag->label()) {
			case L4_PROTO_THREAD:
				/* 
				 * Each instance needs to perform its own
				 * thread creation.
				 */
				handle_thread(i, t, a);
				retval = Romain::Observer::Finished;
				break;
			case L4_PROTO_TASK:
				if ((t->vcpu()->r()->dx & ~0xF) == L4RE_THIS_TASK_CAP) {
					handle_task(i, t, a);
					retval = Romain::Observer::Finished;
				} else {
					do_proxy_syscall(i, t, a);
					retval = Romain::Observer::Replicatable;
				}
				break;
			case L4Re::Protocol::Rm:
				/*
				 * Region management is done only once as e.g.,
				 * regions for attaching need to be replicated
				 * across instances. The real adaptation then
				 * happens during page fault handling.
				 */
				handle_rm(i, t, a);
				retval = Romain::Observer::Replicatable;
				break;
			case L4Re::Protocol::Parent:
				/*
				 * The parent protocol is only used for exitting.
				 */
				{
					struct timeval tv;
					gettimeofday(&tv, 0);
					INFO() << "Instance " << i->id() << " exitting. Time "
					       << "\033[33;1m" << tv.tv_sec << "." << tv.tv_usec << "\033[0m";
					Romain::_the_instance_manager->query_observer_status();
					if (1) enter_kdebug("*#^");
					do_proxy_syscall(i, t, a);
					retval = Romain::Observer::Replicatable;
				}
				break;
			default:
				/*
				 * Proxied syscalls are always only executed
				 * once because for the outside world it must
				 * look as if the master server _is_ the one
				 * application everyone is talking to.
				 */
				do_proxy_syscall(i, t, a);
				retval = Romain::Observer::Replicatable;
				//enter_kdebug("after proxying");
				break;
		}

		t->vcpu()->r()->ip = ebx_pre;
		t->vcpu()->r()->sp = ebp_pre;

	} else {
		MSG() << "GPF";
	}

	//enter_kdebug("done syscall");
	return retval;
}


void Romain::SyscallObserver::do_proxy_syscall(Romain::App_instance *,
                                               Romain::App_thread* t,
                                               Romain::App_model *)
{
	char backup_utcb[L4_UTCB_OFFSET]; // for storing local UTCB content

	l4_utcb_t *addr = reinterpret_cast<l4_utcb_t*>(t->remote_utcb());
	l4_utcb_t *cur_utcb = l4_utcb();
	MSG() << "UTCB @ " << std::hex << (unsigned)addr;

	/*
	 * We are going to perform the system call on behalf of the client. This will
	 * thrash our local UTCB, so we want to store it here.
	 */
	store_utcb((char*)cur_utcb, backup_utcb);
	store_utcb((char*)addr, (char*)cur_utcb);

	//t->vcpu()->print_state();
	//Romain::dump_mem((unsigned*)addr, 40);

	/* Perform Fiasco system call */
	asm volatile (L4_ENTER_KERNEL
	              : "=a" (t->vcpu()->r()->ax),
	              "=b" (t->vcpu()->r()->bx),
	              /* ECX, EDX are overwritten anyway */
	              "=S" (t->vcpu()->r()->si),
	              "=D" (t->vcpu()->r()->di)
	              : "a" (t->vcpu()->r()->ax),
	              /* EBX and EBP will be overwritten with local
	               * values in L4_ENTER_KERNEL */
	                "c" (t->vcpu()->r()->cx),
	                "d" (t->vcpu()->r()->dx),
	                "S" (t->vcpu()->r()->si),
	                "D" (t->vcpu()->r()->di)
	              : "memory", "cc"
	);

	/*
	 * Restore my UTCB
	 */
	store_utcb((char*)cur_utcb, (char*)addr);
	store_utcb(backup_utcb, (char*)cur_utcb);

	//t->vcpu()->print_state();
	//Romain::dump_mem((unsigned*)addr, 40);
	//enter_kdebug("done syscall");
}


void Romain::SyscallObserver::handle_rm(Romain::App_instance* i,
                                        Romain::App_thread* t,
                                        Romain::App_model * a)
{
	MSG() << "RM PROTOCOL";

	l4_utcb_t *utcb = reinterpret_cast<l4_utcb_t*>(t->remote_utcb());
	MSG() << "UTCB @ " << std::hex << (unsigned)utcb;

	//t->vcpu()->print_state();
	//Romain::dump_mem((unsigned*)utcb, 40);

	{
		Romain::Rm_guard r(a->rm(), i->id());
		L4::Ipc::Iostream ios(utcb);
		L4Re::Util::region_map_server<Romain::Region_map_server>(a->rm(), ios);
	}

	//t->vcpu()->print_state();
	//Romain::dump_mem((unsigned*)utcb, 40);
}

void Romain::SyscallObserver::handle_thread(Romain::App_instance *,
                                            Romain::App_thread* t,
                                            Romain::App_model *)
{
	MSG() << "Thread system call";
	l4_utcb_t *utcb = reinterpret_cast<l4_utcb_t*>(t->remote_utcb());
	MSG() << "UTCB @ " << std::hex << (unsigned)utcb;

	//t->vcpu()->print_state();
	//Romain::dump_mem((unsigned*)utcb, 40);

	l4_umword_t op = l4_utcb_mr_u(utcb)->mr[0] & L4_THREAD_OPCODE_MASK;

	switch(op) {
		case L4_THREAD_CONTROL_OP:
			enter_kdebug("THREAD: control");
			break;
		case L4_THREAD_EX_REGS_OP:
			enter_kdebug("THREAD: ex_regs");
			break;
		case L4_THREAD_SWITCH_OP:
			enter_kdebug("THREAD: switch");
			break;
		case L4_THREAD_STATS_OP:
			enter_kdebug("THREAD: stats");
			break;
		case L4_THREAD_VCPU_RESUME_OP:
			enter_kdebug("THREAD: vcpu_resume");
			break;
		case L4_THREAD_REGISTER_DELETE_IRQ_OP:
			enter_kdebug("THREAD: irq");
			break;
		case L4_THREAD_MODIFY_SENDER_OP:
			enter_kdebug("THREAD: modify sender");
			break;
		case L4_THREAD_VCPU_CONTROL_OP:
			enter_kdebug("THREAD: vcpu control");
			break;
		case L4_THREAD_VCPU_CONTROL_EXT_OP:
			enter_kdebug("THREAD: vcpu control ext");
			break;
		case L4_THREAD_GDT_X86_OP:
			handle_thread_gdt(t, utcb);
			break;
		case L4_THREAD_SET_FS_AMD64_OP:
			enter_kdebug("THREAD: set fs amd64");
			break;
		default:
			ERROR() << "unknown thread op: " << std::hex << op;
			break;
	}

	//enter_kdebug("thread");
}

void Romain::SyscallObserver::handle_thread_gdt(Romain::App_thread* t,
                                                l4_utcb_t *utcb)
{
	l4_msgtag_t *tag = reinterpret_cast<l4_msgtag_t*>(&t->vcpu()->r()->ax);
	MSG() << "GDT: words = " << tag->words();

	// 1 word -> query GDT start
	if (tag->words() == 1) {
		l4_utcb_mr_u(utcb)->mr[0] = 0x48 >> 3;
		t->vcpu()->r()->ax = l4_msgtag(0, 1, 0, 0).raw;
	} else { // setup new GDT entry
		unsigned idx = l4_utcb_mr_u(utcb)->mr[1];
		unsigned numbytes = (tag->words() == 4) ? 8 : 16;
		if (idx == 0) {
			t->write_gdt(&l4_utcb_mr_u(utcb)->mr[2], numbytes);
		} else {
			enter_kdebug("XXX");
		}
		t->vcpu()->r()->ax = l4_msgtag((idx << 3) + 0x48 + 3, 0, 0, 0).raw;
	}
}


void Romain::SyscallObserver::handle_task(Romain::App_instance* i,
                                          Romain::App_thread*   t,
                                          Romain::App_model*    a)
{
	l4_utcb_t   *utcb = reinterpret_cast<l4_utcb_t*>(t->remote_utcb());
	l4_umword_t    op = l4_utcb_mr_u(utcb)->mr[0] & L4_THREAD_OPCODE_MASK;
	switch(op) {
		case L4_TASK_UNMAP_OP:
			MSG() << "unmap";
			i->unmap(l4_utcb_mr_u(utcb)->mr[2]);
			break;
		case L4_TASK_CAP_INFO_OP:
			do_proxy_syscall(i,t,a);
			break;
		default:
			MSG() << "Task system call";
			MSG() << "UTCB @ " << std::hex << (unsigned)utcb << " op: " << op
				  << " cap " << (t->vcpu()->r()->dx & ~0xF) << " " << L4RE_THIS_TASK_CAP;
			t->vcpu()->print_state();
			enter_kdebug("unknown task op?");
			break;
	}
}
