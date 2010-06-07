/*
 * (c) 2010 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <l4/mag/server/input_driver>

#include <l4/input/libinput.h>

namespace {
using Mag_server::Input_driver;
using Mag_server::User_state;
using Mag_server::Motion_fwd;

struct Emit
{
  User_state *u;
  Emit(User_state *u) : u(u) {}
  void operator () (L4Re::Event_buffer::Event const &e) const
  { u->handle_event(e); }
};

class Input_driver_libinput : public Input_driver
{
public:
  Input_driver_libinput() : Input_driver("libinput") {}
  int probe()
  {
    if (l4input_init(0xff, 0) == 0)
      return 0;

    return 1;
  }

  void poll_events()
  {
    if (!l4input_ispending())
      return;

    enum { N=20 };
    L4Re::Event_buffer::Event _evb[N];

    int max = l4input_flush(_evb, N);
    //Motion_merger<10> mm;
    Motion_fwd mm;
    mm.process/*<L4Re::Event_buffer::Event>*/(&_evb[0], _evb + max, Emit(_core->user_state()));
  }
};

static Input_driver_libinput _libinput;
}
