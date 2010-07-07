/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

namespace Moe
{

class Ref_cnt_obj
{
public:
  void take() { ++_ref_cnt; }
  unsigned long release() { return --_ref_cnt; }

  unsigned long ref_cnt() const { return _ref_cnt; }

  Ref_cnt_obj() : _ref_cnt(1) {}

private:
  unsigned long _ref_cnt;
};

};
