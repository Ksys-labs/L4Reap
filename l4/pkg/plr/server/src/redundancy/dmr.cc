/*
 * dmr.cc --
 *
 *    n-way modular redundancy implementation 
 *
 * (c) 2011-2013 Björn Döbel <doebel@os.inf.tu-dresden.de>,
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include "../log"
#include "../redundancy.h"
#include "../app_loading"
#include "../fault_observers"
#include "../manager"
#include "../fault_handlers/syscalls_handler.h"

#define MSG() DEBUGf(Romain::Log::Redundancy)
#define MSGi(inst) MSG() << "[" << (inst)->id() << "] "

//extern char * __func__;

/* Replication protocol:
 * =====================
 *
 * Everyone goes to sleep, except the last thread to enter. This thread becomes
 * the 'leader'. The leader returns from this function with the First_syscall return
 * value. It then goes on to execute the system call (in manager.cc). Depending on
 * its return value,
 *
 * a) For replicatable calls: it stores its VCPU state after the system call using
 *    the function put(). All other replicas then use get() to obtain this state.
 *
 * b) For non-replicatable calls: it sets the other replicas' return value to 
 *    Repeat_syscall. The replicas then perform handling themselves.
 *
 * After all the handling, everyone waits in resume() until the last replica reaches
 * the resumption point. Then each VCPU goes back to where it came from.
 *
 *
 * Detection and recovery:
 * =======================
 *
 * Before executing the fault handler, the leader checksums all VCPU states. If a
 * mismatch is found, it calls the recover() function. recover() sets things straight
 * so that after the handler is done, everyone is in an identical state again. The leader
 * then goes on to execute the call.
 */

Romain::DMR::DMR(unsigned instances)
	: _enter_count(0), _leave_count(0), _block_count(0),
      _rv(Romain::RedundancyCallback::Invalid),
      _num_instances(instances), _num_instances_bak(0)
{
	for (unsigned i = 0; i < _num_instances; ++i)
		_orig_vcpu[i] = 0;
	_check(pthread_mutex_init(&_enter_mtx, NULL) != 0, "error initializing mtx");
	_check(pthread_cond_init(&_enter, NULL) != 0,      "error initializing condvar");
	_check(pthread_mutex_init(&_leave_mtx, NULL) != 0, "error initializing mtx");
	_check(pthread_cond_init(&_leave, NULL) != 0,      "error initializing condvar");
	_check(pthread_mutex_init(&_block_mtx, NULL) != 0, "error initializing mtx");
	_check(pthread_cond_init(&_block, NULL) != 0,      "error initializing condvar");
}


void
Romain::Replicator::put(Romain::App_thread *t)
{
	//memset(&_regs, 0, sizeof(_regs)); // XXX
#define PUT(field) _regs.field = t->vcpu()->r()->field
	PUT(es); PUT(ds); PUT(gs); PUT(fs);
	PUT(di); PUT(si); PUT(bp); PUT(pfa);
	PUT(ax); PUT(bx); PUT(cx); PUT(dx);
	PUT(trapno); PUT(err); PUT(ip); PUT(flags);
	PUT(sp); PUT(ss);
#undef PUT
	l4_utcb_t *addr = reinterpret_cast<l4_utcb_t*>(t->remote_utcb());
	memcpy(&_utcb, addr, L4_UTCB_OFFSET);
}


void
Romain::Replicator::get(Romain::App_thread *t)
{
#define PUT(field) t->vcpu()->r()->field = _regs.field
	PUT(es); PUT(ds); PUT(gs); PUT(fs);
	PUT(di); PUT(si); PUT(bp); PUT(pfa);
	PUT(ax); PUT(bx); PUT(cx); PUT(dx);
	PUT(trapno); PUT(err); PUT(ip); PUT(flags);
	PUT(sp); PUT(ss);
#undef PUT
	l4_utcb_t *addr = reinterpret_cast<l4_utcb_t*>(t->remote_utcb());
	memcpy(addr, &_utcb, L4_UTCB_OFFSET);
}

bool
Romain::DMR::checksum_replicas()
{
	unsigned long csums[MAX_REPLICAS] = {0, };
	unsigned idx;

	// calc checksums
	for (idx = 0; idx < _num_instances; ++idx)
		csums[idx] = _orig_vcpu[idx]->csum_state();

	// validate checksums
	for (idx = 1; idx < _num_instances; ++idx)
		if (csums[idx] != csums[idx-1]) {
#if 1
			ERROR() << "State mismatch detected!";
			ERROR() << "=== vCPU states ===";
			for (unsigned cnt = 0; cnt < _num_instances; ++cnt) {
				ERROR() << "--- instance " << cnt << " @ "
					<< _orig_vcpu[cnt]->vcpu() << " (cs: "
					<< std::hex << csums[cnt] << ") ---";
				if (_orig_vcpu[cnt]) {
					_orig_vcpu[cnt]->vcpu()->print_state();
					ERROR() << "vcpu.err = " << std::hex << _orig_vcpu[cnt]->vcpu()->r()->err;
				}
			}
			ERROR() << "Instances: " << _num_instances << " this inst " << idx;
			//enter_kdebug("checksum");
#endif
			return false;
		}

	return true;
}


class RecoverAbort
{
	public:
		static __attribute__((noreturn)) void recover()
		{
			static bool used = false;

			if (!used){
				used = true;
				ERROR() << "Aborting after error.";
				Romain::_the_instance_manager->show_stats();
				enter_kdebug("abort");
				throw("ERROR -> abort");
			}
		}
};


class RedundancyAbort
{
	public:
		static void recover(Romain::App_thread** threads, unsigned count,
							unsigned *good, unsigned *bad)
		{
			unsigned long csums[count];
			unsigned idx;

			// calc checksums
			for (idx = 0; idx < count; ++idx)
				csums[idx] = threads[idx]->csum_state();

			// find mismatch
			for (idx = 1; idx < count; ++idx)
				if (csums[idx] != csums[idx-1]) { // mismatch
					if (csums[idx] == csums[(idx + 1) % count]) {
						*good = idx;
						*bad  = idx-1;
					} else {
						*good = idx-1;
						*bad  = idx;
					}
				}
		}
};


void
Romain::DMR::recover(Romain::App_model *am)
{
	if (_num_instances < 3)
		RecoverAbort::recover(); // noreturn

	unsigned good = ~0, bad = ~0;
	RedundancyAbort::recover(_orig_vcpu, _num_instances, &good, &bad);
	DEBUG() << "good " << good << ", bad " << bad;

	// XXX: This does not suffice. We also need to copy memory content
	//      from a correct replica to the incorrect one
	replicator().put(_orig_vcpu[good]);
	replicator().get(_orig_vcpu[bad]);
	am->rm()->replicate(good, bad);

#if 0
	DEBUG() << "after recovery:";
	for (unsigned i = 0; i < _num_instances; ++i)
		DEBUG() << i << " " << std::hex << _orig_vcpu[i]->csum_state();
#endif
}


Romain::RedundancyCallback::EnterReturnVal
Romain::DMR::enter(Romain::App_instance *i, Romain::App_thread *t,
                   Romain::App_model *a)
{
	(void)a;
	MSGi(i) << "DMR::enter act(" << _enter_count << ")";

	Romain::RedundancyCallback::EnterReturnVal ret = Romain::RedundancyCallback::First_syscall;

	// enter ourselves into the list of faulted threads
	_orig_vcpu[i->id()] = t;

	pthread_mutex_lock(&_enter_mtx);

	/* TODO: select the first replica that makes the sum of all replicas
	 *       larger than N/2, if all their states match.
	 */
	if (++_enter_count < _num_instances) {
		//MSGi(i) << "I'm not the last instance -> going to wait.";
		// wait for the leader
		pthread_cond_wait(&_enter, &_enter_mtx);
		// get the return value set by the leader
		ret = _rv;
	} else {
		// everyone is here, so checksum the VCPUs now
		if (!checksum_replicas())
			recover(a);
		// at this point, recovery has made sure that all replicas
		// are in the same state.
	}

	--_enter_count;

	pthread_mutex_unlock(&_enter_mtx);

	/*
	 * If the leader told us to skip the syscall, get replicated VCPU and
	 * UTCB states here.
	 */
	if (ret == Romain::RedundancyCallback::Skip_syscall) {
		replicator().get(t);
	}

	return ret;
}


void Romain::DMR::leader_repeat(Romain::App_instance *i, Romain::App_thread *t,
                                Romain::App_model *a)
{
	(void)i; (void)t; (void)a;
	MSGi(i) << __func__;
	_rv = Romain::RedundancyCallback::Repeat_syscall;
}


void Romain::DMR::leader_replicate(Romain::App_instance *i, Romain::App_thread *t,
                                   Romain::App_model *a)
{
	(void)i; (void)t; (void)a;
	MSGi(i) << __func__;
	_rv = Romain::RedundancyCallback::Skip_syscall;

	//t->print_vcpu_state();
	replicator().put(t);
}


void Romain::DMR::resume(Romain::App_instance *i, Romain::App_thread *t,
                         Romain::App_model *a)
{
	(void)i; (void)t; (void)a;
	//MSGi(i) << "[l] acquiring leave mtx";
	pthread_mutex_lock(&_leave_mtx);
	if (_leave_count == 0) {
		pthread_mutex_lock(&_enter_mtx);
		pthread_cond_broadcast(&_enter);
		pthread_mutex_unlock(&_enter_mtx);
	}

	//MSGi(i) << "++_leave_count " << _leave_count;
	if (++_leave_count < _num_instances) {
		MSGi(i) << "Waiting for other replicas to commit their syscall.";
		//MSGi(i) << "cond_wait(leave)";
		pthread_cond_wait(&_leave, &_leave_mtx);
		//MSGi(i) << "success: cond_wait(leave)";
	} else {
		for (unsigned i = 0; i < _num_instances; ++i)
			_orig_vcpu[i] = 0;
		pthread_cond_broadcast(&_leave);
	}
	//MSGi(i) << "counts @ resume: " << _enter_count << " " << _leave_count;
	--_leave_count;
	pthread_mutex_unlock(&_leave_mtx);

	//enter_kdebug("DMR::resume");
}

void Romain::DMR::wait(Romain::App_instance *i, Romain::App_thread *t,
                       Romain::App_model *a)
{
	MSGi(i) << __func__;
	pthread_mutex_lock(&_block_mtx);
	++_block_count;
	MSGi(i) << "going to wait. block_count: " << _block_count;
	pthread_cond_broadcast(&_enter);
	pthread_cond_wait(&_block, &_block_mtx);
	pthread_mutex_unlock(&_block_mtx);
}

void Romain::DMR::silence(Romain::App_instance *i, Romain::App_thread *t,
                          Romain::App_model *a)
{
	MSGi(i) << __func__;
	// 1. Tell anyone who is still waiting to enter that he can now do so.
	//    These replicas will all run until they block on _block_mtx.
	pthread_cond_broadcast(&_enter);

	while (_block_count < (_num_instances - 1))
		l4_sleep(20); // XXX handshake

	_num_instances_bak = _num_instances;
	_num_instances     = 1;
}

void Romain::DMR::wakeup(Romain::App_instance *i, Romain::App_thread *t,
                         Romain::App_model *a)
{
	MSGi(i) << __func__;
	_block_count   = 0;
	_num_instances = _num_instances_bak;
	pthread_cond_broadcast(&_block);
}
