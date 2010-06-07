/*
 * (c) 2010 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "user_state"

#include <l4/re/event_enums.h>

namespace Mag_server {

void
User_state::handle_event(Event const &e)
{
  bool update_all = false;

  if (e.payload.type == L4RE_EV_REL)
    {
      switch (e.payload.code)
	{
	case L4RE_REL_X:
	  _next_mouse_pos += Point(e.payload.value, 0);
	  break;
	case L4RE_REL_Y:
	  _next_mouse_pos += Point(0, e.payload.value);
	  break;
	}
      Rect scr(Point(0, 0), _vstack.canvas()->size());
      _next_mouse_pos = _next_mouse_pos.min(scr.p2());
      _next_mouse_pos = _next_mouse_pos.max(scr.p1());
    }
  else if (e.payload.type == L4RE_EV_ABS)
    {
      switch (e.payload.code)
	{
	case L4RE_ABS_X:
	  _next_mouse_pos = Point(e.payload.value * vstack()->canvas()->size().w() / 0x7fff, _next_mouse_pos.y());
	  break;
	case L4RE_ABS_Y:
	  _next_mouse_pos = Point(_next_mouse_pos.x(), e.payload.value * vstack()->canvas()->size().h() / 0x7fff);
	  break;
	}
      Rect scr(Point(0, 0), _vstack.canvas()->size());
      _next_mouse_pos = _next_mouse_pos.min(scr.p2());
      _next_mouse_pos = _next_mouse_pos.max(scr.p1());
    }
  else if (e.payload.type == L4RE_EV_KEY)
    {
      switch (e.payload.value)
	{
	case 0: /* release */
	  if (_pressed_keys > 0)
	    --_pressed_keys;
	  break;

	case 1: /* press */
	  if (_pressed_keys == 0 && _pointed_view != _vstack.focused()
	      && e.payload.code >= L4RE_BTN_MOUSE && e.payload.code <= L4RE_BTN_EXTRA)
	    {
	      _vstack.set_focused(_pointed_view);
	      update_all = true;
	    }
	  ++_pressed_keys;

	  if (e.payload.code == L4RE_KEY_SCROLLLOCK)
	    {
	      _vstack.toggle_mode(Mode::X_ray);
	      update_all = true;
	    }

	  if (e.payload.code == L4RE_KEY_PRINT || e.payload.code == L4RE_KEY_SYSRQ)
	    {
	      _vstack.toggle_mode(Mode::Kill);
	      update_all = true;
	    }

	  break;

	default:
	  break;
	}
    }

  if (1 || e.payload.type == L4RE_EV_SYN)
    {
      if (_mouse_pos != _next_mouse_pos)
	{
	  _mouse_pos = _next_mouse_pos;
	  _pointed_view = _vstack.find(_mouse_pos);
	  vstack()->viewport(_mouse_cursor, Rect(_mouse_pos, _mouse_cursor->size()), true);
	}
    }

  if (!_pointed_view)
    _pointed_view = _vstack.find(_next_mouse_pos);

  if (update_all)
    _vstack.update_all_views();


  if (_vstack.mode().kill())
    return;

  if (e.payload.type == L4RE_EV_KEY)
    if (_vstack.focused())
      _vstack.focused()->handle_event(e, _mouse_pos);

  if (e.payload.type == L4RE_EV_ABS || e.payload.type == L4RE_EV_REL)
    {
      if (e.payload.code == L4RE_ABS_X || e.payload.code == L4RE_ABS_Y)
	{
	  Event me;
	  me.time = e.time;
	  me.payload.type = L4RE_EV_MAX;

	  if (_pressed_keys == 0)
	    {
	      if (_pointed_view)
		_pointed_view->handle_event(me, _mouse_pos);
	    }
	  else if (_vstack.focused())
	    _vstack.focused()->handle_event(me, _mouse_pos);
	}
      else if (_vstack.focused())
	_vstack.focused()->handle_event(e, _mouse_pos);
    }

}

}
