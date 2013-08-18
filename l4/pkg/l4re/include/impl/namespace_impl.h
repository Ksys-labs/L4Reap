/**
 * \file
 * \brief  Namespace client stub implementation
 */
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
#include <l4/re/namespace>
#include <l4/re/namespace-sys.h>
#include <l4/re/protocols>

#include <l4/util/util.h>

#include <l4/cxx/exceptions>
#include <l4/cxx/ipc_helper>
#include <l4/cxx/ipc_stream>

#include <l4/sys/err.h>

#include <cstring>

namespace L4Re {

long
Namespace::_query(char const *name, unsigned len,
                  L4::Cap<void> const &target,
                  l4_umword_t *local_id, bool iterate) const throw()
{
  unsigned long res = len;
  char const *n = name;
  L4::Cap<Namespace> ns(cap());
  while (res > 0)
    {
      L4::Ipc::Iostream io(l4_utcb());
      io << L4::Opcode(L4Re::Namespace_::Query)
         << L4::Ipc::Buf_cp_out<const char>(n, res);
      io << L4::Ipc::Small_buf(target.cap(), local_id ? L4_RCV_ITEM_LOCAL_ID : 0);
      l4_msgtag_t r = io.call(ns.cap(), L4Re::Protocol::Namespace);
      long err = l4_error(r);

      if (err < 0)
	return err;

      bool const partly = err & Partly_resolved;
      if (partly)
	{
	  l4_umword_t dummy;
	  io >> dummy >> L4::Ipc::Buf_in<char const>(n, res);
	}

      L4::Ipc::Snd_fpage cap;
      io >> cap;
      if (cap.id_received())
	{
	  *local_id = cap.data();
	  return res;
	}

      if (partly && iterate)
	ns = L4::cap_cast<Namespace>(target);
      else
	return err;
    }

  return res;
}

long
Namespace::query(char const *name, unsigned len, L4::Cap<void> const &target,
                 int timeout, l4_umword_t *local_id, bool iterate) const throw()
{
  long ret;
  long rem = timeout;
  long to = 0;

  if (rem)
    to = 10;
  do
    {
      ret = _query(name, len, target, local_id, iterate);

      if (ret >= 0)
	return ret;

      if (EXPECT_FALSE(ret < 0 && (ret != -L4_EAGAIN)))
	return ret;

      if (rem == to)
	return ret;

      l4_sleep(to);

      rem -= to;
      if (to < 100)
	to += to;
      if (to > rem)
	to = rem;
    }
  while (486);
}

long
Namespace::query(char const *name, L4::Cap<void> const &target,
                 int timeout, l4_umword_t *local_id,
                 bool iterate) const throw()
{ return query(name, strlen(name), target, timeout, local_id, iterate); }

long
Namespace::register_obj(char const *name, L4::Cap<void> const &o,
                        unsigned flags) const throw()
{
  L4::Ipc::Iostream io(l4_utcb());
  io << L4::Opcode(L4Re::Namespace_::Register)
     << flags << L4::Ipc::Buf_cp_out<char const>(name, strlen(name));
  if (o.is_valid())
    io << L4::Ipc::Snd_fpage(o, L4_FPAGE_RWX & flags) ;
  l4_msgtag_t res = io.call(cap(), L4Re::Protocol::Namespace);
  return l4_error(res);
}

long
Namespace::unlink(char const *name) throw()
{
  L4::Ipc::Iostream io(l4_utcb());
  io << L4::Opcode(L4Re::Namespace_::Unlink)
     << L4::Ipc::Buf_cp_out<char const>(name, strlen(name));
  l4_msgtag_t res = io.call(cap(), L4Re::Protocol::Namespace);
  return l4_error(res);
}

long
Namespace::link(char const *name, unsigned len,
                L4::Cap<L4Re::Namespace> src_dir,
                char const *src_name, unsigned src_len,
                unsigned flags) throw()
{
  L4::Ipc::Iostream io(l4_utcb());
  io << L4::Opcode(L4Re::Namespace_::Link)
     << flags << L4::Ipc::Buf_cp_out<char const>(name, len)
     << L4::Ipc::Buf_cp_out<char const>(src_name, src_len)
     << L4::Ipc::Snd_fpage(src_dir.fpage());
  l4_msgtag_t res = io.call(cap(), L4Re::Protocol::Namespace);
  return l4_error(res);
}

};
