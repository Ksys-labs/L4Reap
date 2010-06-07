/*
 * (c) 2008-2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include <l4/sys/types.h>

class Exception_handler
{
public:
  virtual bool handle(l4_umword_t, void *state);

  static bool do_handle_exception(l4_umword_t, l4_msgtag_t &tag);
  static Exception_handler *handlers[0x100];
  virtual ~Exception_handler() {}
};


