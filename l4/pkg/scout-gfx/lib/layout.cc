#include <l4/scout-gfx/layout>
#include <cstdio>

using namespace Mag_gfx;

namespace Scout_gfx {

void
Layout::remove_item(Layout_item *i)
{
  Layout_item *c;
  for (int x = 0; (c = item(x)); ++x)
    if (c == i)
      {
        remove_item(x);
	invalidate();
	return;
      }
}



}
