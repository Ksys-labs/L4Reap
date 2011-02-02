/*
 * (c) 2010 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <l4/mag-gfx/clip_guard>

#include "view_stack"
#include "view"

#include <cstdio>
#include <cstring>

namespace Mag_server {

class Redraw_queue
{
public:
  enum { Num_entries = 100 };

  // merge all overlapping rects in the queue
  void merge()
  {
    for (unsigned i = last; i > 0; --i)
      for (unsigned j = 0; j < last; ++j)
	{
	  if (i - 1 == j)
	    continue;

	  if ((_q[j] & _q[i-1]).valid())
	    {
	      _q[j] = _q[j] | _q[i-1];
	      memmove(_q + i - 1, _q + i, (last - i) * sizeof(Rect));
	      --last;
	      break;
	    }
	}
  }

  void q(Rect const &r)
  {
    if (!r.valid())
      return;
    //printf("q[%d,%d - %d,%d]\n", r.x1(), r.y1(), r.x2(), r.y2());
    for (unsigned i = 0; i < last; ++i)
      {
	if ((_q[i] & r).valid())
	  {
	    // merge with overlapping rect
	    _q[i] = _q[i] | r;
	    merge();
	    return;
	  }
      }

    if (last < Num_entries)
      _q[last++] = r; // add new entry
    else
      {
	// queue is full, just merge with the last entry
        _q[last-1] = _q[last - 1] | r;
	merge();
      }
  }

  void clear()
  { last = 0; }

  typedef Rect const *iterator;

  iterator begin() const { return _q; }
  iterator end() const { return _q + last; }

private:
  Rect _q[Num_entries];
  unsigned last;
};

static Redraw_queue rdq;

Rect
View_stack::outline(View const *v) const
{
  if (_mode.flat() || !v->need_frame())
    return *v;
  else
    return v->grow(3);
}

void
View_stack::viewport(View *v, Rect const &pos, bool) const
{
  Rect old = outline(v);

  /* take over new view parameters */
  v->set_geometry(pos);

  Rect compound = old | outline(v);

#if 0
  /* update labels (except when moving the mouse cursor) */
  if (v != top())
    _place_labels(compound);
#endif

  /* update area on screen */
  rdq.q(compound);
//  draw_recursive(top(), 0, /*redraw ? 0 : view->session(),*/ compound);
}

void
View_stack::draw_frame(View const *v) const
{
  if (_mode.flat() || !v->need_frame())
    return;

  Rgb32::Color color = v->focused() ? Rgb32::White : View::frame_color();
  _canvas->draw_rect(v->offset(-3, -3, 3, 3), Rgb32::Black);
  _canvas->draw_rect(v->offset(-2, -2, 2, 2), color);
  _canvas->draw_rect(v->offset(-1, -1, 1, 1), Rgb32::Black);
}

void
View_stack::draw_label(View const *v) const
{
  if (_mode.flat() || !v->need_frame())
    return;
}

void
View_stack::draw_recursive(View const *v, View const *dst, Rect const &rect) const
{
  Rect clipped;

  /* find next view that intersects with the current clipping rectangle */
  for ( ; v && !(clipped = outline(v) & rect).valid(); )
    v = v->next();

  if (!v)
    return;

  Rect_tuple border = rect - clipped;

  View const *n = v->next();
  if (n && border.t.valid())
    draw_recursive(n, dst, border.t);
  if (n && border.l.valid())
    draw_recursive(n, dst, border.l);

  if (!dst || dst == v || v->transparent())
    {
      Clip_guard g(_canvas, clipped);
      draw_frame(v);
      v->draw(_canvas, this, _mode);
      draw_label(v);
    }

  if (n && border.r.valid())
    draw_recursive(n, dst, border.r);
  if (n && border.b.valid())
    draw_recursive(n, dst, border.b);
}

void
View_stack::refresh_view(View const *v, View const *dst, Rect const &rect) const
{
  (void)dst;
  Rect r = rect;
  if (v)
    r = r & outline(v);

  rdq.q(r);
  //draw_recursive(top(), dst, r);
}

void
View_stack::flush()
{
  for (Redraw_queue::iterator i = rdq.begin(); i != rdq.end(); ++i)
    {
      draw_recursive(top(), 0, *i);
      if (_canvas_view)
	_canvas_view->refresh(i->x1(), i->y1(), i->w(), i->h());
    }

  rdq.clear();
}

void
View_stack::stack(View *v, View *pivot, bool behind)
{
  remove(v);
  if (behind)
    insert_after(v, pivot);
  else
    insert_before(v, pivot);

  refresh_view(v, 0, outline(v));
}


View *
View_stack::find(Point const &pos) const
{
  View *n = top()->next();
  while (n && !n->contains(pos))
    n = n->next();

  return n;
}

}
