/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
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
Goos::info(Info *info) const throw()
{
  L4::Ipc::Iostream io(l4_utcb());
  io << Opcode(Goos_::Info);
  long err = l4_error(io.call(cap(), L4Re::Protocol::Goos));
  if (EXPECT_FALSE(err < 0))
    return err;

  io.get(*info);
  return err;
}

int
Goos::get_static_buffer(unsigned idx, L4::Cap<Dataspace> ds) const throw()
{
  L4::Ipc::Iostream io(l4_utcb());
  io << Opcode(Goos_::Get_buffer) << idx;
  io << L4::Ipc::Small_buf(ds.cap());
  return l4_error(io.call(cap(), L4Re::Protocol::Goos));
}

int
Goos::create_buffer(unsigned long size, L4::Cap<Dataspace> ds) const throw()
{
  L4::Ipc::Iostream io(l4_utcb());
  io << Opcode(Goos_::Create_buffer) << size;
  io << L4::Ipc::Small_buf(ds.cap());
  return l4_error(io.call(cap(), L4Re::Protocol::Goos));
}

int
Goos::delete_buffer(unsigned idx) const throw()
{
  L4::Ipc::Iostream io(l4_utcb());
  io << Opcode(Goos_::Delete_buffer) << idx;
  return l4_error(io.call(cap(), L4Re::Protocol::Goos));
}

int
Goos::create_view(View *view) const throw()
{
  L4::Ipc::Iostream io(l4_utcb());
  io << Opcode(Goos_::Create_view);
  int err = l4_error(io.call(cap(), L4Re::Protocol::Goos));
  if (err < 0)
    return err;

  *view = View(cap(), err);
  return err;
}

int
Goos::delete_view(View const &v) const throw()
{
  L4::Ipc::Iostream io(l4_utcb());
  io << Opcode(Goos_::Delete_view) << v._view_idx;
  return l4_error(io.call(cap(), L4Re::Protocol::Goos));
}

int
Goos::refresh(int x, int y, int w, int h) throw()
{
  L4::Ipc::Iostream io(l4_utcb());
  io << Opcode(Goos_::Screen_refresh) << x << y << w << h;
  return l4_error(io.call(cap(), L4Re::Protocol::Goos));
}

}}
