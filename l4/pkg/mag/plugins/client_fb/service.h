/*
 * (c) 2010 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include <l4/cxx/ipc_server>
#include <l4/re/video/goos>

#include <l4/mag/server/plugin>
#include <l4/mag/server/user_state>

namespace Mag_server {

class Service : public Object, private Plugin
{
private:
  Core_api const *_core;

protected:
  User_state *ust() const { return _core->user_state(); }
  Registry *reg() const { return _core->registry(); }
  int create(char const *msg, L4::Ipc::Iostream &ios);

public:
  Service(char const *name) : Plugin(name) {}
  char const *type() const { return "service"; }
  void start(Core_api *core);

  //Canvas *screen() const { return _ust.vstack()->canvas(); }

  int dispatch(l4_umword_t obj, L4::Ipc::Iostream &ios);

  void destroy();
  ~Service();
};

}

