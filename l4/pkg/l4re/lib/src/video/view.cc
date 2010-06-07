/*
 * (c) 2008-2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 *
 * As a special exception, you may use this file as part of a free software
 * library without restriction.  Specifically, if other files instantiate
 * templates or use macros or inline functions from this file, or you compile
 * this file and link it with other files to produce an executable, this
 * file does not by itself cause the resulting executable to be covered by
 * the GNU General Public License.  This exception does not however
 * invalidate any other reasons why the executable file might be covered by
 * the GNU General Public License.
 */

#include <l4/re/video/goos>
#include <l4/re/video/goos-sys.h>

#include <l4/re/dataspace>
#include <l4/re/protocols>

#include <l4/cxx/ipc_helper>
#include <l4/cxx/ipc_stream>

#include <l4/sys/err.h>


namespace L4Re { namespace Video {

using L4::Opcode;

int
View::info(Info *info) const throw()
{
  L4::Ipc_iostream io(l4_utcb());
  io << Opcode(Goos_::View_info) << _view_idx;
  long err = l4_error(io.call(_goos.cap(), L4Re::Protocol::Goos));
  if (EXPECT_FALSE(err < 0))
    return err;

  io.get(*info);
  return err;
}

int
View::set_info(Info const &i) const throw()
{
  L4::Ipc_iostream io(l4_utcb());
  io << Opcode(Goos_::View_set_info) << _view_idx;
  io.put(i);
  return l4_error(io.call(_goos.cap(), L4Re::Protocol::Goos));
}

int
View::set_viewport(int scr_x, int scr_y, int w, int h,
                   unsigned long buf_offset) const throw()
{
  Info i;
  i.flags = F_set_buffer_offset | F_set_position;
  i.buffer_offset = buf_offset;
  i.xpos = scr_x;
  i.ypos = scr_y;
  i.width = w;
  i.height = h;
  return set_info(i);
}

int
View::stack(View const &pivot, bool behind) const throw()
{
  L4::Ipc_iostream io(l4_utcb());
  io << Opcode(Goos_::View_stack) << _view_idx << pivot.view_index() << behind;
  return l4_error(io.call(_goos.cap(), L4Re::Protocol::Goos));
}

int
View::refresh(int x, int y, int w, int h) const throw()
{
  L4::Ipc_iostream io(l4_utcb());
  io << Opcode(Goos_::View_refresh) << _view_idx;
  io << x << y << w << h;
  return l4_error(io.call(_goos.cap(), L4Re::Protocol::Goos));
}

}}
