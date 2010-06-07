/*
 * (c) 2010 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include <l4/re/util/object_registry>

extern L4Re::Util::Object_registry *registry;
extern L4::Cap<void> rcv_cap;
int server_loop();
