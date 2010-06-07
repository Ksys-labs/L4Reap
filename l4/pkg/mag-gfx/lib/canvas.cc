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
