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

#include <l4/re/env>
#include <l4/re/namespace>
#include <l4/re/util/cap_alloc>

#include <l4/sys/capability>
#include <l4/sys/factory>
#include <l4/sys/typeinfo_svr>
#include <l4/cxx/ipc_server>
#include <l4/cxx/minmax>

#include <l4/re/protocols>

#include <cstdio>
#include <l4/l4con/l4con.h>
#include <l4/input/libinput.h>

#include <l4/re/util/video/goos_svr>
#include <l4/re/util/video/goos_fb>
#include <l4/re/util/event_svr>
#include <l4/re/util/event_buffer>
#include <l4/re/util/cap_alloc>

#include "object_registry_gc"
#include "main.h"
#include "l4con.h"
#include "gmode.h"
#include "srv.h"
#include "vc.h"

// ---------------------------------------------------------------
//
static L4::Cap<void> rcv_cap()
{
  static L4::Cap<void> _rcv_cap = L4Re::Util::cap_alloc.alloc<void>();
  return _rcv_cap;
}

// XXX: why is this in the L4Re::Util namespace ???
static L4Re::Util::Object_registry_gc
                              con_registry(L4Re::Env::env()->main_thread(),
                                           L4Re::Env::env()->factory());

class Vc : public L4Re::Util::Video::Goos_svr,
           public L4Re::Util::Event_svr<Vc>,
	   public L4::Server_object,
	   public l4con_vc
{
public:
  explicit Vc();
  ~Vc();
  int dispatch(l4_umword_t obj, L4::Ipc::Iostream &ios);

  void setup_info(l4con_vc *vc);
  void reg_fbds(l4_cap_idx_t c);
  void send_event(l4input *ev);
  int create_event();

  long close();
  long pslim_fill(L4::Ipc::Iostream &ios);
  long pslim_copy(L4::Ipc::Iostream &ios);
  long puts(L4::Ipc::Iostream &ios);
  long puts_scale(L4::Ipc::Iostream &ios);
  long get_font_size(L4::Ipc::Iostream &ios);

  virtual int refresh(int x, int y, int w, int h);

  long vc_dispatch(L4::Ipc::Iostream &ios);

  static L4::Cap<void> rcv_cap() { return ::rcv_cap(); }
  void reset_event_buffer() { evbuf.reset(); }
private:
  L4Re::Util::Event_buffer evbuf;
};

Vc::Vc()
{
}

int
Vc::refresh(int x, int y, int w, int h)
{
  l4con_pslim_rect_t r;
  r.x = x;
  r.y = y;
  r.w = w;
  r.h = h;
  return con_vc_direct_update_component(this, &r);
}

int
Vc::create_event()
{
  long r;

  L4Re::Util::Auto_cap<L4Re::Dataspace>::Cap b
    = L4Re::Util::cap_alloc.alloc<L4Re::Dataspace>();
  if (!b.is_valid())
    return -L4_ENOMEM;

  if ((r = L4Re::Env::env()->mem_alloc()->alloc(L4_PAGESIZE, b.get())) < 0)
    return r;

  if ((r = evbuf.attach(b.get(), L4Re::Env::env()->rm())) < 0)
    return r;

  memset(evbuf.buf(), 0, b.get()->size());

  _ds  = b.release();

  return 0;
}

long
Vc::close()
{
  return con_vc_close_component(this);
}

Vc::~Vc()
{
  con_vc_close_component(this);
}

long
Vc::pslim_fill(L4::Ipc::Iostream &ios)
{
  l4con_pslim_rect_t r;
  l4con_pslim_color_t c;
  int x, y, w, h;

  ios >> x >> y >> w >> h >> c;

  r.x = x;
  r.y = y;
  r.w = w;
  r.h = h;

  return con_vc_pslim_fill_component(this, &r, c);
}

long
Vc::pslim_copy(L4::Ipc::Iostream &ios)
{
  l4con_pslim_rect_t r;
  int x, y, w, h;
  l4_int16_t dx, dy;

  ios >> x >> y >> w >> h >> dx >> dy;

  r.x = x;
  r.y = y;
  r.w = w;
  r.h = h;

  return con_vc_pslim_copy_component(this, &r, dx, dy);
}

long
Vc::puts(L4::Ipc::Iostream &ios)
{
  char buf[150];
  char *s = 0;
  unsigned long len;
  short x;
  short y;
  l4con_pslim_color_t fg_color;
  l4con_pslim_color_t bg_color;

  ios >> x >> y >> fg_color >> bg_color
      >> L4::Ipc::Buf_in<char>(s, len);

  len = cxx::min<unsigned long>(len, sizeof(buf));
  memcpy(buf, s, len);

  return con_vc_puts_component(this, buf, len, x, y, fg_color, bg_color);
}

long
Vc::puts_scale(L4::Ipc::Iostream &ios)
{
  char buf[150];
  char *s = 0;
  unsigned long len;
  short x, y, scale_x, scale_y;
  l4con_pslim_color_t fg_color;
  l4con_pslim_color_t bg_color;

  ios >> x >> y >> fg_color >> bg_color >> scale_x >> scale_y
      >> L4::Ipc::Buf_in<char>(s, len);

  len = cxx::min<unsigned long>(len, sizeof(buf));
  memcpy(buf, s, len);

  return con_vc_puts_scale_component(this, buf, len, x, y, fg_color, bg_color,
                                     scale_x, scale_y);
}

long
Vc::get_font_size(L4::Ipc::Iostream &ios)
{
  int w = FONT_XRES, h = FONT_YRES;
  ios << w << h;
  return L4_EOK;
}

long
Vc::vc_dispatch(L4::Ipc::Iostream &ios)
{
  l4_msgtag_t tag;
  ios >> tag;

  if (tag.label() != L4con::Protocol)
    return -L4_EBADPROTO;

  L4::Opcode op;
  ios >> op;

  switch (op)
    {
    case L4con::L4con_::Close:
      return close();
    case L4con::L4con_::Pslim_fill:
      return pslim_fill(ios);
    case L4con::L4con_::Puts:
      return puts(ios);
    case L4con::L4con_::Puts_scale:
      return puts_scale(ios);
    case L4con::L4con_::Get_font_size:
      return get_font_size(ios);
    default:
      return -L4_ENOSYS;
    }
}

int
Vc::dispatch(l4_umword_t obj, L4::Ipc::Iostream &ios)
{
  l4_msgtag_t tag;
  ios >> tag;
  switch (tag.label())
    {
    case L4::Meta::Protocol:
      return L4::Util::handle_meta_request<L4con>(ios);
    case L4Re::Protocol::Goos:
      return L4Re::Util::Video::Goos_svr::dispatch(obj, ios);
    case L4Re::Protocol::Event:
    case L4_PROTO_IRQ:
      return L4Re::Util::Event_svr<Vc>::dispatch(obj, ios);
    case L4con::Protocol:
      return vc_dispatch(ios);
    default:
      return -L4_EBADPROTO;
    }
}

extern "C" l4con_vc *alloc_vc()
{ return new Vc(); }

extern "C" void create_event(struct l4con_vc *vc)
{ ((Vc *)vc)->create_event(); }

void
Vc::setup_info(l4con_vc *vc)
{
  _screen_info.pixel_info           = *(L4Re::Video::Pixel_info *)&fb_info.pixel_info;
  _screen_info.width                = vc->client_xres;
  _screen_info.height               = vc->client_yres;
  _screen_info.flags                = 0;
  _screen_info.num_static_views     = 1;
  _screen_info.num_static_buffers   = 1;

  init_infos();

  _view_info.buffer_offset          = 0;
  _view_info.bytes_per_line         = vc->bytes_per_line;
}

void
Vc::send_event(l4input *ev)
{
  evbuf.put(*reinterpret_cast<L4Re::Event_buffer::Event const*>(ev));
  static_assert(sizeof(L4Re::Event_buffer::Event) == sizeof(*ev),
                "Size mismatch");
  _irq.trigger();
}

extern "C" void fill_out_info(l4con_vc *vc)
{ ((Vc *)vc)->setup_info(vc); }

extern "C" void send_event_client(struct l4con_vc *vc, l4input *ev)
{
  if (!ev->time)
    printf("WARNING: Emiting invalid event!\n");

  if (vc_mode & CON_IN)
    ((Vc *)vc)->send_event(ev);
}

extern "C" void register_fb_ds(struct l4con_vc *vc)
{ ((Vc *)vc)->reg_fbds(vc->vfb_ds); }


void
Vc::reg_fbds(l4_cap_idx_t c)
{
  L4::Cap<L4Re::Dataspace> t(c);
  _fb_ds = t;
}

class Controller : public L4::Server_object
{
public:
  int dispatch(l4_umword_t obj, L4::Ipc::Iostream &ios);
};

int
Controller::dispatch(l4_umword_t, L4::Ipc::Iostream &ios)
{
  l4_msgtag_t tag;
  ios >> tag;

  switch (tag.label())
    {
    case L4::Meta::Protocol:
      return L4::Util::handle_meta_request<L4::Factory>(ios);
    case L4::Factory::Protocol:
      break;
    default:
      return -L4_EBADPROTO;
    }

  if (!L4::kobject_typeid<L4con>()->
        has_proto(L4::Ipc::read<L4::Factory::Proto>(ios)))
    return -L4_ENODEV;

  int connum = con_if_open_component(CON_VFB);
  if (connum < 0)
    return -L4_ENOMEM;

  con_registry.register_obj_with_gc((Vc *)vc[connum], 0);

  //con_registry.ref_cnt_add((Vc *)vc[connum], -1);
  ios << ((Vc *)vc[connum])->obj_cap();
  return L4_EOK;
}

// ---------------------------------------------------------------

class My_timeout_hooks
{
public:
  static l4_cpu_time_t next_timeout(l4_cpu_time_t old)
  { return old + REQUEST_TIMEOUT_DELTA; }

  static l4_cpu_time_t current_time()
  { return l4re_kip()->clock; }

  static void work()
  {
    periodic_work();
    con_registry.gc_run(500);
  }

  void setup_wait(L4::Ipc::Istream &istr, L4::Ipc_svr::Reply_mode)
  {
    istr.reset();
    istr << L4::Ipc::Small_buf(rcv_cap().cap(), L4_RCV_ITEM_LOCAL_ID);
    l4_utcb_br_u(istr.utcb())->bdr = 0;
  }

  static int timeout_br() { return 8; }
};

class My_hooks :
  public L4::Ipc_svr::Ignore_errors,
  public L4::Ipc_svr::Timed_work<My_timeout_hooks>
{};

static L4::Server<My_hooks> con_server(l4_utcb());

int server_loop(void)
{
  static Controller ctrl;

  if (!con_registry.register_obj(&ctrl, "con"))
    {
      printf("Service registration failed.\n");
      return 1;
    }

  printf("Ready. Waiting for clients\n");
  con_server.loop(con_registry);
  return 0;
}
