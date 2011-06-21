/*
 * (c) 2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *          Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <l4/re/env>
#include <l4/re/protocols>
#include <l4/re/namespace>
#include <l4/re/event_enums.h>
#include <l4/cxx/ipc_server>
#include <l4/re/error_helper>
#include <l4/re/rm>
#include <l4/re/dataspace>
#include <l4/re/mem_alloc>
#include <l4/re/util/video/goos_svr>
#include <l4/re/util/event_svr>
#include <l4/re/util/icu_svr>
#include <l4/re/video/goos-sys.h>
#include <l4/re/video/goos>
#include <l4/re/console>
#include <l4/re/util/meta>

#include <l4/mag/server/plugin>
#include <l4/mag/server/object>
#include <l4/mag/server/session>
#include <l4/mag/server/user_state>
#include <l4/mag-gfx/clip_guard>
#include <l4/mag-gfx/texture>
#include <l4/mag-gfx/factory>

#include <l4/cxx/list>
#include <l4/cxx/auto_ptr>

#include <cstring>
#include <cstdio>
#include <memory>
#include <vector>
#include <list>

namespace Mag_server { namespace {

using L4Re::Util::Auto_cap;
using std::auto_ptr;
using Mag_gfx::Texture;
using Mag_gfx::Area;
using L4Re::chksys;

class Mag_client : public Object, private Plugin
{
private:
  Core_api const *_core;

public:
  Mag_client() : Plugin("Mag client") {}
  char const *type() const { return "Mag client"; }
  void start(Core_api *core);

  int dispatch(l4_umword_t obj, L4::Ipc_iostream &ios);
  void destroy();
};

class Client_buffer;
class Client_view;

class Mag_goos
: public Session, public Object,
  public L4Re::Util::Icu_cap_array_svr<Mag_goos>
{
private:
  typedef L4Re::Util::Icu_cap_array_svr<Mag_goos> Icu_svr;

  Core_api const *_core;
  L4Re::Util::Auto_cap<L4Re::Dataspace>::Cap _ev_ds;
  Irq _ev_irq;
  L4Re::Rm::Auto_region<void*> _ev_ds_m;
  L4Re::Event_buffer _events;

  typedef std::vector<cxx::Ref_ptr<Client_buffer> >  Buffer_vector;
  typedef std::vector<cxx::Auto_ptr<Client_view> > View_vector;

  Buffer_vector _buffers;
  View_vector _views;

  int screen_dispatch(l4_umword_t, L4::Ipc_iostream &ios);
  int event_dispatch(l4_umword_t, L4::Ipc_iostream &ios);

public:
  Mag_goos(Core_api const *core);

  void put_event(L4Re::Event_buffer::Event const &ne, bool trigger);
  int dispatch(l4_umword_t obj, L4::Ipc_iostream &ios);

  L4::Cap<void> rcv_cap() const { return _core->rcv_cap(); }

  void destroy();

  static void set_default_background(Session *_s, Property_handler const *, cxx::String const &);
};


class Client_buffer : public cxx::Ref_obj
{
private:
  L4Re::Util::Auto_cap<L4Re::Dataspace>::Cap _ds;
  L4Re::Rm::Auto_region<void *> _texture_mem;
  unsigned long _size;

public:
  unsigned index;

  Client_buffer(Core_api const *core, unsigned long size);

  L4::Cap<L4Re::Dataspace> ds_cap() const { return _ds.get(); }
  void *local_addr() const { return _texture_mem.get(); }
  unsigned long size() const { return _size; }
};


class Client_view : public View
{
private:
  Core_api const *_core;
  cxx::Ref_ptr<Client_buffer> _buffer;
  Mag_goos *_screen;
  Texture *_front_txt;
  Texture *_back_txt;

  unsigned long _buf_offset;

  void swap_textures()
  {
    register Texture *tmp = _front_txt;
    asm volatile ("" : : : "memory");
    _front_txt = _back_txt;
    _back_txt = tmp;
  }

public:
  Client_view(Core_api const *core, Mag_goos *screen);
  virtual ~Client_view();

  int dispatch(l4_umword_t obj, L4::Ipc_iostream &ios);
  void draw(Canvas *, View_stack const *, Mode) const;
  void handle_event(L4Re::Event_buffer::Event const &e, Point const &mouse);

  void get_info(L4Re::Video::View::Info *inf) const;
  void set_info(L4Re::Video::View::Info const &inf,
                cxx::Ref_ptr<Client_buffer> const &b);

  Session *session() const { return _screen; }
};

Client_view::Client_view(Core_api const *core, Mag_goos *screen)
: View(Rect(), F_need_frame), _core(core), _buffer(0), _screen(screen),
  _buf_offset(0)
{
  Pixel_info const *pi = core->user_state()->vstack()->canvas()->type();

  _front_txt = pi->factory->create_texture(Area(0,0), (void*)1);
  _back_txt = pi->factory->create_texture(Area(0,0), (void*)1);
  calc_label_sz(core->label_font());
}

Client_view::~Client_view()
{
  if (_screen && _screen->background() == this)
    {
      // look for other background views below
      View_stack *vs = _core->user_state()->vstack();
      // We can either search below this view in the stack, or
      // we can search from the top of the stack to find the uppermost
      // view of our session that is tagged as background
      View *v = vs->next_view(this); // search below this view
      // View *v = vs->top(); // Search from the top of the stack
      for (; v; v = vs->next_view(v))
	if (v != this && v->session() == _screen && v->background())
	  break;
      _screen->background(v);
    }

  _core->user_state()->forget_view(this);
  delete _back_txt;
  delete _front_txt;
}

inline
void
Client_view::get_info(L4Re::Video::View::Info *inf) const
{
  using L4Re::Video::Color_component;
  inf->flags = L4Re::Video::View::F_fully_dynamic;
  // we do not support chaning the pixel format
  inf->flags &= ~L4Re::Video::View::F_set_pixel;
  if (above())
    inf->flags |= L4Re::Video::View::F_above;

  inf->xpos = x1();
  inf->ypos = y1();
  inf->width = w();
  inf->height = h();
  inf->buffer_offset = _buf_offset;

  Pixel_info const *pi = 0;
  pi = _front_txt->type();

  inf->bytes_per_line = pi->bytes_per_pixel() * _front_txt->size().w();
  inf->pixel_info = *pi;

  if (_buffer)
    inf->buffer_index = _buffer->index;
  else
    inf->buffer_index = ~0;
}

inline
void
Client_view::set_info(L4Re::Video::View::Info const &inf,
                      cxx::Ref_ptr<Client_buffer> const &b)
{
  Pixel_info const *pi = _core->user_state()->vstack()->canvas()->type();

  bool recalc_height = false;
  _back_txt->size(_front_txt->size());
  _back_txt->pixels(_front_txt->pixels());

  if (inf.flags & L4Re::Video::View::F_set_flags)
    set_above(inf.flags & L4Re::Video::View::F_above);

  if (inf.flags & L4Re::Video::View::F_set_background)
    {
      _core->user_state()->vstack()->push_bottom(this);
      set_as_background();
      _screen->background(this);
    }

  if (inf.has_set_bytes_per_line())
    {
      _back_txt->size(Area(inf.bytes_per_line / pi->bytes_per_pixel(), 0));
      recalc_height = true;
    }

  if (inf.has_set_buffer())
    {
      _back_txt->pixels((char *)b->local_addr() + _buf_offset);
      _buffer = b;
      recalc_height = true;
    }

  if (!_buffer)
    {
      _back_txt->size(Area(0, 0));
      _back_txt->pixels((char *)0);
      _front_txt->size(Area(0, 0));
      _front_txt->pixels((char *)0);
    }

  if (inf.has_set_buffer_offset() && _buffer)
    {
      _back_txt->pixels((char *)_buffer->local_addr() + inf.buffer_offset);
      _buf_offset = inf.buffer_offset;
      recalc_height = true;
    }

  if (recalc_height && _buffer)
    {
      unsigned long w = _back_txt->size().w();
      unsigned long bw = w * pi->bytes_per_pixel();
      unsigned long h;

      if (bw > 0 && w > 0)
	{
	  h = _buffer->size();
	  if (h > _buf_offset)
	    h -= _buf_offset;
	  else
	    h = 0;

	  h /= bw;
	}
      else
	{
	  w = 0;
	  h = 0;
	}
      _back_txt->size(Area(w, h));
    }

  if (_back_txt->size() != _front_txt->size()
      || _back_txt->pixels() != _front_txt->pixels())
    swap_textures();

  if (inf.has_set_position())
    _core->user_state()->vstack()->viewport(this, Rect(Point(inf.xpos,
            inf.ypos), Area(inf.width, inf.height)), true);
}


Mag_goos::Mag_goos(Core_api const *core)
: Icu_svr(1, &_ev_irq), _core(core)
{
  L4Re::Env const *e = L4Re::Env::env();
  _ev_ds = L4Re::Util::cap_alloc.alloc<L4Re::Dataspace>();

  chksys(e->mem_alloc()->alloc(L4_PAGESIZE, _ev_ds.get()));
  chksys(e->rm()->attach(&_ev_ds_m, L4_PAGESIZE, L4Re::Rm::Search_addr, _ev_ds.get()));

  _events = L4Re::Event_buffer(_ev_ds_m.get(), L4_PAGESIZE);
}

void Mag_client::start(Core_api *core)
{
  _core = core;
  core->registry()->register_obj(cxx::Ref_ptr<Mag_client>(this), "mag");
  if (!obj_cap().is_valid())
    printf("Service registration failed.\n");
  else
    printf("Plugin: Mag_client service started\n");
}

void Mag_goos::set_default_background(Session *_s, Property_handler const *, cxx::String const &)
{
  Mag_goos *s = static_cast<Mag_goos *>(_s);

  s->flags(F_default_background, 0);
}

namespace {
  Session::Property_handler const _opts[] =
    { { "default-background", false,  &Mag_goos::set_default_background },
      { "dfl-bg",             false,  &Mag_goos::set_default_background },
      { 0, 0, 0 }
    };
};

int
Mag_client::dispatch(l4_umword_t, L4::Ipc_iostream &ios)
{
  l4_msgtag_t tag;
  ios >> tag;

  switch (tag.label())
    {
    case L4::Meta::Protocol:
      return L4Re::Util::handle_meta_request<L4::Factory>(ios);
    case L4::Factory::Protocol:
      if (L4::kobject_typeid<L4Re::Console>()->
	    has_proto(L4::Ipc::read<L4::Factory::Proto>(ios)))
	{
	  L4::Ipc::Istream_copy cp_is = ios;

	  cxx::Ref_ptr<Mag_goos> cf(new Mag_goos(_core));
	  _core->set_session_options(cf.get(), cp_is, _opts);

	  _core->register_session(cf.get());
	  _core->registry()->register_obj(cf);
	  cf->obj_cap()->dec_refcnt(1);

	  ios <<  L4::Ipc::Snd_fpage(cf->obj_cap(), L4_CAP_FPAGE_RWSD);
	  return L4_EOK;
	}
      return -L4_ENODEV;
    default:
      return -L4_EBADPROTO;
    }
}

void
Mag_client::destroy()
{
}

int
Mag_goos::dispatch(l4_umword_t obj, L4::Ipc_iostream &ios)
{
  l4_msgtag_t tag;
  ios >> tag;

  switch (tag.label())
    {
    case L4::Meta::Protocol:
      return L4Re::Util::handle_meta_request<L4Re::Console>(ios);
    case L4Re::Protocol::Goos:
      return screen_dispatch(obj, ios);
    case L4Re::Protocol::Event:
      return event_dispatch(obj, ios);
    case L4_PROTO_IRQ:
      return Icu_svr::dispatch(obj, ios);
    default:
      return -L4_EBADPROTO;
    }
}


int
Mag_goos::event_dispatch(l4_umword_t, L4::Ipc_iostream &ios)
{
  L4::Opcode op;
  ios >> op;
  switch (op)
    {
    case L4Re::Event_::Get:
      ios << L4::Ipc::Snd_fpage(_ev_ds.get().fpage(L4_CAP_FPAGE_RW));
      return L4_EOK;
    default:
      return -L4_ENOSYS;
    }
}

int
Mag_goos::screen_dispatch(l4_umword_t, L4::Ipc_iostream &ios)
{
  L4::Opcode op;
  ios >> op;

  switch (op)
    {
    case ::L4Re::Video::Goos_::Info:
        {
	  using L4Re::Video::Color_component;
	  using L4Re::Video::Goos;

	  Goos::Info i;
          Area a = _core->user_state()->vstack()->canvas()->size();
          Pixel_info const *mag_pi = _core->user_state()->vstack()->canvas()->type();
	  i.width = a.w();
	  i.height = a.h();
	  i.flags = Goos::F_pointer
	    | Goos::F_dynamic_views
	    | Goos::F_dynamic_buffers;

	  i.num_static_views = 0;
	  i.num_static_buffers = 0;
	  i.pixel_info = *mag_pi;

	  ios.put(i);

          return L4_EOK;
        }

    case L4Re::Video::Goos_::Create_buffer:
        {
	  unsigned long size;
          ios >> size;

	  cxx::Ref_ptr<Client_buffer> b(new Client_buffer(_core, size));
	  _buffers.push_back(b);
	  b->index = _buffers.size() - 1;

          ios << L4::Ipc::Snd_fpage(b->ds_cap(), L4_CAP_FPAGE_RW);

          return b->index;
        }

    case L4Re::Video::Goos_::Create_view:
        {
	  cxx::Auto_ptr<Client_view> v(new Client_view(_core, this));
	  unsigned idx = 0;
	  for (View_vector::iterator i = _views.begin(); i != _views.end();
	       ++i, ++idx)
	    if (!*i)
	      {
		*i = v;
		return idx;
	      }

	  _views.push_back(v);
          return _views.size() - 1;
        }

    case L4Re::Video::Goos_::Delete_view:
        {
	  unsigned idx;
	  ios >> idx;
	  if (idx >= _views.size())
	    return -L4_ERANGE;

	  _views[idx].reset(0);
	  return 0;
        }

    case L4Re::Video::Goos_::Get_buffer:
	{
	  unsigned idx;
	  ios >> idx;
	  if (idx >= _buffers.size())
	    return -L4_ERANGE;

	  ios << _buffers[idx]->ds_cap();
	  return L4_EOK;
	}

    case L4Re::Video::Goos_::View_info:
	{
	  unsigned idx;
	  ios >> idx;
	  if (idx >= _views.size())
	    return -L4_ERANGE;

	  Client_view *cv = _views[idx].get();

	  L4Re::Video::View::Info vi;
	  vi.view_index = idx;
	  cv->get_info(&vi);
	  ios.put(vi);

	  return L4_EOK;
	}

    case L4Re::Video::Goos_::View_set_info:
	{
	  unsigned idx;
	  ios >> idx;
	  if (idx >= _views.size())
	    return -L4_ERANGE;

	  Client_view *cv = _views[idx].get();

	  L4Re::Video::View::Info vi;
	  ios.get(vi);

	  cxx::Weak_ptr<Client_buffer> cb(0);
	  if (vi.has_set_buffer())
	    {
	      if (vi.buffer_index >= _buffers.size())
		return -L4_ERANGE;

	      cb = _buffers[vi.buffer_index];
	    }

	  cv->set_info(vi, cb);
	  return L4_EOK;
	}

    case L4Re::Video::Goos_::View_stack:
	{
          Client_view *pivot = 0;
          Client_view *cv;
          bool behind;
	  unsigned cvi, pvi;

          ios >> cvi >> pvi >> behind;

	  if (cvi >= _views.size())
	    return -L4_ERANGE;

	  cv = _views[cvi].get();

	  if (pvi < _views.size())
	    pivot = _views[pvi].get();

          if (!pivot)
            {
              if (!behind)
                _core->user_state()->vstack()->push_bottom(cv);
              else
                _core->user_state()->vstack()->push_top(cv);
            }
          else
            _core->user_state()->vstack()->stack(cv, pivot, behind);

        }
      return L4_EOK;

    case L4Re::Video::Goos_::View_refresh:
        {
	  unsigned idx;
	  int x, y, w, h;
	  ios >> idx >> x >> y >> w >> h;

	  if (idx >= _views.size())
	    return -L4_ERANGE;

	  Client_view *cv = _views[idx].get();
	  _core->user_state()->vstack()->refresh_view(cv, 0, Rect(cv->p1() + Point(x,y), Area(w,h)));

	  return L4_EOK;
	}

    case L4Re::Video::Goos_::Screen_refresh:
        {
	  int x, y, w, h;
	  ios >> x >> y >> w >> h;

	  _core->user_state()->vstack()->refresh_view(0, 0, Rect(Point(x,y), Area(w,h)));

	  return L4_EOK;
	}

    default:
      return -L4_ENOSYS;
    }
}

void
Mag_goos::destroy()
{
  _buffers.clear();
  _views.clear();
}


Client_buffer::Client_buffer(Core_api const *, unsigned long size)
: _size(l4_round_page(size))
{
  L4Re::Rm::Auto_region<void *> dsa;
  _ds = L4Re::Util::cap_alloc.alloc<L4Re::Dataspace>();

  L4Re::chksys(L4Re::Env::env()->mem_alloc()->alloc(_size, _ds.get()));
  L4Re::chksys(L4Re::Env::env()->rm()->attach(&dsa, _size,
	L4Re::Rm::Search_addr, _ds.get()));

  _texture_mem = dsa;
}



void
Mag_goos::put_event(L4Re::Event_buffer::Event const &ne, bool trigger)
{
  if (_events.put(ne) && trigger)
    _ev_irq.trigger();
}


void
Client_view::draw(Canvas *c, View_stack const *, Mode mode) const
{
  Canvas::Mix_mode op = mode.flat() ? Canvas::Solid : Canvas::Mixed;
  if (mode.xray() && !mode.kill() && focused())
    op = Canvas::Solid;

  Clip_guard cg(c, *this);

  if (!c->clip_valid())
    return;

  Rgb32::Color mix_color = /*mode.kill() ? kill_color() :*/ session()->color();
  Area s(0, 0);
  if (_buffer)
    {
      c->draw_texture(_front_txt, mix_color, p1(), op);
      s = _front_txt->size();
    }

  Area r = size() - s;
  if (r.h() > 0)
    c->draw_box(Rect(p1() + Point(0, s.h()), Area(size().w(), r.h())), mix_color);

  if (r.w() > 0 && size().h() != r.h())
    c->draw_box(Rect(p1() + Point(s.w(), 0), Area(r.w(), s.h())), mix_color);
}

void
Client_view::handle_event(L4Re::Event_buffer::Event const &e,
                          Point const &mouse)
{
  if (e.payload.type == L4RE_EV_MAX)
    {
      L4Re::Event_buffer::Event ne;
      ne.time = e.time;
      ne.payload.type = L4RE_EV_ABS;
      ne.payload.code = L4RE_ABS_X;
      ne.payload.value = mouse.x();
      ne.payload.stream_id = e.payload.stream_id;
      _screen->put_event(ne, false);
      ne.payload.code = L4RE_ABS_Y;
      ne.payload.value = mouse.y();
      _screen->put_event(ne, true);
      return;
    }

  _screen->put_event(e, true);

}


static Mag_client _mag_client;

}}
