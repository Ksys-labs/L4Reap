/*
 * (c) 2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *          Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <l4/sys/capability>
#include <l4/re/env>
#include <l4/re/util/cap_alloc>
#include <l4/re/namespace>
#include <l4/sys/icu>
#include <l4/re/error_helper>
#include <l4/re/dataspace>
#include <l4/util/kip.h>
#include <l4/cxx/iostream>

#include "main.h"
#include "hw_root_bus.h"
#include "hw_device.h"
#include "server.h"
#include "res.h"
#include "pci.h"
#include "__acpi.h"
#include "vbus.h"
#include "vbus_factory.h"
#include "phys_space.h"
#include "ux.h"
#include "cfg.h"

#include <cstdio>
#include <typeinfo>
#include <algorithm>
#include <string>
#include <getopt.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>

#include "cfg_parser.h"

using L4Re::Util::Auto_cap;

class Io_config_x : public Io_config
{
public:
  Io_config_x() : _do_transparent_msi(false) {}

  bool transparent_msi(Hw::Device *) const
  { return _do_transparent_msi; }

  bool legacy_ide_resources(Hw::Device *) const
  { return true; }

  bool expansion_rom(Hw::Device *) const
  { return false; }

  void set_transparent_msi(bool v) { _do_transparent_msi = v; }

  int verbose() const { return _verbose_lvl; }
  void inc_verbosity() { ++_verbose_lvl; }

private:
  bool _do_transparent_msi;
  int _verbose_lvl;
};

static Io_config_x _my_cfg __attribute__((init_priority(30000)));
Io_config *Io_config::cfg = &_my_cfg;


Hw::Device *
system_bus()
{
  static Hw::Root_bus _sb("System Bus");
  return &_sb;
}

Hw_icu *
system_icu()
{
  static Hw_icu _icu;
  return &_icu;
}

template< typename X >
char const *type_name(X const &x)
{
  char *n = const_cast<char *>(typeid(x).name());
  strtol(n, &n, 0);
  return n;
}

void dump(Device *d)
{
  Device::iterator i = Device::iterator(0, d, 100);
  for (; i != d->end(); ++i)
    {
      for (int x = 0; x < i->depth(); ++x)
	printf("  ");
      printf("%s: %s \"%s\"\n",
             type_name(*i), i->name(),
             i->hid() ? i->hid() : "");
      if (dlevel(DBG_INFO))
        i->dump(i->depth() * 2);
      if (dlevel(DBG_DEBUG))
        for (Resource_list::iterator r = i->resources()->begin();
             r != i->resources()->end(); ++r)
          if (*r)
            r->dump(i->depth() * 2 + 2);
    }
}


Hw_icu::Hw_icu()
{
  icu = L4Re::Env::env()->get_cap<L4::Icu>("icu");
  if (icu.is_valid())
    icu->info(&info);
}



struct Add_system_bus
{
  void operator () (Vi::Device *dev)
  {
    Vi::System_bus *b = dynamic_cast<Vi::System_bus*>(dev);
    if (!b)
      {
        d_printf(DBG_ERR, "ERROR: found non system-bus device as root device, ignored\n");
	return;
      }

    b->request_child_resources();
    b->allocate_pending_child_resources();
    b->setup_resources();
    if (!registry->register_obj(b, b->name()).is_valid())
      {
	d_printf(DBG_WARN, "WARNING: Service registration failed: '%s'\n", b->name());
	return;
      }
  }
};

static void
read_config(char const *cfg_file)
{
  Vi::Device *vdev = 0;

  d_printf(DBG_INFO, "Loading: config '%s'\n", cfg_file);


  int fd = open(cfg_file, O_RDONLY);

  if (fd < 0)
    {
      d_printf(DBG_ERR, "ERROR: failed to open config file '%s'\n", cfg_file);
      exit(1);
    }

  struct stat sd;
  if (fstat(fd, &sd))
    {
      d_printf(DBG_ERR, "ERROR: failed to stat config file '%s'\n", cfg_file);
      exit(1);
    }

  void *adr = mmap(NULL, sd.st_size, PROT_READ, MAP_SHARED, fd, 0);
  if (adr == MAP_FAILED)
    {
      d_printf(DBG_ERR, "ERROR: failed to mmap config file '%s'\n", cfg_file);
      exit(1);
    }

  char const *config_file = (char const *)adr;

  cfg::Scanner s(config_file, config_file + sd.st_size, cfg_file);
  cfg::Parser p(&s, 0, vdev, system_bus());
  p.parse();

  munmap(adr, sd.st_size);

  if (!vdev)
    return;

  std::for_each(Vi::Device::iterator(0, vdev, 0), Vi::Device::end(), Add_system_bus());

  if (dlevel(DBG_DEBUG))
    dump(vdev);
}



static int
arg_init(int argc, char * const *argv, Io_config_x *cfg)
{
  while (1)
    {
      int optidx = 0;
      int c;

      struct option opts[] =
      {
        { "verbose",         0, 0, 'v' },
        { "transparent-msi", 0, 0, 1 },
        { 0, 0, 0, 0 },
      };

      c = getopt_long(argc, argv, "v", opts, &optidx);
      if (c == -1)
        break;

      switch (c)
        {
        case 'v':
          _my_cfg.inc_verbosity();
          break;
        case 1:
	  d_printf(DBG_INFO, "Enabling transparent MSIs\n");
          cfg->set_transparent_msi(true);
          break;
        }
    }
  return optind;
}

static
int
run(int argc, char * const *argv)
{
  int argfileidx = arg_init(argc, argv, &_my_cfg);

  printf("Io service\n");
  set_debug_level(Io_config::cfg->verbose());

  d_printf(DBG_ERR, "Verboseness level: %d\n", Io_config::cfg->verbose());

  res_init();

  if (dlevel(DBG_DEBUG))
    Phys_space::space.dump();

#if defined(ARCH_x86) || defined(ARCH_amd64)
  bool is_ux = l4util_kip_kernel_is_ux(l4re_kip());
  //res_get_ioport(0xcf8, 4);
  res_get_ioport(0, 16);

  if (!is_ux)
    acpica_init();
#endif

  system_bus()->plugin();

#if defined(ARCH_x86) || defined(ARCH_amd64)
  if (is_ux)
    ux_setup(system_bus());
#endif

  for (; argfileidx < argc; ++argfileidx)
    read_config(argv[argfileidx]);

  if (dlevel(DBG_DEBUG))
    {
      printf("Real Hardware -----------------------------------\n");
      dump(system_bus());
    }

  fprintf(stderr, "Ready. Waiting for request.\n");
  server_loop();

  return 0;
}

int
main(int argc, char * const *argv)
{
  try
    {
      return run(argc, argv);
    }
  catch (L4::Runtime_error &e)
    {
      L4::cerr << "FATAL uncought exception: " << e
               << "\nterminating...\n";

    }
  catch (...)
    {
      L4::cerr << "FATAL uncought exception of unknown type\n"
               << "terminating...\n";
    }
  return -1;
}
