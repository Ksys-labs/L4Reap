
#include <l4/scout-gfx/box_layout>
#include <l4/scout-gfx/doc/item>
#include <l4/scout-gfx/style>

#include <l4/mag-gfx/canvas>


Scout_gfx::Item::Item(Style *style, const char *str, int indent)
: _indent(indent), _tag(str), _style(style)
{
  set_child_layout(Box_layout::create_vert());
}


void
Scout_gfx::Item::draw(Canvas *c, Point const &p)
{
  c->draw_string(p, _style->font, _style->color, _tag);
  Parent_widget::draw(c, p);
}
