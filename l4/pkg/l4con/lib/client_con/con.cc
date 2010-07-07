/**
 * \file
 * \brief  L4 Console
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Torsten Frenzel <frenzel@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <l4/l4con/l4con.h>
#include <l4/cxx/exceptions>
#include <l4/cxx/ipc_helper>
#include <l4/cxx/ipc_stream>

#include <l4/sys/err.h>

long
L4con::close() const throw()
{
  L4::Ipc_iostream io(l4_utcb());
  io << L4::Opcode(L4con_::Close);
  return l4_error(io.call(Framebuffer::cap(), L4con::Protocol::Vc));
}

long
L4con::pslim_fill(int x, int y, int w, int h, unsigned int color) const throw()
{
  L4::Ipc_iostream io(l4_utcb());
  io << L4::Opcode(L4con_::Pslim_fill);
  io << x << y << w << h << color;
  return l4_error(io.call(Framebuffer::cap(), L4con::Protocol::Vc));
}

long
L4con::pslim_copy(int x, int y, int w, int h, l4_int16_t dx, l4_int16_t dy) const throw()
{
  L4::Ipc_iostream io(l4_utcb());
  io << L4::Opcode(L4con_::Pslim_copy);
  io << x << y << w << h << dx << dy;
  return l4_error(io.call(Framebuffer::cap(), L4con::Protocol::Vc));
}

long
L4con::puts(const char *s, unsigned long len, short x, short y,
            unsigned int fg_color, unsigned int bg_color) const throw()
{
  L4::Ipc_iostream io(l4_utcb());
  io << L4::Opcode(L4con_::Puts);
  io << x << y << fg_color << bg_color
     << L4::ipc_buf_cp_out(s, len);
  return l4_error(io.call(Framebuffer::cap(), L4con::Protocol::Vc));
}

long
L4con::puts_scale(const char *s, unsigned long len,
                  short x, short y,
                  unsigned int fg_color, unsigned int bg_color,
                  short scale_x, short scale_y) const throw()
{
  L4::Ipc_iostream io(l4_utcb());
  io << L4::Opcode(L4con_::Puts_scale);
  io << x << y << fg_color << bg_color << scale_x << scale_y
     << L4::ipc_buf_cp_out(s, len);
  return l4_error(io.call(Framebuffer::cap(), L4con::Protocol::Vc));
}

long
L4con::get_font_size(unsigned int *fn_w, unsigned int *fn_h) const throw()
{
  L4::Ipc_iostream io(l4_utcb());
  io << L4::Opcode(L4con_::Get_font_size);
  long r = l4_error(io.call(Framebuffer::cap(), L4con::Protocol::Vc));
  if (EXPECT_FALSE(r < 0))
    return r;

  io >> *fn_w >> *fn_h;
  return L4_EOK;
}







/* C part */

L4_CV long
l4con_close(l4re_fb_t fb) L4_NOTHROW
{
  L4::Cap<L4con> f(fb);
  return f->close();
}

L4_CV long
l4con_pslim_fill(l4re_fb_t fb, int x, int y, int w, int h, unsigned int color) L4_NOTHROW
{
  L4::Cap<L4con> f(fb);
  return f->pslim_fill(x, y, w, h, color);
}

L4_CV long
l4con_puts(l4re_fb_t fb, const char *s, unsigned long len, short x, short y,
             unsigned int fg_color, unsigned int bg_color) L4_NOTHROW
{
  L4::Cap<L4con> f(fb);
  return f->puts(s, len, x, y, fg_color, bg_color);
}

L4_CV long
l4con_puts_scale(l4re_fb_t fb, const char *s, unsigned long len,
                 short x, short y,
                 unsigned int fg_color, unsigned int bg_color,
                 short scale_x, short scale_y) L4_NOTHROW
{
  L4::Cap<L4con> f(fb);
  return f->puts_scale(s, len, x, y, fg_color, bg_color, scale_x, scale_y);
}

L4_CV long
l4con_get_font_size(l4re_fb_t fb, unsigned int *fn_w, unsigned int *fn_h) L4_NOTHROW
{
  L4::Cap<L4con> f(fb);
  return f->get_font_size(fn_w, fn_h);
}

