#pragma once

/*
 * Traditional vCPU handling method: every replica raises a fault
 * on its local vCPU and handles it on the own physical core.
 */
#define LOCAL_HANDLING 1

/*
 * Resilient cores test case 1: Upon a vCPU fault, migrate
 * the faulting vCPU to a resilient core (== CPU0). There,
 * handling is executed (and can be assumed to not fail). Before
 * resuming the vCPU, migrate back to the replica CPU, which
 * is assumed to be unsafe.
 */
#define MIGRATE_VCPU 0

/*
 * Resilient cores test case 2: Have a dedicated vCPU handler
 * thread on a resilient core (==CPU0). The non-resilient vCPU
 * handler then only sends an IPC to this thread in order to trigger
 * handling.
 */
#define SPLIT_HANDLING 0

namespace Romain
{
	enum {
		MAX_REPLICAS       =  3,        // maximum # of allowed replicas
		MAX_OBSERVERS      = 16,        // maximum # of fault observers
		TRAMPOLINE_SIZE    = 64,        // size of the per-thread trampoline area
		HANDLER_STACK_SIZE = (1 << 14), // size of the VCPU handler stack
	};
}
