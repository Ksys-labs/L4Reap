/*
 * (c) 2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *          Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include <l4/x86emu/int10.h>
#include <l4/io/io.h>
#include <cstdio>

#include "fb.h"
#include "splash.h"

bool
Vesa_fb::setup_drv(Prog_args *pa)
{
  l4util_mb_vbe_ctrl_t *vbe;
  l4util_mb_vbe_mode_t *vbi;

  if (x86emu_int10_set_vbemode(pa->vbemode, &vbe, &vbi))
    return false;

  _vidmem_size = 64*1024*vbe->total_memory;

  _vidmem_start = 0;
  int error;
  if ((error = l4io_request_iomem(vbi->phys_base, _vidmem_size,
                                  0, &_vidmem_start)) < 0)
    printf("map of gfx mem failed\n");

  _vidmem_end   = _vidmem_start + _vidmem_size;

  _screen_info.width = vbi->x_resolution;
  _screen_info.height = vbi->y_resolution;
  _screen_info.flags = L4Re::Video::Goos::F_auto_refresh;
  _screen_info.pixel_info = L4Re::Video::Pixel_info(vbi);

  _view_info.buffer_offset = 0; //base_offset;
  _view_info.bytes_per_line = vbi->bytes_per_scanline;

  init_infos();

  printf("Framebuffer memory: phys: %x - %lx\n",
         vbi->phys_base, vbi->phys_base + _vidmem_size);
  printf("                    virt: %lx - %lx\n",
         _vidmem_start, _vidmem_start + _vidmem_size);

  x86emu_int10_done();

  splash_display(&_view_info, _vidmem_start);

  return true;
}
