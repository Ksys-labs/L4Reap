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
#include <l4/mag/server/session>

#include <l4/re/util/video/goos_svr>
#include <l4/re/util/event_svr>
#include <l4/cxx/ipc_server>
#include <l4/re/util/cap_alloc>
#include <l4/re/rm>
#include <l4/re/util/icu_svr>
#include <l4/mag/server/plugin>

namespace Mag_server {

class Client_fb
: public View, public Session, public Object,
  private L4Re::Util::Video::Goos_svr,
  public L4Re::Util::Icu_cap_array_svr<Client_fb>
{
private:
  typedef L4Re::Util::Icu_cap_array_svr<Client_fb> Icu_svr;
  Core_api const *_core;
  Texture *_fb;
  int _bar_height;

  L4Re::Util::Auto_cap<L4Re::Dataspace>::Cap _ev_ds;
  L4Re::Rm::Auto_region<void*> _ev_ds_m;
  L4Re::Event_buffer _events;
  Irq _ev_irq;

  enum
  {
    F_fb_fixed_location = 1 << 0,
    F_fb_shaded         = 1 << 1,
    F_fb_focus          = 1 << 2,
  };
  unsigned _flags;
public:
  int setup();

  explicit Client_fb(Core_api const *core);

  L4::Cap<void> rcv_cap() const { return _core->rcv_cap(); }

  void toggle_shaded();

  void draw(Canvas *, View_stack const *, Mode) const;
  void handle_event(L4Re::Event_buffer::Event const &e,
                    Point const &mouse);

  int dispatch(l4_umword_t obj, L4::Ipc::Iostream &s);
  int refresh(int x, int y, int w, int h);

  Area visible_size() const;

  void destroy();

  Session *session() const { return const_cast<Client_fb*>(this); }

  static void set_geometry_prop(Session *s, Property_handler const *p, cxx::String const &v);
  static void set_flags_prop(Session *s, Property_handler const *p, cxx::String const &v);
  static void set_bar_height_prop(Session *s, Property_handler const *p, cxx::String const &v);
};

}
