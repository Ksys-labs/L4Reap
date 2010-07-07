/*
 * (c) 2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <l4/mag-gfx/canvas>

namespace Mag_gfx {

void
Canvas::draw_rect(Rect const &r, Rgba32::Color color)
{
  draw_box(r.top(1), color);
  draw_box(r.bottom(1), color);
  draw_box(r.left(1), color);
  draw_box(r.right(1), color);
}

}
