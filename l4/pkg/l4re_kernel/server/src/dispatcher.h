/*
 * (c) 2008-2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include <l4/cxx/ipc_server>

#include "region.h"

class Dispatcher : public L4::Server_object
{
public:
  int dispatch(l4_umword_t obj, L4::Ipc_iostream &ios);

private:
  int handle_callback(L4::Ipc_iostream &ios);
  int handle_exception(L4::Ipc_iostream &ios);
};
