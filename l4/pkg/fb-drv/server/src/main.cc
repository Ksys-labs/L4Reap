/*
 * (c) 2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *          Alexander Warg <warg@os.inf.tu-dresden.de>,
 *          Torsten Frenzel <frenzel@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include <l4/sys/capability>
#include <l4/sys/typeinfo_svr>
#include <l4/cxx/ipc_server>

#include <cstdio>
#include <getopt.h>
#include <cstdlib>

#include "fb.h"

L4Re::Util::Object_registry registry;
static L4::Server<> server(l4_utcb());

void
Phys_fb::setup_ds(char const *name)
{
  registry.register_obj(this, name);
  _fb_ds = L4::Cap<L4Re::Dataspace>(obj_cap().cap());
  _ds_start = _vidmem_start;
  _ds_size = _vidmem_size;
  _rw_flags = Writable;
  _cache_flags = L4::Ipc::Gen_fpage<L4::Ipc::Snd_item>::Uncached;
}

int
Phys_fb::map_hook(l4_addr_t offs, unsigned long flags,
                  l4_addr_t min, l4_addr_t max)
{
  // map everything at once, a framebuffer will usually used fully
  (void)offs; (void)flags; (void)min; (void)max;
  if (_map_done)
    return 0;

  int err;
  L4::Cap<L4Re::Dataspace> ds;
  unsigned long sz = 1;
  l4_addr_t off;
  unsigned fl;
  l4_addr_t a = _vidmem_start;

  if ((err = L4Re::Env::env()->rm()->find(&a, &sz, &off, &fl, &ds)) < 0)
    {
      printf("Failed to query video memory: %d\n", err);
      return err;
    }

  if ((err = ds->map_region(off, L4Re::Dataspace::Map_rw,
                            _vidmem_start, _vidmem_end)) < 0)
    {
      printf("Failed to map video memory: %d\n", err);
      return err;
    }

  _map_done = 1;
  return 0;
}

int
Phys_fb::dispatch(l4_umword_t obj, L4::Ipc::Iostream &ios)
{
  l4_msgtag_t tag;
  ios >> tag;
  switch (tag.label())
    {
    case L4::Meta::Protocol:
      return L4::Util::handle_meta_request<L4Re::Video::Goos>(ios);
    case L4Re::Protocol::Goos:
      return L4Re::Util::Video::Goos_svr::dispatch(obj, ios);
    case L4Re::Protocol::Dataspace:
      return L4Re::Util::Dataspace_svr::dispatch(obj, ios);
    default:
      return -L4_EBADPROTO;
    }
}


Prog_args::Prog_args(int argc, char *argv[])
 : vbemode(~0), do_dummy(false), config_str(0)
{
  while (1)
    {
      struct option opts[] = {
            { "vbemode", required_argument, 0, 'm' },
            { "config", required_argument, 0, 'c' },
            { "dummy", no_argument, 0, 'D' },
            { 0, 0, 0, 0 },
      };

      int c = getopt_long(argc, argv, "c:m:D", opts, NULL);
      if (c == -1)
        break;

      switch (c)
        {
        case 'c':
          config_str = optarg;
          break;
        case 'm':
          vbemode = strtol(optarg, 0, 0);
          break;
        case 'D':
          do_dummy = true;
          break;
        default:
          printf("Unknown option '%c'\n", c);
          break;
        }
    }
}


int main(int argc, char *argv[])
{
#ifdef ARCH_arm
  Lcd_drv_fb arch_fb;
#elif defined(ARCH_x86) || defined(ARCH_amd64)
  Vesa_fb arch_fb;
#else
  Dummy_fb arch_fb;
#endif
  Dummy_fb dummy_fb;
  Phys_fb *fb = &dummy_fb;

  Prog_args args(argc, argv);

  if (!fb->setup_drv(&args))
    {
      fb = &arch_fb;
      if (!fb->setup_drv(&args))
        {
          printf("Failed to setup Framebuffer\n");
          return 1;
        }
    }
  fb->setup_ds("fb");

  if (!fb->running())
    {
      printf("Failed to initialize frame buffer; Aborting.\n");
      return 1;
    }

  if (!fb->obj_cap().is_valid())
    {
      printf("Failed to connect.\n");
      return 1;
    }

  printf("Starting server loop\n");
  server.loop(registry);

  return 0;
}
