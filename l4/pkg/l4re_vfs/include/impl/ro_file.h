/*
 * (c) 2010 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 *
 * As a special exception, you may use this file as part of a free software
 * library without restriction.  Specifically, if other files instantiate
 * templates or use macros or inline functions from this file, or you compile
 * this file and link it with other files to produce an executable, this
 * file does not by itself cause the resulting executable to be covered by
 * the GNU General Public License.  This exception does not however
 * invalidate any other reasons why the executable file might be covered by
 * the GNU General Public License.
 */
#pragma once

#include <l4/l4re_vfs/backend>
#include "simple_store.h"

namespace L4Re { namespace Core {

class Ro_file : public L4Re::Vfs::Be_file
{
private:
  L4::Cap<L4Re::Dataspace> _ds;
  off64_t _f_pos;
  off64_t _size;
  char const *_addr;

public:
  explicit Ro_file(L4::Cap<L4Re::Dataspace> ds) throw()
  : Be_file(), _ds(ds), _f_pos(0), _addr(0)
  {
    _ds->take();
    _size = _ds->size();
  }

  L4::Cap<L4Re::Dataspace> data_space() const throw() { return _ds; }

  ssize_t readv(const struct iovec*, int iovcnt) throw();
  ssize_t writev(const struct iovec*, int iovcnt) throw();

  off64_t lseek64(off64_t, int) throw();
  int fstat64(struct stat64 *buf) const throw();

  int get_status_flags() const throw()
  { return O_RDONLY; }

  int set_status_flags(long) throw()
  { return 0; }

  ~Ro_file() throw();

  void *operator new(size_t s) throw();
  void operator delete(void *b) throw();

private:

  ssize_t read(const struct iovec*) throw();
//  ssize_t write(const struct iovec*) throw();


private:

  static Simple_store<Ro_file> store;
};


}}
