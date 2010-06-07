INTERFACE:

#include "jdb.h"

class Jdb_kern_info_bench : public Jdb_kern_info_module
{
private:
  static Unsigned64 get_time_now();
  static void show_arch();
};

//---------------------------------------------------------------------------
IMPLEMENTATION:

static Jdb_kern_info_bench k_a INIT_PRIORITY(JDB_MODULE_INIT_PRIO+1);

PUBLIC
Jdb_kern_info_bench::Jdb_kern_info_bench()
  : Jdb_kern_info_module('b', "Benchmark privileged instructions")
{
  Jdb_kern_info::register_subcmd(this);
}

PUBLIC
void
Jdb_kern_info_bench::show()
{
  do_mp_benchmark();
  show_arch();
}


//---------------------------------------------------------------------------
IMPLEMENTATION [!mp || !jdb_ipi_bench]:

PRIVATE
void
Jdb_kern_info_bench::do_mp_benchmark()
{}

//---------------------------------------------------------------------------
IMPLEMENTATION [mp && jdb_ipi_bench]:

#include "ipi.h"

static int volatile ipi_bench_spin_done;

PRIVATE static
void
Jdb_kern_info_bench::wait_for_ipi(unsigned, void *)
{
  Proc::sti();
  while (!ipi_bench_spin_done)
    Proc::pause();
  Proc::cli();
}

PRIVATE static
void
Jdb_kern_info_bench::empty_func(void *)
{}

PRIVATE static
void
Jdb_kern_info_bench::do_ipi_bench(unsigned my_cpu, void *_partner)
{
  Unsigned64 time;
  unsigned partner = (unsigned long)_partner;
  const int runs2 = 3;
  unsigned i;

  Ipi::remote_call_wait(my_cpu, partner, empty_func, 0);

  time = get_time_now();
  for (i = 0; i < (1 << runs2); i++)
    Ipi::remote_call_wait(my_cpu, partner, empty_func, 0);

  printf(" %2u:%4lld", partner, (get_time_now() - time) >> runs2);

  ipi_bench_spin_done = 1;
  asm volatile ("" ::: "memory");
}

PRIVATE
void
Jdb_kern_info_bench::do_mp_benchmark()
{
  // IPI bench matrix
  printf("IPI latency:\n");
  for (unsigned u = 0; u < Config::Max_num_cpus; ++u)
    if (Cpu::online(u))
      {
        printf("l%2u(p%2u): ", u, Cpu::cpus.cpu(u).phys_id() >> 24);

	for (unsigned v = 0; v < Config::Max_num_cpus; ++v)
	  if (Cpu::online(v))
	    {
	      if (u == v)
		printf(" %2u: X  ", u);
	      else
		{
		  ipi_bench_spin_done = 0;

		  // v is waiting for IPIs
		  if (v != 0)
		    Jdb::remote_work(v, wait_for_ipi, 0, false);

		  // u is doing benchmark
		  if (u == 0)
		    do_ipi_bench(0, (void *)v);
		  else
                    Jdb::remote_work(u, do_ipi_bench, (void *)v, false);

		  // v is waiting for IPIs
		  if (v == 0)
		    wait_for_ipi(0, 0);

		  asm volatile ("" ::: "memory");

		  while (!ipi_bench_spin_done)
		    Proc::pause();

		}
	    }
	printf("\n");
      }
}
