/*
 * (c) 2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include <sys/mman.h>
#include <cstdio>
#include <getopt.h>
#include <cstdlib>

#include "fb.h"

bool
Dummy_fb::setup_drv(Prog_args *pa)
{
  if (!pa->do_dummy)
    return false;

  _screen_info.width      = 1024;
  _screen_info.height     = 768;
  _screen_info.flags      = L4Re::Video::Goos::F_auto_refresh;
  _screen_info.pixel_info = L4Re::Video::Pixel_info(2, 5, 11, 6, 5, 5, 0);

  _vidmem_size = _screen_info.width * _screen_info.height
                 * _screen_info.pixel_info.bytes_per_pixel();
  _vidmem_size = l4_round_page(_vidmem_size);

  void *v = mmap(0, _vidmem_size, PROT_WRITE,
                 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  if (v == MAP_FAILED)
    return false;

  _vidmem_start = (unsigned long)v;
  _vidmem_end   = _vidmem_start + _vidmem_size;

  _view_info.buffer_offset = 0;
  _view_info.bytes_per_line
     = _screen_info.width * _screen_info.pixel_info.bytes_per_pixel();

  init_infos();

  return true;
}
