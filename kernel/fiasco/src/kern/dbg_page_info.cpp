INTERFACE [debug]:

#include "types.h"
#include "lock_guard.h"
#include "spin_lock.h"
#include "slab_cache_anon.h"

class Dbg_page_info_table;

class Dbg_page_info
{
  friend class Dbg_page_info_table;

private:
  Page_number const _pfn;
  Dbg_page_info *_n;
  typedef unsigned long Buf[5];
  Buf _buf;

  char *b() { return reinterpret_cast<char*>(_buf); }
  char const *b() const { return reinterpret_cast<char const*>(_buf); }

  typedef slab_cache_anon Allocator;

public:
  void *operator new (size_t) { return alloc()->alloc(); }
  void operator delete (void *p, size_t) { alloc()->free(p); }

  enum { Buffer_size = sizeof(Buf) };

  Dbg_page_info(Page_number pfn) : _pfn(pfn), _n(0) {}

  bool match(Page_number p) { return _pfn == p; }

  template<typename T>
  T *info()
  { return reinterpret_cast<T*>(b()); }

  template<typename T>
  T const *info() const
  { return reinterpret_cast<T const*>(b()); }
};

class Dbg_page_info_table
{
public:
  typedef Spin_lock_coloc<Dbg_page_info*> Entry;
  enum { Hash_tab_size = 1024 };

private:
  Entry _tab[Hash_tab_size];
  static unsigned hash(Page_number p) { return p.value() % Hash_tab_size; }
};



IMPLEMENTATION [debug]:

#include "kmem_slab_simple.h"


static Dbg_page_info_table _t;

PUBLIC static
Dbg_page_info_table &
Dbg_page_info::table()
{
  return _t;
}

static Kmem_slab_t<Dbg_page_info> _dbg_page_info_allocator("Dbg_page_info");

PRIVATE static
Dbg_page_info::Allocator *
Dbg_page_info::alloc()
{ return &_dbg_page_info_allocator; }


PUBLIC static
Dbg_page_info *
Dbg_page_info::find(Dbg_page_info *i, Page_number p)
{
  for (; i; i = i->_n)
    if (i->match(p))
      return i;
  return 0;
}

PUBLIC
Dbg_page_info *
Dbg_page_info_table::operator [] (Page_number pfn) const
{
  Entry &e = const_cast<Dbg_page_info_table*>(this)->_tab[hash(pfn)];
  Lock_guard<Entry> g(&e);
  return Dbg_page_info::find(e.get_unused(), pfn);
}

PUBLIC
void
Dbg_page_info_table::insert(Dbg_page_info *i)
{
  Entry *e = &_tab[hash(i->_pfn)];
  Lock_guard<Entry> g(e);
  i->_n = e->get_unused();
  e->set_unused(i);
}

PUBLIC
Dbg_page_info *
Dbg_page_info_table::remove(Page_number pfn)
{
  Entry *e = &_tab[hash(pfn)];
  Lock_guard<Entry> g(e);
  Dbg_page_info *i = e->get_unused();

  if (!i)
    return 0;

  if (i->match(pfn))
    {
      e->set_unused(i->_n);
      return i;
    }

  for (; i->_n; i = i->_n)
    if (i->_n->match(pfn))
      {
	Dbg_page_info *r = i->_n;
	i->_n = i->_n->_n;
	return r;
      }

  return 0;
}
