/*
 * manager.cc --
 *
 *     Instance manager implementation.
 *
 * (c) 2011 Björn Döbel <doebel@os.inf.tu-dresden.de>,
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include "manager"
#include "app_loading"
#include "configuration"

#include <l4/sys/segment.h>

#define MSG() DEBUGf(Romain::Log::Manager)

Romain::Configuration Romain::globalconfig;

L4_INLINE unsigned countbits(long v)
{
	v = v - ((v >> 1) & 0x55555555);                         // reuse input as temporary
	v = (v & 0x33333333) + ((v >> 2) & 0x33333333);          // temp
	return ((v + ((v >> 4) & 0xF0F0F0F)) * 0x1010101) >> 24; // count
}

L4_INLINE l4_umword_t count_online_cpus()
{
	l4_umword_t ret;
	l4_sched_cpu_set_t set = l4_sched_cpu_set(0, 0);
	if (l4_error(L4Re::Env::env()->scheduler()->info(&ret, &set)) < 0) {
		ERROR() << "reading CPU info";
	}
	ret = countbits(set.map);

	INFO() << "Found " << ret << " CPUs.";
	return ret;
}

Romain::InstanceManager::InstanceManager(unsigned int argc,
                                         char const **argv,
                                         unsigned num_instances)
	: _am(0),
	  _instances(),
	  _callback(0),
	  _num_observers(0),
	  _num_inst(num_instances),
	  _num_cpu(1),
	  _argc(argc), // XXX: remove
	  _argv(argv)  // XXX: remove
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
	BoolObserverConfig("general:print_vcpu_state",
	                   this, "vcpu_state");
	ObserverConfig(this, "trap_limit");

	/*
	 * We always need the system call observer.
	 */
	ObserverConfig(this, "syscalls");
	ObserverConfig(this, "pagefaults");
	ObserverConfig(this, "trap");

	StringObserverConfig("general:debug", this);
	BoolObserverConfig("general:intercept_kip", this, "kip-time");
	BoolObserverConfig("general:swifi", this, "swifi");
}


void Romain::InstanceManager::configure_redundancy()
{
	char const *redundancy = ConfigStringValue("general:redundancy");
	if (!redundancy) redundancy = "none";
	INFO() << "red: '" << redundancy << "'";
	if (strcmp(redundancy, "none") == 0) {
		_num_inst = 1;
		set_redundancy_callback(new NoRed());
	} else if (strcmp(redundancy, "dual") == 0) {
		_num_inst = 2;
		set_redundancy_callback(new DMR(2));
	} else if (strcmp(redundancy, "triple") == 0) {
		_num_inst = 3;
		set_redundancy_callback(new DMR(3));
	} else {
		ERROR() << "Invalid redundancy setting: " << redundancy;
		enter_kdebug("Invalid redundancy setting");
	}
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
	char *log = strdup(ConfigStringValue("general:log", "none"));
	configure_logflags(log);
	
	Log::withtime = ConfigBoolValue("general:print_time", true);

	configure_fault_observers();
	configure_redundancy();
	free(log);
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
                                                 Romain::App_thread *thread)
{
	Romain::Stack st(sp);

	st.push(_am);
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


void Romain::InstanceManager::run_instances()
{
	for (unsigned i = 0; i < _num_inst; ++i) {
		Romain::App_thread *at = new Romain::App_thread(_init_eip, _init_esp,
		                                          reinterpret_cast<l4_addr_t>(VCPU_handler),
		                                          reinterpret_cast<l4_addr_t>(VCPU_startup)
		);

		at->setup_gdt(_am->stack()->target_top() - 4, 4);

		Romain::Region_handler &rh = const_cast<Romain::Region_handler&>(
		                                  _am->rm()->find(_am->prog_info()->utcbs_start)->second);
		_check(_am->rm()->copy_existing_mapping(rh, 0, i) != true,
		       "could not create UTCB copy");
		at->remote_utcb(rh.local_region(i).start());

		DEBUG() << "prepare: " << (void*)at->handler_sp();
		at->handler_sp(prepare_stack(at->handler_sp(), _instances[i], at));
		at->thread_sp((l4_addr_t)_am->stack()->relocate(_am->stack()->ptr()));
		if (_num_cpu > 1) {
			INFO() << i << " " << (i+1) % _num_cpu << " " << _num_cpu;
			at->cpu((i+1) % _num_cpu);
		} else {
			at->cpu(0);
		}

		startup_notify(_instances[i], at, _am);

		at->start();
	}
}
