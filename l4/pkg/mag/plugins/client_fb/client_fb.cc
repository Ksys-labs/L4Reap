/*
 * (c) 2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *          Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "client_fb.h"

#include <l4/mag-gfx/clip_guard>
#include <l4/mag-gfx/texture>

#include <l4/mag/server/view_stack>
#include <l4/mag/server/factory>

#include <l4/re/env>
#include <l4/re/event_enums.h>
#include <l4/re/error_helper>
#include <l4/re/util/cap_alloc>
#include <l4/sys/factory>
#include <l4/re/util/meta>
#include <l4/re/console>

#include <l4/mag/server/user_state>
#include <cstdio>
#include <cstring>


namespace Mag_server {

using L4Re::chksys;
using L4Re::chkcap;
using L4Re::Util::Auto_cap;

enum { Bar_height = 16 };

Client_fb::Client_fb(Core_api const *core)
: View(Rect(), F_need_frame),
  Icu_svr(1, &_ev_irq),
  _core(core), _fb(0),
  _bar_height(Bar_height),
  _flags(0)
{}

void
Client_fb::set_geometry_prop(Session *_s, Property_handler const *, cxx::String const &v)
{
  Client_fb *s = static_cast<Client_fb*>(_s);
  // ignore multiple geometry properties
  if (s->_fb)
    return;

  int w, h, x=50, y=50;
  int r;

  cxx::String a = v;

  r = a.from_dec(&w);
  if (r >= a.len() || a[r] != 'x')
    L4Re::chksys(-L4_EINVAL, "invalid geometry format");

  a = a.substr(r + 1);
  r = a.from_dec(&h);

  if (r < a.len() && a[r] == '+')
    {
      a = a.substr(r + 1);
      r = a.from_dec(&x);
    }

  if (r < a.len() && a[r] == '+')
    {
      a = a.substr(r + 1);
      r = a.from_dec(&y);
    }

  if (w <= 0 || h <= 0 || w >= 10000 || h >= 10000)
    L4Re::chksys(-L4_ERANGE, "invalid geometry (too big)");

  Area sz = s->_core->user_state()->vstack()->canvas()->size();

  if (x < 10 - w)
    x = 10 - w;

  if (x >= sz.w())
    x = sz.w() - 10;

  if (y < 10 - h)
    y = 10 - h;

  if (y >= sz.h())
    y = sz.h() - 10;

  s->set_geometry(Rect(Point(x, y), Area(w, h)));
}

void
Client_fb::set_flags_prop(Session *_s, Property_handler const *p, cxx::String const &)
{
  Client_fb *s = static_cast<Client_fb*>(_s);

  if (!strcmp(p->tag, "focus"))
    s->_flags |= F_fb_focus;

  if (!strcmp(p->tag, "shaded"))
    s->_flags |= F_fb_shaded;

  if (!strcmp(p->tag, "fixed"))
    s->_flags |= F_fb_fixed_location;
}

void
Client_fb::set_bar_height_prop(Session *_s, Property_handler const *, cxx::String const &v)
{
  Client_fb *s = static_cast<Client_fb*>(_s);
  int r = v.from_dec(&s->_bar_height);
  if (r < v.len())
    L4Re::chksys(-L4_EINVAL, "invalid bar height format");

  s->_bar_height = std::max(std::min(s->_bar_height, 100), 4);
}

int
Client_fb::setup()
{
  using L4Re::Video::View;
  using L4Re::Video::Color_component;
  using L4Re::Video::Goos;

  Area res(size());

  Auto_cap<L4Re::Dataspace>::Cap ds(
      L4Re::Util::cap_alloc.alloc<L4Re::Dataspace>());

  Screen_factory *sf = dynamic_cast<Screen_factory*>(_core->user_state()->vstack()->canvas()->type()->factory);
  //Screen_factory *sf = dynamic_cast<Screen_factory*>(Rgb16::type()->factory);

  L4Re::chksys(L4Re::Env::env()->mem_alloc()->alloc(sf->get_texture_size(res),
                                                    ds.get()));

  L4Re::Rm::Auto_region<void *> dsa;
  L4Re::chksys(L4Re::Env::env()->rm()->attach(&dsa, ds->size(), L4Re::Rm::Search_addr, ds.get(), 0, L4_SUPERPAGESHIFT));

  _fb = sf->create_texture(res, dsa.get());

  set_geometry(Rect(p1(), visible_size()));
  dsa.release();
  _fb_ds = ds.release();

  if (_flags & F_fb_focus)
    _core->user_state()->set_focus(this);

  _view_info.flags = View::F_none;

  _view_info.view_index = 0;
  _view_info.xpos = 0;
  _view_info.ypos = 0;
  _view_info.width = _fb->size().w();
  _view_info.height = _fb->size().h();
  _view_info.buffer_offset = 0;
  _view_info.buffer_index = 0;
  _view_info.bytes_per_line = _view_info.width * _fb->type()->bytes_per_pixel();
  _view_info.pixel_info = *_fb->type();

  _screen_info.flags = Goos::F_pointer;
  _screen_info.width = _view_info.width;
  _screen_info.height = _view_info.height;
  _screen_info.num_static_views = 1;
  _screen_info.num_static_buffers = 1;
  _screen_info.pixel_info = _view_info.pixel_info;


  L4Re::Env const *e = L4Re::Env::env();
  _ev_ds = L4Re::Util::cap_alloc.alloc<L4Re::Dataspace>();


  chksys(e->mem_alloc()->alloc(L4_PAGESIZE, _ev_ds.get()));
  chksys(e->rm()->attach(&_ev_ds_m, L4_PAGESIZE, L4Re::Rm::Search_addr, _ev_ds.get(), 0, L4_PAGESHIFT));

  _events = L4Re::Event_buffer(_ev_ds_m.get(), L4_PAGESIZE);

  calc_label_sz(_core->label_font());
  return 0;
}

void
Client_fb::draw(Canvas *canvas, View_stack const *, Mode mode) const
{
  /* use dimming in x-ray mode */
  Canvas::Mix_mode op = mode.flat() ? Canvas::Solid : Canvas::Mixed;

  /* is this the currently focused view? */
  Rgb32::Color frame_color = focused() ? Rgb32::White : View::frame_color();

  /* do not dim the focused view in x-ray mode */
  if (mode.xray() && !mode.kill() && focused())
    op = Canvas::Solid;

  /*
   * The view content and label should never overdraw the
   * frame of the view in non-flat Nitpicker modes. The frame
   * is located outside the view area. By shrinking the
   * clipping area to the view area, we protect the frame.
   */
  Clip_guard clip_guard(canvas, *this);

  /*
   * If the clipping area shrinked to zero,
   * we do not process drawing operations.
   */
  if (!canvas->clip_valid()/* || !_session*/)
    return;

  /* draw view content */
  Rgb32::Color mix_color = /*mode.kill() ? kill_color() :*/ session()->color();

  canvas->draw_box(top(_bar_height), Rgb32::Color(56, 68, 88));

  canvas->draw_texture(_fb, mix_color, p1() + Point(0, _bar_height), op);
}

Area
Client_fb::visible_size() const
{
  if (_flags & F_fb_shaded)
    return Area(_fb->size().w(), _bar_height);

  return _fb->size() + Area(0, _bar_height);
}


void
Client_fb::toggle_shaded()
{
  Rect r = *this;
  _flags ^= F_fb_shaded;
  Rect n = Rect(p1(), visible_size());
  set_geometry(n);
  _core->user_state()->vstack()->refresh_view(0, 0, r | n);
}

void
Client_fb::handle_event(L4Re::Event_buffer::Event const &e,
                        Point const &mouse)
{
  static Point left_drag;

  if (e.payload.type == L4RE_EV_ABS && e.payload.code == 1 && left_drag != Point())
    {
      Rect npos = Rect(p1() + mouse - left_drag, size());
      left_drag = mouse;
      _core->user_state()->vstack()->viewport(this, npos, true);
      return;
    }

  Rect bar = top(_bar_height);

  if (e.payload.type == L4RE_EV_KEY)
    {
      View_stack *_stack = _core->user_state()->vstack();
      if (e.payload.code == L4RE_BTN_LEFT && e.payload.value == 1 &&
          !_stack->on_top(this))
	_stack->push_top(this);

      if (e.payload.code == L4RE_BTN_LEFT
          && bar.contains(mouse)
          && !(_flags & F_fb_fixed_location))
	{
	  if (e.payload.value == 1)
	    left_drag = mouse;
	  else if (e.payload.value == 0)
	    left_drag = Point();
	  return;
	}

      if (e.payload.code == L4RE_BTN_MIDDLE
          && bar.contains(mouse)
          && e.payload.value == 1)
        {
          toggle_shaded();
          return;
        }
    }

  if (e.payload.type == L4RE_EV_ABS && e.payload.code <= L4RE_ABS_Y)
    {
      // wait for the following ABS_Y axis
      if (e.payload.type == L4RE_ABS_X)
	return;

      Rect r = (*this - bar).b();
      if (!r.contains(mouse))
	return;

      Point mp = p1() + Point(0, _bar_height);
      mp = Point(_fb->size()).min(Point(0,0).max(mouse - mp));
      L4Re::Event_buffer::Event ne;
      ne.time = e.time;
      ne.payload.type = L4RE_EV_ABS;
      ne.payload.code = L4RE_ABS_X;
      ne.payload.value = mp.x();
      ne.payload.stream_id = e.payload.stream_id;
      _events.put(ne);
      ne.payload.code = L4RE_ABS_Y;
      ne.payload.value = mp.y();
      _events.put(ne);
      _ev_irq.trigger();
      return;
    }

  // no events if window is shaded
  if (_flags & F_fb_shaded)
    return;

  if (_events.put(e))
    _ev_irq.trigger();

}


int
Client_fb::refresh(int x, int y, int w, int h)
{
  _core->user_state()->vstack()->refresh_view(this, 0, Rect(p1() + Point(x, y + _bar_height), Area(w, h)));
  return 0;
}

int
Client_fb::dispatch(l4_umword_t obj, L4::Ipc_iostream &s)
{
  l4_msgtag_t tag;
  s >> tag;

  switch (tag.label())
    {
    case L4::Meta::Protocol:
      return L4Re::Util::handle_meta_request<L4Re::Console>(s);
    case L4_PROTO_IRQ:
      return Icu_svr::dispatch(obj, s);
    case L4Re::Protocol::Goos:
      return L4Re::Util::Video::Goos_svr::dispatch(obj, s);
    case L4Re::Protocol::Event:
	{
	  L4::Opcode op;
	  s >> op;
	  switch (op)
	    {
	    case L4Re::Event_::Get:
	      s << _ev_ds.get();
	      return L4_EOK;
	    default:
	      return -L4_ENOSYS;
	    }
	}
    default:
      return -L4_EBADPROTO;
    }
}


void
Client_fb::destroy()
{
  _core->user_state()->forget_view(this);
  delete _fb;
  _fb = 0;
}

}
