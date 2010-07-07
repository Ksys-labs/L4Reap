/*
 * (c) 2010 Alexander Warg <warg@os.inf.tu-dresden.de>
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

#include <l4/re/env>
#include <l4/sys/factory>

#include "vcon_stream.h"
#include "vfs_api.h"

namespace L4Re { namespace Core {
Vcon_stream::Vcon_stream(L4::Cap<L4::Vcon> s) throw()
: Be_file(), _s(s)
{
#if 1
  //printf("VCON: create IRQ\n");
  _irq = cap_alloc()->alloc<L4::Irq>();
  //printf("VCON: irq cap = %lx\n", _irq.cap());
  int res = l4_error(L4Re::Env::env()->factory()->create_irq(_irq));
  //printf("VCON: irq create res=%d\n", res);

  if (res < 0)
    return; // handle errors!!!

  res = l4_error(L4::cap_reinterpret_cast<L4::Icu>(_s)->bind(0, _irq));
  //printf("VCON: bound irq to con res=%d\n", res);
#endif
}

ssize_t
Vcon_stream::readv(const struct iovec *iovec, int iovcnt) throw()
{
  (void) iovec;
  (void) iovcnt;
#if 0
  ssize_t bytes = 0;
  for (; iovcnt > 0; --iovcnt, ++iovec)
    {
      if (iovec->iov_len == 0)
	continue;

      char *buf = (char *)iovec->iov_base;
      size_t len = iovec->iov_len;

      while (1)
	{
	  int ret = _s->read(buf, len);

	  // BS: what is this ??
	  if (ret > (int)len)
	    ret = len;

	  if (ret < 0)
	    return ret;
	  else if (ret == 0)
	    {
	      if (bytes)
		return bytes;

	      _irq->detach();
	      _irq->attach(12, L4_IRQ_F_NONE, L4::Cap<L4::Thread>::Invalid);
	      ret = _s->read(buf, len);
	      if (ret < 0)
		return ret;
	      else if (ret == 0)
		{
		  _irq->receive();
		  continue;
		}
	    }

	  bytes += ret;
	  len   -= ret;
	  buf   += ret;

	  if (len == 0)
	    break;
	}
    }

  return bytes;
#endif
  return 0;
}

ssize_t
Vcon_stream::writev(const struct iovec *iovec, int iovcnt) throw()
{
  l4_msg_regs_t store;
  l4_msg_regs_t *mr = l4_utcb_mr();

  Vfs_config::memcpy(&store, mr, sizeof(store));

  ssize_t written = 0;
  while (iovcnt)
    {
      size_t sl = iovec->iov_len;
      char const *b = (char const *)iovec->iov_base;

      for (; sl > L4_VCON_WRITE_SIZE
           ; sl -= L4_VCON_WRITE_SIZE, b += L4_VCON_WRITE_SIZE)
        _s->write(b, L4_VCON_WRITE_SIZE);

      _s->write(b, sl);

      written += iovec->iov_len;

      ++iovec;
      --iovcnt;
    }
  Vfs_config::memcpy(mr, &store, sizeof(store));
  return written;
}

int
Vcon_stream::fstat64(struct stat64 *buf) const throw()
{
  buf->st_size = 0;
  buf->st_mode = 0666;
  buf->st_dev = _s.cap();
  buf->st_ino = 0;
  return 0;
}

}}
