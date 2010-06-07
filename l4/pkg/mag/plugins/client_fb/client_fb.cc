/*
 * (c) 2010 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "client_fb.h"

#include <l4/mag-gfx/clip_guard>
#include <l4/mag-gfx/texture>

#include <l4/mag/server/view_stack>

#include <l4/re/env>
#include <l4/re/event_enums.h>
#include <l4/re/error_helper>
#include <l4/sys/factory>
#include <l4/re/util/meta>
#include <l4/re/console>

#include <l4/mag/server/user_state>
#include <cstdio>


namespace Mag_server {

using L4Re::chksys;
using L4Re::chkcap;

Client_fb::Client_fb(Core_api const *core, Rect const &pos, Point const &offs,
                     Texture *fb, L4::Cap<L4Re::Dataspace> const &fb_ds)
: View(pos, F_need_frame),
  Icu_svr(1, &_ev_irq),
  _core(core), _offs(offs), _fb(fb),
  _bar_size(pos.w(), 16)
{
  using L4Re::Video::View;
  using L4Re::Video::Color_component;
  using L4Re::Video::Goos;

  _fb_ds = fb_ds;
  _view_info.flags = View::F_none;

  _view_info.view_index = 0;
  _view_info.xpos = 0;
  _view_info.ypos = 0;
  _view_info.width = fb->size().w();
  _view_info.height = fb->size().h();
  _view_info.buffer_offset = 0;
  _view_info.buffer_index = 0;
  _view_info.bytes_per_line = _view_info.width * fb->type()->bytes_per_pixel();
  _view_info.pixel_info = *fb->type();

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
}

void
Client_fb::draw(Canvas *canvas, View_stack const *, Mode mode, bool focused) const
{
  /* use dimming in x-ray mode */
  Canvas::Mix_mode op = mode.flat() ? Canvas::Solid : Canvas::Mixed;

  /* is this the currently focused view? */
  Rgb32::Color frame_color = focused ? Rgb32::White : View::frame_color();

  /* do not dim the focused view in x-ray mode */
  if (mode.xray() && !mode.kill() && focused)
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
  Rgb32::Color mix_color = mode.kill() ? kill_color() : Rgb32::Black;

  canvas->draw_box(Rect(p1(), _bar_size), Rgb32::Color(56, 68,88));

  canvas->draw_texture(_fb, mix_color, _offs + p1() + Point(0, _bar_size.h()), op);

#if 0
  if (mode.flat())
    return;

  /* draw label */
  draw_label(canvas, _label_rect.p1(), _session->label(), WHITE, _title, frame_color);
#endif
}

void
Client_fb::handle_event(L4Re::Event_buffer::Event const &e,
                        Point const &mouse)
{
  static Point left_drag;

  if (e.payload.type == L4RE_EV_MAX && left_drag != Point())
    {
      Rect npos = Rect(p1() + mouse - left_drag, size());
      left_drag = mouse;
      _core->user_state()->vstack()->viewport(this, npos, true);
      return;
    }

  if (e.payload.type == L4RE_EV_KEY)
    {
      View_stack *_stack = _core->user_state()->vstack();
      if (e.payload.code == L4RE_BTN_LEFT && e.payload.value == 1 &&
          !_stack->on_top(this))
	_stack->push_top(this);

      if (e.payload.code == L4RE_BTN_LEFT && Rect(p1(), _bar_size).contains(mouse))
	{
	  if (e.payload.value == 1)
	    left_drag = mouse;
	  else if (e.payload.value == 0)
	    left_drag = Point();
	  return;
	}
    }

  if (e.payload.type == L4RE_EV_MAX)
    {
      Rect r = (*this - Rect(p1(), _bar_size)).b;
      if (!r.contains(mouse))
	return;

      L4Re::Event_buffer::Event ne;
      ne.time = e.time;
      ne.payload.type = L4RE_EV_ABS;
      ne.payload.code = L4RE_ABS_X;
      ne.payload.value = mouse.x() - p1().x();
      ne.payload.stream_id = 0;
      _events.put(ne);
      ne.payload.code = L4RE_ABS_Y;
      ne.payload.value = mouse.y() - p1().y() - _bar_size.h();
      _events.put(ne);
      _ev_irq.trigger();
      return;
    }

  if (_events.put(e))
    _ev_irq.trigger();

}


int
Client_fb::refresh(int x, int y, int w, int h)
{
  _core->user_state()->vstack()->refresh_view(this, 0, Rect(p1() + Point(x, y + _bar_size.h()), Area(w, h)));
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
