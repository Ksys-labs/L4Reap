/*
 * (c) 2010 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <l4/mag-gfx/canvas>

#include <l4/mag/server/view>

#include <l4/re/util/video/goos_svr>
#include <l4/re/util/event_svr>
#include <l4/cxx/ipc_server>
#include <l4/re/util/cap_alloc>
#include <l4/re/rm>
#include <l4/re/util/icu_svr>
#include <l4/mag/server/plugin>

namespace Mag_server {

class Client_fb
: public View, public Object,
  private L4Re::Util::Video::Goos_svr,
  public L4Re::Util::Icu_cap_array_svr<Client_fb>
{
private:
  typedef L4Re::Util::Icu_cap_array_svr<Client_fb> Icu_svr;
  Core_api const *_core;
  Point _offs;
  Texture *_fb;
  Area _bar_size;

  L4Re::Util::Auto_cap<L4Re::Dataspace>::Cap _ev_ds;
  L4Re::Rm::Auto_region<void*> _ev_ds_m;
  L4Re::Event_buffer _events;
  Irq _ev_irq;

public:
  Client_fb(Core_api const *core, Rect const &pos, Point const &offs,
            Texture *fb, L4::Cap<L4Re::Dataspace> const &fb_ds);

  L4::Cap<void> rcv_cap() const { return _core->rcv_cap(); }

  void set_offs(Point const &offs)
  { _offs = offs; }

  void draw(Canvas *, View_stack const *, Mode) const;
  void handle_event(L4Re::Event_buffer::Event const &e,
                    Point const &mouse);

  int dispatch(l4_umword_t obj, L4::Ipc_iostream &s);
  int refresh(int x, int y, int w, int h);

  void destroy();
};

}
