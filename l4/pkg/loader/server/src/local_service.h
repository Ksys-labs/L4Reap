/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include <l4/cxx/ipc_server>
#include <l4/cxx/string>

namespace Ldr {
class Local_service : public L4::Server_object
{
public:
  virtual L4::Server_object *open(int argc, cxx::String const *argv) = 0;
};
}
