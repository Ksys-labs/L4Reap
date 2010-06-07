#include <l4/scout-gfx/document>
#include <l4/scout-gfx/box_layout>
#include <l4/mag-gfx/clip_guard>

namespace Scout_gfx {
  /**
   * Constructor
   */
Document::Document()
: toc(0), title("")
{
  set_child_layout(Box_layout::create_vert());
}

void
Document::append(Widget *e)
{
  Parent_widget::append(e);
  child_layout()->add_item(e);
}

void
Document::remove(Widget *e)
{
  child_layout()->remove_item(e);
  Parent_widget::remove(e);
}


Center::Center(Widget *content)
{
  set_child_layout(Box_layout::create_vert());
  child_layout()->set_alignment(Mag_gfx::Align_h_center);

  if (content)
    append(content);
}


void
Center::append(Widget *c)
{
  Parent_widget::append(c);
  c->set_alignment(Mag_gfx::Align_h_center);
  child_layout()->add_item(c);
}

void
Center::remove(Widget *c)
{
  Parent_widget::remove(c);
  child_layout()->remove_item(c);
}

}
