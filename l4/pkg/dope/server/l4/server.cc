/*
 * \brief   DOpE server module
 * \date    2002-11-13
 * \author  Norman Feske <nf2@inf.tu-dresden.de>
 *
 * This module is the main communication interface
 * between DOpE clients and DOpE.
 */

/*
 * Copyright (C) 2002-2004  Norman Feske  <nf2@os.inf.tu-dresden.de>
 * Technische Universitaet Dresden, Operating Systems Research Group
 *
 * This file is part of the DOpE package, which is distributed under
 * the  terms  of the  GNU General Public Licence 2.  Please see the
 * COPYING file for details.
 */

/*** GENERAL ***/
#include <cstdlib>
#include <cstdio>
#include <ctype.h>

/*** DOPE SPECIFIC ***/
#include "dopestd.h"
#include "thread.h"
#include "server.h"
#include "appman.h"
#include "script.h"
#include "scope.h"
#include "screen.h"
#include "messenger.h"
#include "userstate.h"
#include "scheduler.h"
#include <l4/dope/dopedef.h>

#include <l4/re/env>
#include <l4/re/namespace>
#include <l4/re/util/cap_alloc>
#include <l4/sys/factory>
#include <l4/sys/typeinfo_svr>
#include <l4/cxx/exceptions>

#include <l4/cxx/iostream>
#include <l4/cxx/l4iostream>
#include <l4/cxx/minmax>

#include <l4/re/protocols>
#include <l4/re/util/object_registry>
#include <l4/re/util/video/goos_svr>
#include <l4/re/util/event_svr>
#include <l4/re/util/event_buffer>
#include <l4/re/c/event.h>
#include <l4/re/video/goos>

#include <l4/sys/debugger.h>
#include <pthread-l4.h>

static struct userstate_services *userstate;
static struct thread_services    *thread;
static struct appman_services    *appman;
static struct script_services    *script;
static struct scope_services     *scope;
static struct screen_services    *screen;
static struct scheduler_services *scheduler;
static struct messenger_services *msg;

extern "C" int init_server(struct dope_services *d);


static int get_val(const char *configstr, const char *var, unsigned long *val)
{
  char *a;
  a = strstr(configstr, var);
  if (!a)
    return 0;
  *val = strtoul(a + strlen(var), NULL, 0);
  return 1;
}

static int get_string(const char *configstr, const char *var,
                      char **val, unsigned long *vallen)
{
  char *a;
  a = strstr(configstr, var);
  if (!a)
    return 0;
  a += strlen(var);
  *val = a;
  while (*a && !isspace(*a))
    a++;
  *vallen = a - *val;
  return 1;
}

// ------------------------------------------------------------------------
static L4Re::Util::Object_registry *dope_registry;

// ------------------------------------------------------------------------

static L4::Cap<void> rcv_cap()
{
  static L4::Cap<void> _rcv = L4Re::Util::cap_alloc.alloc<void>();
  return _rcv;
}

class Dope_base : public L4Re::Util::Event_svr<Dope_base>,
                  public L4::Server_object
{
protected:
  s32 app_id;
  L4Re::Util::Event_buffer evbuf;

  int cmd(const char *c);
  int cmd_ret(const char *c, char *result, unsigned long res_len);
  int create_event();

  bool register_appid(s32 ai, Dope_base *obj);

public:
  void event_cb(l4re_event_t *e, unsigned nr_events);
  static Dope_base *get_obj(s32 ai);
  static L4::Cap<void> rcv_cap() { return ::rcv_cap(); }

private:
  static void check_appid(s32 ai);
  static Dope_base *appid_to_obj[MAX_APPS];
};

Dope_base *Dope_base::appid_to_obj[MAX_APPS];

void Dope_base::check_appid(s32 ai)
{
  if (ai >= MAX_APPS)
    {
      printf("Oops: %d >= MAX_APPS\n", ai);
      exit(1);
    }
}

bool Dope_base::register_appid(s32 ai, Dope_base *obj)
{
  check_appid(ai);
  appid_to_obj[ai] = obj;
  return true;
}

Dope_base *Dope_base::get_obj(s32 ai)
{
  check_appid(ai);
  return appid_to_obj[ai];
}

extern "C" void dope_event_inject(s32 appid, l4re_event_t *e,
                                  unsigned nr_events)
{
  Dope_base::get_obj(appid)->event_cb(e, nr_events);
}

int Dope_base::cmd(const char *c)
{
  INFO(printf("Server(exec_cmd): cmd %s execution requested by app_id=%u\n", c, (u32)app_id);)
  appman->lock(app_id);
  int ret = script->exec_command(app_id, (char *)c, NULL, 0);
  appman->unlock(app_id);
  if (ret < 0)
    printf("DOpE(exec_cmd): Error - command \"%s\" returned %d\n", c, ret);
  return ret;
}

int Dope_base::cmd_ret(const char *c, char *result, unsigned long res_len)
{
  INFO(printf("Server(exec_req): cmd %s execution requested by app_id=%u\n",
              c, (u32)app_id);)
  appman->lock(app_id);
  result[0] = 0;
  int ret = script->exec_command(app_id, (char *)c, result, res_len);
  appman->unlock(app_id);
  INFO(printf("Server(exec_req): send result msg: %s\n", result));

  if (ret < 0) printf("DOpE(exec_req): Error - command \"%s\" returned %d\n", c, ret);
  return ret;
}

int
Dope_base::create_event()
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

void Dope_base::event_cb(l4re_event_t *e, unsigned nr_events)
{
#if 0
  printf("emmit event (client=%p): event (t=%d c=%d v=%d stream=%lx)\n",
         this, e->type, e->code, e->value, e->stream_id);
#endif
  for (unsigned i = 0; i < nr_events; ++i)
    evbuf.put(*reinterpret_cast<L4Re::Event_buffer::Event const*>(&e[i]));
  _irq.trigger();
}


// ------------------------------------------------------------------------

class Dope_app : public Dope_base
{
public:
  explicit Dope_app(const char *configstr);
  int dispatch(l4_umword_t obj, L4::Ipc_iostream &ios);
  int dispatch_dope_app(l4_umword_t, L4::Ipc_iostream &ios);

  int client_cmd(L4::Ipc_iostream &ios);
  int client_cmd_req(L4::Ipc_iostream &ios);
  int client_vscreen_get_fb(L4::Ipc_iostream &ios);
  int client_get_keystate(L4::Ipc_iostream &ios);
};

Dope_app::Dope_app(const char *configstr)
{
  dope_registry->register_obj(this);

  unsigned long val;
  char *s;
  char buf[80];
  const char *appname = "App";

  if (get_string(configstr, "name=", &s, &val))
    {
      strncpy(buf, s, val);
      buf[val] = 0;
      appname = buf;
    }

  if (int r = create_event())
    throw (L4::Runtime_error(r));

  app_id = appman->reg_app(appname);
  register_appid(app_id, this);

  SCOPE *rootscope = scope->create();
  appman->set_rootscope(app_id, rootscope);
  INFO(printf("Server(init_app): application init request. appname=%s\n", appname));
}

int Dope_app::client_cmd(L4::Ipc_iostream &ios)
{
  unsigned long len;
  char buf_in[256];

  len = sizeof(buf_in);
  ios >> L4::ipc_buf_cp_in(buf_in, len);
  buf_in[len] = 0;

  cmd(buf_in);
  return -L4_EOK;
}

int Dope_app::client_cmd_req(L4::Ipc_iostream &ios)
{
  unsigned long len;
  char buf_in[256];
  char buf_out[256];

  len = sizeof(buf_in);
  ios >> L4::ipc_buf_cp_in(buf_in, len);
  buf_in[len] = 0;

  cmd_ret(buf_in, buf_out, sizeof(buf_out));

  len = strlen(buf_out);
  if (len >= sizeof(buf_out))
    {
      len = sizeof(buf_out) - 1;
      buf_out[sizeof(buf_out) - 1] = 0;
    }

  ios << L4::ipc_buf_cp_out(buf_out, len);

  return -L4_EOK;
}

int Dope_app::client_vscreen_get_fb(L4::Ipc_iostream &ios)
{
  unsigned long len;
  char buf_in[30];
  char buf[40];

  len = sizeof(buf_in);
  ios >> L4::ipc_buf_cp_in(buf_in, len);
  buf_in[len] = 0;

  snprintf(buf, sizeof(buf), "%s.map()", buf_in);
  cmd_ret(buf, buf_in, sizeof(buf_in));
  l4_cap_idx_t x = strtoul(&buf_in[3], NULL, 0);
  L4::Cap<L4Re::Dataspace> ds = L4::Cap<L4Re::Dataspace>(x);
  ios << ds;
  return -L4_EOK;
}

int Dope_app::client_get_keystate(L4::Ipc_iostream &ios)
{
  long x;
  ios >> x; // get keycode
  x = userstate->get_keystate(x);
  ios << x; // send state
  return -L4_EOK;
}

int Dope_app::dispatch_dope_app(l4_umword_t, L4::Ipc_iostream &ios)
{
  L4::Opcode op;
  ios >> op;

  switch (op)
    {
    case Dope::Dope_app_::Cmd:
      return client_cmd(ios);
    case Dope::Dope_app_::Cmd_req:
      return client_cmd_req(ios);
    case Dope::Dope_app_::Vscreen_get_fb:
      return client_vscreen_get_fb(ios);
    case Dope::Dope_app_::Get_keystate:
      return client_get_keystate(ios);
    default:
      return -L4_ENOSYS;
    };
}

int Dope_app::dispatch(l4_umword_t obj, L4::Ipc_iostream &ios)
{
  l4_msgtag_t tag;
  ios >> tag;

  switch (tag.label())
    {
    case Dope::Protocol::App:
      return dispatch_dope_app(obj, ios);
    case L4Re::Protocol::Event:
    case L4_PROTO_IRQ:
      return L4Re::Util::Event_svr<Dope_base>::dispatch(obj, ios);
    default:
      return -L4_EBADPROTO;
    };
}


// ------------------------------------------------------------------------

class Dope_fb : public L4Re::Util::Video::Goos_svr,
                public Dope_base
{
public:
  explicit Dope_fb(const char *configstr);
  int dispatch(l4_umword_t obj, L4::Ipc_iostream &ios);

  virtual int refresh(int x, int y, int w, int h);
};

Dope_fb::Dope_fb(const char *configstr)
{
  unsigned long val;
  unsigned xpos = 20, ypos = 20;

  dope_registry->register_obj(this);

  _screen_info.width     = 300;
  _screen_info.height    = 200;


  if (get_val(configstr, "w=", &val))
    _screen_info.width  = val;
  if (get_val(configstr, "h=", &val))
    _screen_info.height = val;

  if (get_val(configstr, "x=", &val))
    xpos = val;
  if (get_val(configstr, "y=", &val))
    ypos = val;

  L4Re::Video::Pixel_info pixinfo(16, 5, 11, 6, 5, 5, 0);
  pixinfo.bytes_per_pixel(2);

  _screen_info.flags        = L4Re::Video::Goos::F_pointer;
  _screen_info.num_static_views   = 1;
  _screen_info.num_static_buffers = 1;
  _screen_info.pixel_info   = pixinfo;

  init_infos();
  _view_info.bytes_per_line = _screen_info.width * 2;
  _view_info.buffer_offset  = 0;

  const char *appname = "fb";
  //const char *listener = "listener";
  char *s;
  char buf[80];

  if (get_string(configstr, "name=", &s, &val))
    {
      strncpy(buf, s, val);
      buf[val] = 0;
      appname = buf;
    }

  app_id = appman->reg_app(appname);
  register_appid(app_id, this);

  SCOPE *rootscope = scope->create();
  //THREAD *listener_thread = thread->alloc_thread();
  appman->set_rootscope(app_id, rootscope);

  INFO(printf("Server(init_fb): fb init request. appname=%s\n", appname));

  //thread->ident2thread(listener, listener_thread);
  //appman->reg_list_thread(app_id, listener_thread);
  //appman->reg_listener(app_id, (void *)&listener_thread->tid);

  //appman->reg_app_thread(app_id, (THREAD *)&client_thread);

  if (int r = create_event())
    throw (L4::Runtime_error(r));

  cmd("x=new Window()");
  cmd("y=new VScreen()");
  snprintf(buf, sizeof(buf), "y.setmode(%ld,%ld,\"RGB16\")",
                             _screen_info.width, _screen_info.height);
  buf[sizeof(buf)-1] = 0;
  cmd(buf);

  snprintf(buf, sizeof(buf), "x.set(-x %d -y %d -content y -workw %ld -workh %ld)",
                             xpos, ypos,
                             _screen_info.width, _screen_info.height);
  buf[sizeof(buf)-1] = 0;
  cmd(buf);

  snprintf(buf, sizeof(buf), "y.bind(\"press\",   1)");
  buf[sizeof(buf)-1] = 0;
  cmd(buf);

  snprintf(buf, sizeof(buf), "y.bind(\"release\",  1)");
  buf[sizeof(buf)-1] = 0;
  cmd(buf);

  snprintf(buf, sizeof(buf), "y.bind(\"motion\",   1)");
  buf[sizeof(buf)-1] = 0;
  cmd(buf);

  cmd("x.open()");

  cmd_ret("y.map()", buf, sizeof(buf));

  printf("y.map() = %s\n", buf);

  l4_cap_idx_t x = strtoul(&buf[3], NULL, 0);
  _fb_ds = L4::Cap<L4Re::Dataspace>(x);

//  printf("fb_ds = %lx\n", _fb_ds.cap());
//  printf("evbuf2 = %p\n", evbuf.buf());
}

int Dope_fb::refresh(int x, int y, int w, int h)
{
  char buf[70];
  // Hmm: y is a VSCREEN, so we could use y->vscr->refresh(y, ...)
  snprintf(buf, sizeof(buf), "y.refresh(-x %d -y %d -w %d -h %d)", x, y, w, h);
  buf[sizeof(buf) - 1] = 0;
  cmd(buf);
  return L4_EOK;
}

int Dope_fb::dispatch(l4_umword_t obj, L4::Ipc_iostream &ios)
{
  l4_msgtag_t tag;
  ios >> tag;

  switch (tag.label())
    {
    case L4Re::Protocol::Goos:
       return L4Re::Util::Video::Goos_svr::dispatch(obj, ios);
    case L4Re::Protocol::Event:
    case L4_PROTO_IRQ:
       return L4Re::Util::Event_svr<Dope_base>::dispatch(obj, ios);
    default:
       return -L4_EBADPROTO;
    }
}

// ------------------------------------------------------------------------

class Controller : public L4::Server_object
{
public:
  int dispatch(l4_umword_t obj, L4::Ipc_iostream &ios);
};

int
Controller::dispatch(l4_umword_t, L4::Ipc_iostream &ios)
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

  L4::Factory::Proto op;
  L4::Ipc::Varg arg;
  ios >> op >> arg;
  if (!arg.is_of<char const*>())
    return -L4_EINVAL;

  unsigned long size = 100;
  char s[size];
  strncpy(s, arg.value<char const*>(), cxx::min<int>(size, arg.length()));
  s[size] = 0;

  try
    {
      switch (op)
	{
	default:
	  printf("Invalid object type requested\n");
	  return -L4_ENODEV;

	case L4Re::Video::Goos::Protocol:
	    {
	      Dope_fb *x = new Dope_fb(s);
	      ios << x->obj_cap();
	      return L4_EOK;
	    }
	case 0: // dope iface
	    {
	      Dope_app *x = new Dope_app(s);
	      ios << x->obj_cap();
	      return L4_EOK;
	    }
	}
    }
  catch (L4::Runtime_error const &e)
    {
      return e.err_no();
    }
}

struct My_loop_hooks :
  public L4::Ipc_svr::Ignore_errors,
  public L4::Ipc_svr::Default_timeout,
  public L4::Ipc_svr::Compound_reply
{
  void setup_wait(L4::Ipc_istream &istr, L4::Ipc_svr::Reply_mode)
  {
    istr.reset();
    istr << L4::Small_buf(rcv_cap().cap(), L4_RCV_ITEM_LOCAL_ID);
    l4_utcb_br_u(istr.utcb())->bdr = 0;
  }
};

/*** DOpE SERVER THREAD ***/
static void *server_thread(void *)
{
  static Controller ctrl;

  l4_debugger_set_object_name(pthread_getl4cap(pthread_self()), "dope-srv");

  dope_registry = new L4Re::Util::Object_registry
      (L4::Cap<L4::Thread>(pthread_getl4cap(pthread_self())),
       L4Re::Env::env()->factory());

  if (!dope_registry)
    return NULL;

  static L4::Server<My_loop_hooks> dope_server(l4_utcb());

  if (!dope_registry->register_obj(&ctrl, "dope").is_valid())
    {
      printf("Service registration failed.\n");
      return NULL;
    }

  INFO(printf("Server(server_thread): entering server loop\n"));
  dope_server.loop(dope_registry);

  return NULL;
}


/******************************************************
 *** FUNCTIONS THAT ARE CALLED BY THE SERVER THREAD ***
 ******************************************************/

long dope_manager_init_app_component(void *,
    const char* ,//appname,
    const char* ,//listener,
    void *)
{
  printf("%s %d\n", __func__, __LINE__); 
#if 0
	s32 app_id = appman->reg_app((char *)appname);
	SCOPE *rootscope = scope->create();
	THREAD *listener_thread = thread->alloc_thread();
	appman->set_rootscope(app_id, rootscope);

	INFO(printf("Server(init_app): application init request. appname=%s, listener=%s (new app_id=%x)\n", appname, listener, (int)app_id));

	thread->ident2thread(listener, listener_thread);
	appman->reg_list_thread(app_id, listener_thread);
	appman->reg_listener(app_id, (void *)&listener_thread->tid);
	return app_id;
#endif
        return 0;
}


void dope_manager_deinit_app_component(void *,
                                       long ,//app_id,
                                       void *)
{
  printf("%s %d\n", __func__, __LINE__); 
#if 0
	struct thread client_thread = { *_dice_corba_obj };

	/* check if the app_id belongs to the client */
	if (!thread->thread_equal(&client_thread, appman->get_listener(app_id))) {
		printf("Server(deinit_app): Error: permission denied\n");
		return;
	}

	INFO(printf("Server(deinit_app): application (id=%u) deinit requested\n", (u32)app_id);)
	scheduler->release_app(app_id);
	userstate->release_app(app_id);
	screen->forget_children(app_id);
	appman->unreg_app(app_id);
#endif
}


long dope_manager_exec_cmd_component(void *,
                                     long ,//app_id,
                                     const char* ,//cmd,
                                     void *)
{
#if 0
	struct thread client_thread = { *_dice_corba_obj };
	int ret;

	/* check if the app_id belongs to the client */
	if (!thread->thread_equal(&client_thread, appman->get_listener(app_id))) {
		printf("Server(exec_cmd): Error: permission denied\n");
		return DOPE_ERR_PERM;
	}

	appman->reg_app_thread(app_id, (THREAD *)&client_thread);
	INFO(printf("Server(exec_cmd): cmd %s execution requested by app_id=%u\n", cmd, (u32)app_id);)
//	printf("Server(exec_cmd): cmd %s execution requested by app_id=%u\n", cmd, (u32)app_id);
	appman->lock(app_id);
	ret = script->exec_command(app_id, (char *)cmd, NULL, 0);
	appman->unlock(app_id);
	if (ret < 0) printf("DOpE(exec_cmd): Error - command \"%s\" returned %d\n", cmd, ret);
	return ret;
#endif
        return 0;
}


long dope_manager_exec_req_component(void *,
                                     long ,//app_id,
                                     const char* ,//cmd,
                                     char /*result*/[256],
                                     int * , //res_len,
                                     void *)
{
#if 0
	struct thread client_thread = { *_dice_corba_obj };
	int ret;

	/* check if the app_id belongs to the client */
	if (!thread->thread_equal(&client_thread, appman->get_listener(app_id))) {
		printf("Server(exec_req): Error: permission denied\n");
		return DOPE_ERR_PERM;
	}

	appman->reg_app_thread(app_id, (THREAD *)&client_thread);

	INFO(printf("Server(exec_req): cmd %s execution requested by app_id=%u\n", cmd, (u32)app_id);)
	appman->lock(app_id);
	result[0] = 0;
	ret = script->exec_command(app_id, (char *)cmd, &result[0], *res_len);
	appman->unlock(app_id);
	INFO(printf("Server(exec_req): send result msg: %s\n", result));

	if (ret < 0) printf("DOpE(exec_req): Error - command \"%s\" returned %d\n", cmd, ret);
	result[255] = 0;
	return ret;
#endif
        return 0;
}


long dope_manager_get_keystate_component(void *,
                                         long keycode,
                                         void *)
{
	return userstate->get_keystate(keycode);
}


char dope_manager_get_ascii_component(void  *,
                                      long keycode,
                                      void *)
{
	return userstate->get_ascii(keycode);
}


/*************************
 *** SERVICE FUNCTIONS ***
 *************************/

/*** START SERVING ***/
static void start(void)
{
	INFO(printf("Server(start): creating server thread\n");)
	thread->start_thread(NULL, &server_thread, NULL);
}


/****************************************
 *** SERVICE STRUCTURE OF THIS MODULE ***
 ****************************************/

static struct server_services services = {
	start,
};


/**************************
 *** MODULE ENTRY POINT ***
 **************************/

extern "C" int init_server(struct dope_services *d)
{
  thread    = (thread_services *)d->get_module("Thread 1.0");
  appman    = (appman_services *)d->get_module("ApplicationManager 1.0");
  script    = (script_services *)d->get_module("Script 1.0");
  msg       = (messenger_services *)d->get_module("Messenger 1.0");
  userstate = (userstate_services *)d->get_module("UserState 1.0");
  scope     = (scope_services *)d->get_module("Scope 1.0");
  screen    = (screen_services *)d->get_module("Screen 1.0");
  scheduler = (scheduler_services *)d->get_module("Scheduler 1.0");

  d->register_module("Server 1.0", &services);
  return 1;
}
