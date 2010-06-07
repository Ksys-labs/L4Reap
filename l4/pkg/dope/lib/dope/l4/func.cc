/*** GENERAL INCLUDES ***/
#include <stdio.h>
#include <stdarg.h>

/*** LOCAL INCLUDES ***/
#include "dopelib.h"
#include "dopestd.h"
#include "sync.h"
#include "init.h"

#include <l4/dope/dopedef.h>
#include <l4/dope/vscreen.h>
#include <l4/re/dataspace>
#include <l4/re/util/cap_alloc>
#include <l4/re/env>

#include <l4/cxx/ipc_stream>

extern l4_cap_idx_t dope_server;

int dope_req(char *res, unsigned long res_max, const char *cmd)
{
  if (!cmd || l4_is_invalid_cap(dope_server))
    return -1;

  L4::Ipc_iostream io(l4_utcb());
  io << L4::Opcode(Dope::Dope_app_::Cmd_req)
     << L4::ipc_buf_cp_out(cmd, strlen(cmd));

  l4_msgtag_t r = io.call(dope_server, Dope::Protocol::App);

  io >> L4::ipc_buf_cp_in(res, res_max);

  res[res_max] = 0;
  return l4_error(r);
}


int dope_reqf(char *res, unsigned res_max, const char *format, ...)
{
  int ret;
  va_list list;
  static char cmdstr[1024];

  dopelib_mutex_lock(dopelib_cmdf_mutex);
  va_start(list, format);
  vsnprintf(cmdstr, 1024, format, list);
  va_end(list);
  ret = dope_req(res, res_max, cmdstr);
  dopelib_mutex_unlock(dopelib_cmdf_mutex);

  return ret;
}

int dope_cmd(const char *cmd)
{
  if (!cmd || l4_is_invalid_cap(dope_server))
    return -1;

  L4::Ipc_iostream io(l4_utcb());
  io << L4::Opcode(Dope::Dope_app_::Cmd)
     << L4::ipc_buf_cp_out(cmd, strlen(cmd));

  l4_msgtag_t r = io.call(dope_server, Dope::Protocol::App);
  return l4_error(r);
}

int dope_cmdf(const char *format, ...)
{
  int ret;
  va_list list;
  static char cmdstr[1024];

  dopelib_mutex_lock(dopelib_cmdf_mutex);
  va_start(list, format);
  vsnprintf(cmdstr, 1024, format, list);
  va_end(list);
  ret = dope_cmd(cmdstr);
  dopelib_mutex_unlock(dopelib_cmdf_mutex);

  return ret;
}

void *dope_vscr_get_fb(const char *s)
{
  L4::Ipc_iostream io(l4_utcb());
  L4::Cap<L4Re::Dataspace> ds;

  ds = L4Re::Util::cap_alloc.alloc<L4Re::Dataspace>();

  io << L4::Opcode(Dope::Dope_app_::Vscreen_get_fb)
     << L4::ipc_buf_cp_out(s, strlen(s));
  io << L4::Small_buf(ds.cap(), 0);

  l4_msgtag_t r = io.call(dope_server, Dope::Protocol::App);

  if (l4_error(r))
    return 0;

  long sz = ds->size();
  if (sz < 0)
    return 0;

  void *addr = 0;
  if (L4Re::Env::env()->rm()->attach(&addr, sz, L4Re::Rm::Search_addr, ds))
    return 0;

  return addr;
}

#include <l4/util/util.h>
void vscr_server_waitsync(void *x)
{
  (void)x;
  // XXX: ohlala, not gooddy, the old code blocked on the server here, we
  // probably want something eventy here
  l4_sleep(100);
}

long dope_get_keystate(long keycode)
{
  L4::Ipc_iostream io(l4_utcb());
  io << L4::Opcode(Dope::Dope_app_::Get_keystate) << keycode;
  l4_msgtag_t r = io.call(dope_server, Dope::Protocol::App);

  if (l4_error(r))
    return 0;

  io >> keycode;
  return keycode;
}

#if 0
char dope_get_ascii(long id, long keycode) {
	DICE_DECLARE_ENV(_env);
	return dope_manager_get_ascii_call(dope_server, keycode, &_env);
}
#endif
