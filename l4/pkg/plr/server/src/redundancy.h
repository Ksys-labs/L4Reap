#pragma once

#include "app"
#include <pthread-l4.h>

namespace Romain {

	class App_model;
	class App_thread;
	class App_instance;

	class Replicator {

		/*
		 * Register and UTCB state used for replicatable syscalls
		 */
		l4_vcpu_regs_t  _regs;
		char            _utcb[L4_UTCB_OFFSET]; // max UTCB size

		public:
			Replicator()
			{
				memset(&_regs, 0, sizeof(l4_vcpu_regs_t));
				memset(_utcb,  0, L4_UTCB_OFFSET);
			}

			void put(Romain::App_thread* t);
			void get(Romain::App_thread* t);
	};

	/*
	 * Generic interface for a redundancy checker
	 *
	 * An object matching this interface is used by the vCPU fault handler (see
	 * server/src/handler.cc:VCPU_handler) to track redundant handler
	 * execution.
	 */
	class RedundancyCallback
	{
		Replicator      _replicator;

		public:

			Replicator& replicator() { return _replicator; }

			/*
			 * Return value from enter() call.
			 */
			enum EnterReturnVal {
				Invalid,
				/*
				 * You are the first to exec this fault handler
				 * -> for replicatable system calls this triggers the initial
				 *    execution whose result is then copied to all other
				 *    replicas.
				 */
				First_syscall,
				/*
				 * This tells us that we are not the initial executor of the
				 * syscall, but it is to be repeated by every replica itself.
				 */
				Repeat_syscall,
				/*
				 * Skip the handler completely.
				 */
				Skip_syscall,
			};

			/*
			 * Enter redundant handler.
			 *
			 * Determines if the handler needs to be executed.
			 */
			virtual EnterReturnVal enter(Romain::App_instance *i, Romain::App_thread *t, Romain::App_model *a) = 0;

			/*
			 * Function for the master replica to notify subsequent ones to
			 * not execute a handler, but use the master's return state (UTCB, vCPU)
			 * instead.
			 */
			virtual void leader_replicate(Romain::App_instance *i, Romain::App_thread *t, Romain::App_model *a) = 0;

			/*
			 * Master telling the replicas to run the fault handler
			 * themselves.
			 */
			virtual void leader_repeat(Romain::App_instance *i, Romain::App_thread *t, Romain::App_model *a) = 0;

			/*
			 * Function called by each replica before resuming execution
			 * or handling of the next pending fault.
			 */
			virtual void resume(Romain::App_instance *i, Romain::App_thread *t, Romain::App_model *a) = 0;

			/*
			 * Block this replica until explicitly woken up.
			 */
			virtual void wait(Romain::App_instance *i, Romain::App_thread *t, Romain::App_model *a) = 0;

			/*
			 * Wait until all other replicas got into wait state.
			 */
			virtual void silence(Romain::App_instance *i, Romain::App_thread *t, Romain::App_model *a) = 0;

			/*
			 * Wake up all other replicas.
			 */
			virtual void wakeup(Romain::App_instance *i, Romain::App_thread *t, Romain::App_model *a) = 0;
	};


	class NoRed : public Romain::RedundancyCallback
	{
		public:
			virtual EnterReturnVal enter(Romain::App_instance *, Romain::App_thread *, Romain::App_model *)
			{ return Romain::RedundancyCallback::First_syscall; }
			virtual void leader_replicate(Romain::App_instance *, Romain::App_thread *, Romain::App_model *) {}
			virtual void leader_repeat(Romain::App_instance *, Romain::App_thread *, Romain::App_model *) {}
			virtual void resume(Romain::App_instance *, Romain::App_thread *, Romain::App_model *) {}
			virtual void wait(Romain::App_instance *i, Romain::App_thread *t, Romain::App_model *a)
			{ enter_kdebug("single instances should never wait."); }
			virtual void silence(Romain::App_instance *i, Romain::App_thread *t, Romain::App_model *a) {}
			virtual void wakeup(Romain::App_instance *i, Romain::App_thread *t, Romain::App_model *a)  {}
	};


	/*
	 * N-way modular redundancy.
	 *
	 * For details on the internal protocol used, see redundancy/dmr.cc
	 */
	class DMR : public Romain::RedundancyCallback
	{
		/* Replica sync points */

		/* Sync 1: everyone waits upon entering the fault handler */
		pthread_cond_t  _enter;
		pthread_mutex_t _enter_mtx;
		unsigned        _enter_count;

		/* Sync 2: everyone waits before leaving the fault handler */
		pthread_cond_t  _leave;
		pthread_mutex_t _leave_mtx;
		unsigned        _leave_count;

		/* Sync 3: additional sync point used by the fault injection framework
		 * XXX: should this be here? */
		pthread_cond_t  _block;
		pthread_mutex_t _block_mtx;
		unsigned        _block_count;

		EnterReturnVal  _rv;

		/*
		 * Effective number of running instances
		 *
		 * -> this number may change at runtime (if we switch off some replicas
		 *    for a while). In this case, _num_instances_bak contains the original
		 *    number of instances.
		 */
		unsigned        _num_instances;
		unsigned        _num_instances_bak;

		/*
		 * List of replicas waiting to execute their fault handler
		 */
		Romain::App_thread* _orig_vcpu[Romain::MAX_REPLICAS];

		/*
		 * Checksum all replicas, return if they match.
		 */
		bool checksum_replicas();

		/*
		 * Recover after checksum mismatch.
		 */
		void recover(Romain::App_model*);

		public:

			DMR(unsigned instances);

			virtual EnterReturnVal enter(Romain::App_instance *i, Romain::App_thread *t, Romain::App_model *a);
			virtual void leader_replicate(Romain::App_instance *i, Romain::App_thread *t, Romain::App_model *a);
			virtual void leader_repeat(Romain::App_instance *i, Romain::App_thread *t, Romain::App_model *a);
			virtual void resume(Romain::App_instance *i, Romain::App_thread *t, Romain::App_model *a);
			virtual void wait(Romain::App_instance *i, Romain::App_thread *t, Romain::App_model *a);
			virtual void silence(Romain::App_instance *i, Romain::App_thread *t, Romain::App_model *a);
			virtual void wakeup(Romain::App_instance *i, Romain::App_thread *t, Romain::App_model *a);
	};
}
