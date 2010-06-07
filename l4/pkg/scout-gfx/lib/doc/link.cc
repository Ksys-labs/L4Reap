#include <l4/scout-gfx/doc/link>

namespace Scout_gfx {

/****************
 ** Link_token **
 ****************/

Browser *
Link_token::browser() const
{
  for (Widget *e = parent(); e; e = e->parent())
    if (Browser *b = dynamic_cast<Browser*>(e))
      return b;

  return 0;
}


void Link_token::handle_event(Event const &e)
{
  Token::handle_event(e);
  if (e.type != Event::PRESS)
    return;

  /* make browser to follow link */
  Browser *b = browser();
  if (b && _dst)
    b->go_to(_dst);
}


void
Link_token::mfocus(int flag)
{
  Token::mfocus(flag);
  Token *x = this;
  do
    {
      Link_token *lt = static_cast<Link_token*>(x);
      if (flag && lt->_dst_value != _MAX_ALPHA)
	lt->fade_to(_MAX_ALPHA, 50);

      if (!flag && lt->_dst_value != 0)
	lt->fade_to(0, 2);

      x = lt->Token::_next;
    }
  while (x != this);
}

}
