/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>,
 *               Torsten Frenzel <frenzel@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <l4/re/log>
#include <l4/re/log-sys.h>
#include <l4/re/protocols>
#include <l4/sys/kdebug.h>
#include <l4/cxx/minmax>

#include "log.h"
#include "global.h"
#include "obj_reg.h"

#include <cstdio>
#include <unistd.h>

static Ldr::Log *last_log = 0;

static void my_outnstring(char const *s, unsigned long len)
{
  write(1, s, len);
}

static void mycpy(char **buf, int *rem, char const *s, int l)
{
  int o = cxx::min(*rem, l);
  memcpy(*buf, s, o);
  *rem -= o;
  *buf += o;
}

static char msgbuf[4096];

int
Ldr::Log::dispatch(l4_umword_t, L4::Ipc_iostream &ios)
{
  l4_msgtag_t tag;
  ios >> tag;

  if (tag.label() != L4_PROTO_LOG)
    return -L4_EBADPROTO;

  L4::Opcode op;

  // get opcode out of the message stream
  ios >> op;

  // we only have one opcode
  if (op != L4Re::Log_::Print)
    return -L4_ENOSYS;

  char *msg = Glbl::log_buffer;
  unsigned long len_msg = sizeof(Glbl::log_buffer);

  ios >> L4::ipc_buf_cp_in(msg, len_msg);

  int rem = sizeof(msgbuf);
  while (len_msg > 0 && msg[0])
    {
      char *obuf = msgbuf;
#if 1
      if (color())
	{
	  int n = snprintf(obuf, rem, "\033[%s3%dm",
	      (color() & 8) ? "01;" : "", (color() & 7));
	  obuf = obuf + n;
	  rem -= n;
	}
      else
	mycpy(&obuf, &rem, "\033[0m", 4);
#endif
      if (last_log != this)
	{
	  if (last_log != 0)
	    my_outnstring("\n", 1);

	  mycpy(&obuf, &rem, _tag, cxx::min<unsigned long>(_l, Max_tag));
	  if (_l < Max_tag)
	    mycpy(&obuf, &rem, "             ", Max_tag-_l);

	  if (_in_line)
	    mycpy(&obuf, &rem, ": ", 2);
	  else
	    mycpy(&obuf, &rem, "| ", 2);
	}

      unsigned long i;
      for (i = 0; i < len_msg; ++i)
	if (msg[i] == '\n' || msg[i] == 0)
	  break;

      mycpy(&obuf, &rem, msg, i);

      if (i <len_msg && msg[i] == '\n')
	{
          if (color())
            mycpy(&obuf, &rem, "\033[0m", 4);
	  mycpy(&obuf, &rem, "\n", 1);
	  _in_line = false;
	  last_log = 0;
	  ++i;
	}
      else
	{
	  last_log = this;
	  _in_line = true;
	}
      my_outnstring(msgbuf, obuf-msgbuf);

      msg += i;
      len_msg -= i;
    }

  if (_in_line && color())
    my_outnstring("\033[0m", 4);

  // and finally done
  return -L4_ENOREPLY;
}


int
Ldr::Log::color_value(cxx::String const &col)
{
  int c = 0, bright = 0;

  if (col.empty())
    return 0;

  switch(col[0])
    {
    case 'N': bright = 1; case 'n': c = 0; break;
    case 'R': bright = 1; case 'r': c = 1; break;
    case 'G': bright = 1; case 'g': c = 2; break;
    case 'Y': bright = 1; case 'y': c = 3; break;
    case 'B': bright = 1; case 'b': c = 4; break;
    case 'M': bright = 1; case 'm': c = 5; break;
    case 'C': bright = 1; case 'c': c = 6; break;
    case 'W': bright = 1; case 'w': c = 7; break;
    default: c = 0;
    }

  return (bright << 3) | c;
}

