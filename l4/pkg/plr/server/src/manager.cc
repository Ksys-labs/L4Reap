/*
 * manager.cc --
 *
 *     Instance manager implementation.
 *
 * (c) 2011-2013 Björn Döbel <doebel@os.inf.tu-dresden.de>,
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include "manager"
#include "app_loading"
#include "configuration"

#include <l4/sys/segment.h>
#include <l4/re/mem_alloc>
#include <l4/re/rm>
#include <l4/re/env>
#include <l4/re/dataspace>
#include <l4/re/util/cap_alloc>
#include <l4/plr/uu.h>

#define MSG() DEBUGf(Romain::Log::Manager)
#include "fault_handlers/syscalls_factory.h"

Romain::Configuration Romain::globalconfig;


L4_INLINE unsigned countbits(long v)
{
	v = v - ((v >> 1) & 0x55555555);                         // reuse input as temporary
	v = (v & 0x33333333) + ((v >> 2) & 0x33333333);          // temp
	return ((v + ((v >> 4) & 0xF0F0F0F)) * 0x1010101) >> 24; // count
}


L4_INLINE l4_umword_t count_online_cpus()
{
	l4_umword_t maxcpu = 0;
	l4_sched_cpu_set_t cpuonline = l4_sched_cpu_set(0, 0);
	if (l4_error(L4Re::Env::env()->scheduler()->info(&maxcpu, &cpuonline)) < 0) {
		ERROR() << "reading CPU info";
	}

	INFO() << "Online " << countbits(cpuonline.map) << " / MAX " << maxcpu;

	return countbits(cpuonline.map) > maxcpu ? maxcpu : countbits(cpuonline.map);
}


Romain::InstanceManager::InstanceManager(unsigned int argc,
                                         char const **argv,
                                         unsigned num_instances)
	: LogicalCPUMap(),
	  _am(0),
	  _instances(),
	  _threadgroups(),
	  _num_observers(0),
	  _num_inst(num_instances),
	  _num_cpu(1),
	  _argc(argc), // XXX: remove
	  _argv(argv), // XXX: remove
	  _logBuf(0)
{
	configure();

	_gdt_min = fiasco_gdt_get_entry_offset(L4_INVALID_CAP, l4_utcb());
	MSG() << "GDT MIN: " << _gdt_min;

	_num_cpu = count_online_cpus();
	/*
	 * initial parameter is argv for the client program, this means
	 * *argv is the file name to load.
	 */
	_name = *argv;

	_am = new Romain::App_model(_name, argc, argv);
	Romain::Elf_Ldr loader(_am);
	//loader.load();
	loader.launch();

	_init_eip = _am->prog_info()->entry;
	_init_esp = _am->prog_info()->stack_addr;
	INFO() << "Program entry point at 0x" << std::hex << _init_eip;
	INFO() << "              stack at 0x" << std::hex << _init_esp;

#if SPLIT_HANDLING
	int res = pthread_create(&_split_handler, 0, split_handler_fn, this);
	_check(res != 0, "could not create split handler thread");
#endif
}


void Romain::InstanceManager::configure_logflags(char *flags)
{
	printf("flags %p\n", flags);
	if (!flags) {
		Romain::Log::logFlags = 0;
	} else {
		unsigned max = strlen(flags);
		for (unsigned j = 0; j < max; ++j) {
			if (flags[j] == ',') flags[j] = 0;
		}

		char const *c = flags;
		while (c <= flags + max) {
			printf("  %s\n", c);
			if ((strcmp(c, "mem") == 0) || (strcmp(c, "memory") == 0)) {
				Romain::Log::logFlags |= Romain::Log::Memory;
			} else if (strcmp(c, "emulator") == 0) {
				Romain::Log::logFlags |= Romain::Log::Emulator;
			} else if (strcmp(c, "manager") == 0) {
				Romain::Log::logFlags |= Romain::Log::Manager;
			} else if (strcmp(c, "faults") == 0) {
				Romain::Log::logFlags |= Romain::Log::Faults;
			} else if (strcmp(c, "redundancy") == 0) {
				Romain::Log::logFlags |= Romain::Log::Redundancy;
			} else if (strcmp(c, "loader") == 0) {
				Romain::Log::logFlags |= Romain::Log::Loader;
			} else if (strcmp(c, "swifi") == 0) {
				Romain::Log::logFlags |= Romain::Log::Swifi;
			} else if (strcmp(c, "gdb") == 0) {
				Romain::Log::logFlags |= Romain::Log::Gdb;
			} else if (strcmp(c, "all") == 0) {
				Romain::Log::logFlags = Romain::Log::All;
			}

			c += (strlen(c)+1);
		}
		printf("Flags: %08lx\n", Romain::Log::logFlags);
	}
}


void Romain::InstanceManager::configure_fault_observers()
{
	/*
	 * First, register those observers that don't interfere
	 * with anyone else and get notified all the time.
	 */
	DEBUG() << "[observer] vcpu state.";
	BoolObserverConfig("general:print_vcpu_state",
	                   this, "vcpu_state");
	DEBUG() << "[observer] trap limit.";
	ObserverConfig(this, "trap_limit");

	/*
	 * Always needed -- slightly ordered by the number of
	 * calls they are expected to see, so that we minimize
	 * the amount of unnecessary observer callbacks.
	 */
	DEBUG() << "[observer] page faults.";
	ObserverConfig(this, "pagefaults");
	DEBUG() << "[observer] syscalls.";
	ObserverConfig(this, "syscalls");
	DEBUG() << "[observer] threads.";
	BoolObserverConfig("general:threads", this, "threads");
	DEBUG() << "[observer] trap.";
	ObserverConfig(this, "trap");

	DEBUG() << "[observer] simpledbg.";
	StringObserverConfig("general:debug", this);
	DEBUG() << "[observer] intercept-kip.";
	BoolObserverConfig("general:intercept_kip", this, "kip-time");
	DEBUG() << "[observer] swifi.";
	BoolObserverConfig("general:swifi", this, "swifi");
	DEBUG() << "[observer] logreplica.";
	BoolObserverConfig("general:logreplica", this, "replicalog");
}


void Romain::InstanceManager::configure_redundancy()
{
	char const *redundancy = ConfigStringValue("general:redundancy");
	if (!redundancy) redundancy = "none";
	INFO() << "red: '" << redundancy << "'";
	if (strcmp(redundancy, "none") == 0) {
		_num_inst = 1;
	} else if (strcmp(redundancy, "dual") == 0) {
		_num_inst = 2;
	} else if (strcmp(redundancy, "triple") == 0) {
		_num_inst = 3;
	} else {
		ERROR() << "Invalid redundancy setting: " << redundancy;
		enter_kdebug("Invalid redundancy setting");
	}
}


void Romain::InstanceManager::configure_logbuf(int sizeMB)
{
	INFO() << "Log buffer size: " << sizeMB << " MB requested.";
	unsigned size_in_bytes = sizeMB << 20;

	L4::Cap<L4Re::Dataspace> ds;

	l4_addr_t addr = Romain::Region_map::allocate_and_attach(&ds, size_in_bytes, 0, L4_SUPERPAGESHIFT);
    INFO() << "Log buffer attached to 0x" << std::hex << addr;

    _logBuf->set_buffer(reinterpret_cast<unsigned char*>(addr), size_in_bytes);
}


/*
 * Romain ini file settings
 * =====================
 *
 *  'general' section
 *  -----------------
 *
 *  The 'general' section determines which fault handlers are registered.
 *
 *  print_vcpu_state [bool]
 *     - Registers a handler printing the state of a VCPU upon every
 *       fault entry
 *
 *  debug [string = {simple,gdb}]
 *     - Configures a debugger stub. 'simple' refers to builtin debugging,
 *       'gdb' starts a gdb stub. Further configuration for the debuggers
 *       is done in separate INI sections.
 *
 *  page_fault_handling [string = {ro}]
 *     - Specify the way in which paging is done.
 *       'ro' means that client memory is mapped read-only and write
 *       accesses to the respective regions are emulated.
 *
 *  redundancy [string = {dual, triple}]
 *     - configure the number of replicas that are started
 *
 *
 *  log [string list]
 *     - comma-separated list of strings for configuring logging
 *     - available flags are:
 *       - mem|memory -> memory management
 *       - emulator   -> instruction emulation
 *       - manager    -> replica management
 *       - faults     -> generic fault entry path
 *       - redundancy -> DMR/TMR-specific logs
 *       - swifi      -> fault injetion
 *       - gdb        -> GDB stub logging
 *       - all        -> everything
 *
 *  logbuf [int] (-1)
 *       - establish a log buffer with the given size in MB
 *       - runtime events are logged into this buffer and can later
 *         be dumped for postprocessing -> this is an alternative to
 *         printing a lot of stuff to the serial console
 *
 *  logcpu [int]
 *       - event generation needs a global timestamp. On real SMP hardware
 *         CPUs disagree on their local TSC values. As a workaround, we start
 *         a dedicated thread that busily writes its local TSC to a global timer
 *         variable that is then read by everyone else. This of course requires
 *         the thread to solely run on a dedicated CPU. This option sets the
 *         respective CPU #.
 *
 *  logrdtsc [bool] (false)
 *       - use local TSC instead of global time stamp counter for event timestamps
 *         -> use on Qemu where a dedicated timestamp thread does not work properly
 *
 *  logreplica [bool] (false)
 *       - assign each replica a log buffer (mapped to REPLICA_LOG_ADDRESS)
 *
 *  logtimeout [int] (-1)
 *       - run the replicated app for N seconds, then halt all threads and
 *         print replica stats (as if on termination)
 *
 *  replicalogsize [int] (-1)
 *       - buffser size for the replica-specific log buffer
 *
 *  swifi [bool] (false)
 *       - Perform fault injection experiments, details are configured
 *         in the [swifi] section.
 *
 *  kip-time [bool] (false)
 *       - Turn on/off KIP timer access. This is used to turn replica
 *         accesses to the clock field of the KIP into traps (by placing
 *         software breakpoints on specifically configured instructions).
 *         Use this, if your application needs clock info from the KIP.
 *
 *  max_traps [int] (-1)
 *       - Handle a maximum amount of traps before terminating the 
 *         replicated application. Use as a debugging aid.
 *
 *  print_time [bool] (true)
 *       - include timing information in printed output.
 *
 *  'gdbstub' section
 *  -----------------
 *
 *  This section configures the behavior of the GDB stub.
 *
 *  port [int]
 *     - Configures the GDB stub to use a TCP/IP connection and wait
 *       for a remote GDB to connect on the port specified. If this
 *       option is _not_ set, the GDB stub will try to use a serial
 *       connection through COM2.
 *
 *  XXX make COM port configurable
 *
 *  'simpledbg' section
 *  -------------------
 *
 *  This section configures Romain's builtin debugger, which is programmed through
 *  INI file commands only and performs a narrow range of debugging tasks only.
 *
 *  singlestep [int]
 *     - Patches an INT3 breakpoint to the given address. Then executes the program
 *       until the breakpoint is hit and thereafter switches to single-stepping
 *       mode.
 *
 *  'kip-time' section
 *  ------------------
 *
 *  The KIP-time instrumentation needs a list of addresses that point to
 *  KIP->clock accessing instructions. These are supplied as a comma-separated
 *  list of hex values for the target command.
 *
 *  target [comma-separated list of hex addresses]
 *
 *  'swifi' section
 *  ---------------
 *
 *  Configures fault-injection experiments that are performed on replicas.
 *  By default, SWIFI currently injects faults into replica #0.
 *
 *  - target [hex]
 *    specifies an address to place a breakpoint on. Upon hitting this
 *    BP, a SWIFI injection is performed.
 *
 *  - inject [string]
 *    specifies what kind of injection to perform when hitting the BP.
 *    Available values:
 *      - 'gpr' -> flip a random bit in a randomly selected
 *                 general-purpose register
 */
void Romain::InstanceManager::configure()
{
#define USE_SHARABLE_TIMESTAMP 1

	int logMB = ConfigIntValue("general:logbuf");

#if USE_SHARABLE_TIMESTAMP
	_logBuf = new Measurements::EventBuf(true);
	L4::Cap<L4Re::Dataspace> tsds;
	l4_addr_t ts_addr = Romain::Region_map::allocate_and_attach(&tsds, L4_PAGESIZE);
	l4_touch_ro((void*)ts_addr, L4_PAGESIZE);
	_logBuf->set_tsc_buffer(reinterpret_cast<l4_uint64_t*>(ts_addr));
#else
	_logBuf = new Measurements::EventBuf();
#endif
	if (logMB != -1) {
		configure_logbuf(logMB);
	}

	Log::logLocalTSC = ConfigBoolValue("general:logrdtsc", false);

	/*
	 * These modes are exclusive: either we use the local TSC _xor_ we start a
	 * timer thread on a dedicated CPU.
	 */
	if (!Log::logLocalTSC) {
		int logCPU = ConfigIntValue("general:logcpu");
		if (logCPU != -1) {
			INFO() << "Starting counter thread on CPU " << logCPU;
			INFO() << "Timestamp @ 0x" << std::hex << (l4_addr_t)_logBuf->timestamp;
			Measurements::EventBuf::launchTimerThread((l4_addr_t)_logBuf->timestamp,
			                                          logCPU);
		}
	}

	char *log = strdup(ConfigStringValue("general:log", "none"));
	configure_logflags(log);
	
	Log::withtime = ConfigBoolValue("general:print_time", true);

	configure_fault_observers();
	configure_redundancy();
	free(log);
}


void Romain::InstanceManager::logdump()
{
	int logMB = ConfigIntValue("general:logbuf");
	if (logMB != -1) {
		char const *filename = "sampledump.txt";

		unsigned oldest = _logBuf->oldest();
		unsigned dump_start, dump_size;

		if (oldest == 0) { // half-full -> dump from 0 to index
			dump_start = 0;
			dump_size  = _logBuf->index * sizeof(Measurements::GenericEvent);
		} else { // buffer completely full -> dump full size starting from oldest entry
			dump_start = oldest * sizeof(Measurements::GenericEvent);
			dump_size  = _logBuf->size * sizeof(Measurements::GenericEvent);
		}

		uu_dumpz_ringbuffer(filename, _logBuf->buffer,
		                    _logBuf->size * sizeof(Measurements::GenericEvent),
		                    dump_start, dump_size);
	}
}


/*
 * Prepare the stack that is used by the fault handler whenever a
 * VCPU enters the master task.
 *
 * This pushes relevant pointers to the stack so that the handler
 * functions can use them as parameters.
 */
l4_addr_t Romain::InstanceManager::prepare_stack(l4_addr_t sp,
                                                 Romain::App_instance *inst,
                                                 Romain::App_thread *thread,
                                                 Romain::Thread_group *tgroup)
{
	Romain::Stack st(sp);

	st.push(_am);
	st.push(tgroup);
	st.push(thread);
	st.push(inst);
	st.push(this);
	st.push(0); // this would be the return address, but
	            // handlers return by vcpu_resume()

	return st.sp();
}


void Romain::InstanceManager::create_instances()
{
	for (unsigned i = 0; i < _num_inst; ++i) {
		_instances.push_back(new Romain::App_instance(_name, i));
	}
}


Romain::App_thread*
Romain::InstanceManager::create_thread(l4_umword_t eip, l4_umword_t esp,
                                       unsigned instance_id, Romain::Thread_group *group)
{
		Romain::App_thread *at = new Romain::App_thread(eip, esp,
		                                          reinterpret_cast<l4_addr_t>(VCPU_handler),
		                                          reinterpret_cast<l4_addr_t>(VCPU_startup)
		);

		/*
		 * Set up the VCPU handler thread. It has been allocated in
		 * App_thread's constructor.
		 */
		DEBUG() << "prepare: " << (void*)at->handler_sp();
		at->handler_sp(prepare_stack(at->handler_sp(),
		                             _instances[instance_id], at, group));

		/*
		 * phys. CPU assignment, currently done by mapping instances to dedicated
		 * physical CPUs
		 */
		if (_num_cpu > 1) {
			INFO() << instance_id << " " << (instance_id+1) % _num_cpu << " " << _num_cpu;
			
			unsigned logCPU = 1;

			/* XXX REPLICAS PER CPU XXX */
			//logCPU = logicalToCPU(group->uid % _num_cpu);

			/* XXX INSTANCES PER CPU XXX */
			//logCPU = logicalToCPU((instance_id + 1) % _num_cpu);

			/* XXX OVERLAPPING REPLICAS XXX */
			//logCPU = logicalToCPU((group->uid + instance_id) % _num_cpu);

			/* XXX RANDOM PLACEMENT XXX */
			//logCPU = logicalToCPU(random() % _num_cpu);
			
			/* XXX Threads assigned RR to CPUs */
			static int threadcount = 0;
			logCPU = logicalToCPU(threadcount % _num_cpu);
			threadcount++;

			/* XXX The hard-coded placement map */
			//int cpumap[12] = {0, 1, 2,
			//                  0, 0, 0,
			//                  3, 4, 5,
			//                  0, 0, 0};
			//logCPU = logicalToCPU(cpumap[threadcount]);
			//threadcount++;

			at->cpu(logCPU);
		} else {
			at->cpu(0);
		}

		return at;
}


Romain::Thread_group *
Romain::InstanceManager::create_thread_group(l4_umword_t eip, l4_umword_t esp, std::string n,
                                             unsigned cap, unsigned uid)
{
	Romain::Thread_group *group = new Romain::Thread_group(n, cap, uid);
	group->set_redundancy_callback(new DMR(_num_inst));

	for (unsigned i = 0; i < _num_inst; ++i) {
		/*
		 * Thread creation
		 */
		Romain::App_thread *at = create_thread(eip, esp, i, group);
		group->add_replica(at);
	}

	_threadgroups.push_back(group);
	return group;
}


void Romain::InstanceManager::run_instances()
{
	Romain::Thread_group *group = create_thread_group(_init_eip, _init_esp, "init",
													  Romain::FIRST_REPLICA_CAP, 0);
	DEBUG() << "created group object @ " << (void*)group;
	theObjectFactory.register_thread_group(group, Romain::FIRST_REPLICA_CAP);

	_check(group->threads.size() != _num_inst, "not enough threads created?");

	for (unsigned i = 0; i < _num_inst; ++i) {

		App_thread *at = group->threads[i];

		/*
		 * Stack setup
		 */
		at->thread_sp((l4_addr_t)_am->stack()->relocate(_am->stack()->ptr()));

		/*
		 * The initial UTCB address is on top of the app's stack. This location
		 * is used for the first GDT entry, which L4Re later uses to find the
		 * thread's UTCB address.
		 */
		at->setup_utcb_segdesc(_am->stack()->target_top() - 4, 4);

		/*
		 * Establish UTCB mapping
		 */
		Romain::Region_handler &rh = const_cast<Romain::Region_handler&>(
		                                  _am->rm()->find(_am->prog_info()->utcbs_start)->second);
		_check(_am->rm()->copy_existing_mapping(rh, 0, i) != true,
		       "could not create UTCB copy");
		at->remote_utcb(rh.local_region(i).start());

		/*
		 * Notfiy handlers about an instance that has started
		 */
		startup_notify(_instances[i], at, group, _am);

		/*
		 * Start the thread itself
		 */
		at->vcpu()->r()->sp = at->thread_sp();
		at->start();
		at->commit_client_gdt();
	}

	group->activate();
}
