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

#include <cstddef>
#include <l4/sys/types.h>
#include <l4/cxx/list>
#include <l4/cxx/ipc_server>
#include <l4/cxx/ipc_stream>
#include <l4/re/dataspace>

#include "server_obj.h"
#include "ref_cnt.h"
#include "globals.h"
#include "quota.h"

namespace Moe {

class Dataspace : public Server_object, public Q_object
{
public:

  enum Ds_rw
  {
    Read_only   = L4Re::Dataspace::Map_ro,
    Writable    = L4Re::Dataspace::Map_rw,
    Cow_enabled = 0x100,
  };

  struct Address
  {
    l4_fpage_t fpage;
    l4_addr_t offs;

    Address(long error) throw() : offs(-1UL) { fpage.raw = error; }

    Address(l4_addr_t base, l4_addr_t size, Ds_rw rw = Read_only,
            l4_addr_t offs = 0) throw()
    : fpage(l4_fpage(base, size, rw ? L4_FPAGE_RWX : L4_FPAGE_RX)),
      offs(offs) {}

    unsigned long bs() const throw() { return fpage.raw & ~((1 << 12)-1); }
    unsigned long sz() const throw() { return 1 << l4_fpage_size(fpage); }
    unsigned long of() const throw() { return offs; }
    l4_fpage_t fp() const throw() { return fpage; }

    template< typename T >
    T adr() const throw() { return (T)(bs() + offs); }

    void *adr() const throw() { return (void*)(bs() + offs); }

    bool is_nil() const throw() { return offs == -1UL; }
    /**
     * \brief Get the error code that led to the invalid address.
     * \pre is_nil() must return true.
     */
    long error() const throw() { return fpage.raw; }

  };

  Dataspace(unsigned long size, unsigned long flags = 0) throw()
    : _size(size), _flags(flags)
  {}


  unsigned long size() const throw() { return _size; }
  virtual void unmap(bool ro = false) const throw() = 0;
  virtual Address address(l4_addr_t ds_offset,
                          Ds_rw rw = Writable, l4_addr_t hot_spot = 0,
                          l4_addr_t min = 0, l4_addr_t max = ~0) const = 0;

  virtual int pre_allocate(l4_addr_t offset, l4_size_t size, unsigned rights) = 0;

  unsigned long is_writable() const throw() { return _flags & Writable; }
  unsigned long can_cow() const throw() { return _flags & Cow_enabled; }
  unsigned long flags() const throw() { return _flags; }
  virtual ~Dataspace() {}

  virtual unsigned long page_shift() const throw() = 0;
  unsigned long page_size() const throw() { return 1UL << page_shift(); }

  virtual bool is_static() const throw() = 0;
  virtual long clear(unsigned long offs, unsigned long size) const throw();

protected:
  void size(unsigned long size) throw() { _size = size; }

public:
  unsigned long round_size() const throw()
  { return l4_round_size(size(), page_shift()); }
  bool check_limit(l4_addr_t offset) const throw()
  { return offset < round_size(); }

public:
  int dispatch(l4_umword_t obj, L4::Ipc_iostream &ios);
  int map(l4_addr_t offs, l4_addr_t spot, bool rw,
          l4_addr_t min, l4_addr_t max, L4::Snd_fpage &memory);
  int stats(L4Re::Dataspace::Stats &stats);
  //int copy_in(unsigned long dst_offs, Dataspace *src, unsigned long src_offs,
  //    unsigned long size);
  virtual int phys(l4_addr_t offset, l4_addr_t &phys_addr, l4_size_t &phys_size) throw();

  int dispatch(unsigned long obj, unsigned long op, L4::Ipc_iostream &ios);

private:
  unsigned long _size;
  unsigned long _flags;
};

}



