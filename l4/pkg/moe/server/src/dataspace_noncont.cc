/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "dataspace_noncont.h"
#include "page_alloc.h"
#include "slab_alloc.h"
#include "pages.h"

#include <l4/sys/task.h>

#include <l4/cxx/iostream>
#include <l4/cxx/minmax>
#include <l4/cxx/exceptions>
#include <cstring>

using cxx::min;

typedef Moe::Q_alloc_static<Single_page_alloc_base> Page_alloc;

void
Moe::Dataspace_noncont::unmap_page(Page const &p, bool ro) const throw()
{
  if (p.valid())
    l4_task_unmap(L4_BASE_TASK_CAP,
                  l4_fpage((unsigned long)*p, page_shift(),
                  ro ? L4_FPAGE_W : L4_FPAGE_RWX), L4_FP_OTHER_SPACES);
}

void
Moe::Dataspace_noncont::free_page(Page &p) const throw()
{
  unmap_page(p);
  if (p.valid() && !Moe::Pages::unshare(*p))
    {
      //L4::cout << "free page @" << *p << '\n';
      Page_alloc::_free(quota(), *p, page_size());
    }

  p.set(0,0);
}

Moe::Dataspace::Address
Moe::Dataspace_noncont::address(l4_addr_t offset,
                                Ds_rw rw, l4_addr_t,
                                l4_addr_t, l4_addr_t) const
{
  // XXX: There may be a problem with data spaces with
  //      page_size() > L4_PAGE_SIZE
  //      MUST review that!!
  if (!check_limit(offset))
    return Address(-L4_ERANGE);

  Page &p = alloc_page(offset);

  if (!is_writable())
    rw = Read_only;

  if ((rw == Writable) && (p.flags() & Page_cow))
    {
      if (Moe::Pages::ref_count(*p) == 1)
	p.set(*p, p.flags() & ~Page_cow);
      else
	{
	  void *np = Page_alloc::_alloc(quota(), page_size(), page_size());
	  Moe::Pages::share(np);

	  // L4::cout << "copy on write for " << *p << " to " << np << '\n';
	  memcpy(np, *p, page_size());
	  unmap_page(p);
	  Moe::Pages::unshare(*p);
	  p.set(np, 0);
	}
    }

  if (!*p)
    {
      p.set(Page_alloc::_alloc(quota(), page_size(), page_size()),0);
      Moe::Pages::share(*p);
      memset(*p, 0, page_size());
    }

  return Address(l4_addr_t(*p), page_shift(), rw, offset & (page_size()-1));
}

int
Moe::Dataspace_noncont::pre_allocate(l4_addr_t offset, l4_size_t size, unsigned rights)
{
  l4_size_t ps = page_size();
  for (; size >= ps; size -= ps, offset += ps)
    {
      Address a = address(offset, rights & L4_CAP_FPAGE_W ? Writable : Read_only);
      if (a.is_nil())
	return a.error();
    }
  return 0;
}

void 
Moe::Dataspace_noncont::unmap(bool ro) const throw()
{
  register const unsigned long pgsz = page_size();
  for (unsigned long i = num_pages(); i > 0; --i)
    unmap_page(page((i-1)*pgsz), ro);
}

long 
Moe::Dataspace_noncont::clear(unsigned long offs, unsigned long _size) const throw()
{
  if (!check_limit(offs))
    return -L4_ERANGE;

  unsigned long sz = _size = min(_size, round_size()-offs);
  unsigned long pg_sz = page_size();
  unsigned long pre_sz = offs & (pg_sz-1);
  if (pre_sz)
    {
      pre_sz = min(pg_sz - pre_sz, sz);
      Moe::Dataspace::clear(offs, pre_sz);
      sz -= pre_sz;
      offs += pre_sz;
    }

  unsigned long u_sz = sz & ~(pg_sz-1);

  while (u_sz)
    {
      // printf("ds free page offs %lx\n", offs);
      free_page(page(offs));
      offs += pg_sz;
      u_sz -= pg_sz;
    }

  sz &= (pg_sz-1);

  if (sz)
    Moe::Dataspace::clear(offs, sz);

  return _size;
}

typedef Moe::Q_alloc<Moe::Dataspace_noncont, Slab_alloc> Allocator;

static Allocator *alloc()
{
  static Allocator a;
  return &a;
}

void *Moe::Dataspace_noncont::operator new (size_t, Quota *q)
{
  void *a = alloc()->alloc(q);
  if (a) return a;
  throw L4::Out_of_memory();
}

void Moe::Dataspace_noncont::operator delete (void *m) throw()
{ alloc()->free((Moe::Dataspace_noncont*)m); }

namespace {
  class Mem_one_page : public Moe::Dataspace_noncont
  {
  public:
    Mem_one_page(unsigned long size, unsigned long flags) throw()
    : Moe::Dataspace_noncont(size, flags)
    {}

    ~Mem_one_page() throw()
    { free_page(page(0)); }

    Page &page(unsigned long /*offs*/) const throw()
    { return const_cast<Page &>(reinterpret_cast<Page const &>(pages)); }

    Page &alloc_page(unsigned long /*offs*/) const throw()
    { return const_cast<Page &>(reinterpret_cast<Page const &>(pages)); }
  };

  class Mem_small : public Moe::Dataspace_noncont
  {
    enum
    {
      Meta_align_bits = 10,
      Meta_align      = 1UL << Meta_align_bits,

      Page_shift      = 12,
    };

  public:
    unsigned long meta_size() const throw()
    { return (l4_round_size(num_pages()*sizeof(unsigned long), Meta_align_bits)); }
    Mem_small(unsigned long size, unsigned long flags)
    : Moe::Dataspace_noncont(size, flags)
    {
      pages = (unsigned long*)Page_alloc::_alloc(quota(), meta_size(), Meta_align);
      memset(pages, 0, meta_size());
    }
      
    ~Mem_small() throw()
    {
      register const unsigned long pgsz = page_size();
      for (unsigned long i = num_pages(); i > 0; --i)
	free_page(page((i-1)*pgsz));

      Page_alloc::_free(quota(), pages, meta_size());
    }

    Page &page(unsigned long offs) const throw()
    { return (Page &)(pages[offs >> 12]); }
    
    Page &alloc_page(unsigned long offs) const throw()
    { return (Page &)(pages[offs >> 12]); }

  };
  
  class Mem_big : public Moe::Dataspace_noncont
  {
  public:

    static unsigned long meta2_size() throw()
    { return L4_PAGESIZE; }

    static unsigned long entries2() throw()
    { return meta2_size()/sizeof(unsigned long); }

  
  private:
    class L1
    {
    private:
      unsigned long p;
      
    public:
      Page *l2() const throw() { return (Page*)(p & ~0xfffUL); }
      Page &operator [] (unsigned long offs) throw()
      { return l2()[(offs >> 12) & (entries2()-1)]; }
      Page *operator * () const throw() { return l2(); }
      unsigned long cnt() const throw() { return p & 0xfffUL; }
      void inc() throw() { p = (p & ~0xfffUL) | (((p & 0xfffUL)+1) & 0xfffUL); }
      void dec() throw() { p = (p & ~0xfffUL) | (((p & 0xfffUL)-1) & 0xfffUL); }
      void set(void* _p) throw() { p = (unsigned long)_p; }
    };

    L1 &__p(unsigned long offs) const throw()
    { return ((L1*)pages)[(offs >> 12) / entries2()]; }
    
  public:   
    unsigned long entries1() const throw()
    { return (num_pages() + entries2() - 1)/entries2(); }

    long meta1_size() const throw()
    { return (entries1() * sizeof(unsigned long) + 1023) & ~1023; }

    Mem_big(unsigned long size, unsigned long flags)
    : Moe::Dataspace_noncont(size, flags)
    {
      pages = (unsigned long*)Page_alloc::_alloc(quota(), meta1_size(), 1024);
      memset(pages, 0, meta1_size());
    }

    ~Mem_big() throw()
    {
      for (unsigned long i = 0; i < size(); i+=page_size())
	{
	  free_page(page(i));
	}

      for (unsigned long i = 0; i < size(); i+=page_size()*1024)
	{
	  L1 &p = __p(i); 

	  if (*p)
	    Page_alloc::_free(quota(), *p, meta2_size());
	  p.set(0);
	}

      Page_alloc::_free(quota(), pages, meta1_size());
    }

    Page &page(unsigned long offs) const throw()
    {
      static Page invalid_page;
      if (!__p(offs).cnt())
	return invalid_page;
      
      return __p(offs)[offs];
    }
    
    Page &alloc_page(unsigned long offs) const
    {
      L1 &_p = __p(offs);
      if (!_p.cnt())
	{
	  void *a = Page_alloc::_alloc(quota(), meta2_size(), meta2_size());

	  _p.set(a);
	  memset(a, 0, meta2_size());
	}

      Page &_pa = _p[offs];

      if (!_pa.valid())
	_p.inc();

      return _pa;
    }


  };
};


Moe::Dataspace_noncont *
Moe::Dataspace_noncont::create(Quota *q, unsigned long size,
                               unsigned long flags)
{
  if (size <= L4_PAGESIZE)
    return new (q) Mem_one_page(size, flags);
  else if (size <= L4_PAGESIZE * (L4_PAGESIZE/sizeof(unsigned long)))
    return new (q) Mem_small(size, flags);
  else
    return new (q) Mem_big(size, flags);
}

