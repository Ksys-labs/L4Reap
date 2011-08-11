/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include <l4/sys/capability>
#include <l4/cxx/ipc_server>

namespace Moe {

class Log : public Moe::Server_object
{
private:
  char const *_tag;
  unsigned long _l;
  unsigned char _color;
  bool _in_line;

public:
  Log() : _tag(0), _l(0), _color(0), _in_line(false) {}
  void set_tag(char const *tag, int len)
  { _tag = tag; _l = len; }
  void set_color(unsigned char color)
  { _color = color; }

  char const *tag() const { return _tag; }
  unsigned char color() const { return _color; }

  int dispatch(l4_umword_t obj, L4::Ipc::Iostream &ios);

  virtual ~Log() {}

  static int color_value(cxx::String const &col);
};
}
