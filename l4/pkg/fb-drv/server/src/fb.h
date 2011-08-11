/*
 * (c) 2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *          Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include <l4/re/util/video/goos_svr>
#include <l4/re/util/object_registry>
#include <l4/re/util/dataspace_svr>

class Prog_args
{
public:
  Prog_args(int argc, char *argv[]);

  int vbemode;
  bool do_dummy;
  char *config_str;
};


class Phys_fb : public L4Re::Util::Video::Goos_svr,
                public L4Re::Util::Dataspace_svr,
		public L4::Server_object
{
public:
  Phys_fb() : _vidmem_start(0), _map_done(0) {}

  ~Phys_fb() throw() {}
  virtual bool setup_drv(Prog_args *pa) = 0;
  void setup_ds(char const *name);
  int dispatch(l4_umword_t obj, L4::Ipc::Iostream &ios);
  int map_hook(l4_addr_t offs, unsigned long flags,
               l4_addr_t min, l4_addr_t max);

  bool running() { return _vidmem_start; };

protected:
  l4_addr_t _vidmem_start;
  l4_addr_t _vidmem_end;
  l4_addr_t _vidmem_size;

private:
  bool _map_done;
};


class Vesa_fb : public Phys_fb
{
public:
  bool setup_drv(Prog_args *pa);
};


class Lcd_drv_fb : public Phys_fb
{
public:
  bool setup_drv(Prog_args *pa);
};

class Dummy_fb : public Phys_fb
{
public:
  bool setup_drv(Prog_args *pa);
};
