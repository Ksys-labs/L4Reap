/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include <l4/sys/capability>
#include <l4/cxx/ipc_server>
#include <l4/cxx/string>

#include "local_service.h"
#include <cstring>

namespace Ldr {

class Log : public L4::Server_object
{
private:
  enum { Max_tag = 8 };
  char _tag[Max_tag];
  unsigned char _l;
  unsigned char _color;
  bool _in_line;

public:
  Log() : _l(0), _color(0), _in_line(false) {}
  void set_tag(cxx::String const &tag)
  {
    _l = cxx::min<unsigned long>(tag.len(), Max_tag);
    memcpy(_tag, tag.start(), _l);
  }

  void set_color(unsigned char color)
  { _color = color; }

  char const *tag() const { return _tag; }
  unsigned char color() const { return _color; }

  int dispatch(l4_umword_t obj, L4::Ipc::Iostream &ios);

  static int color_value(cxx::String const &col);
  virtual ~Log() {}
};
}
