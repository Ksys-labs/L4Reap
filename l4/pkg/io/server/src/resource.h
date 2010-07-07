/*
 * (c) 2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *          Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include <l4/sys/icu>

#include <l4/vbus/vbus_types.h>
#include <list>

#include <l4/re/dataspace>
#include <l4/re/util/cap_alloc>
#include <l4/re/rm>

#include "res.h"

class Resource;
class Device;

template< typename T >
class List;

class __List
{
protected:
  struct I
  {
    void *r;
    I *n;
  };

  I *_f;

public:

  template< typename T >
  class iterator
  {
  private:
    friend class List<T>;
    I *_c;
    explicit iterator(I *c = 0) : _c(c) {}

  public:
    T *operator * () const { return (T*)(_c->r); }
    T *operator -> () const { return (T*)(_c->r); }

    iterator operator ++ () { _c = _c->n; return *this; }
    iterator operator ++ (int) { iterator u = *this; _c = _c->n; return u; }

    bool operator == (iterator o) const { return _c == o._c; }
    bool operator != (iterator o) const { return _c != o._c; }
  };

  __List() : _f(0) {}


  unsigned size() const
  {
    unsigned n = 0;
    for (I *x = _f; x; x = x->n)
      ++n;

    return n;
  }

  void *elem(unsigned idx) const
  {
    for (I *x = _f; x; x = x->n, --idx)
      if (!idx)
	return x->r;

    return 0;
  }

  void insert(void *r)
  {
    I *n = new I;
    n->r = r;
    n->n = _f;
    _f = n;
  }

};

template< typename T >
class List : private __List
{
public:

  using __List::size;

  typedef __List::iterator<T> iterator;
  
  iterator begin() const { return iterator(_f); }
  iterator end() const { return iterator(0); }
  T *operator [] (unsigned idx) const
  { return (T*)__List::elem(idx); }

  T *elem(unsigned idx) const
  { return (T*)__List::elem(idx); }

  void insert(T *r)
  { __List::insert(r); }

};

typedef List<Resource> Resource_list;


class Resource_space
{
public:
  virtual bool request(Resource *parent, Device *pdev,
                       Resource *child, Device *cdev) = 0;
  virtual bool alloc(Resource *parent, Device *pdev,
                     Resource *child, Device *cdev, bool resize) = 0;
  virtual ~Resource_space() {}
};

class Resource
{
private:
  unsigned long _f;
  Resource *_p;

public:
  enum Type
  {
    Invalid_res = L4VBUS_RESOURCE_INVALID,
    Irq_res     = L4VBUS_RESOURCE_IRQ,
    Mmio_res    = L4VBUS_RESOURCE_MEM,
    Io_res      = L4VBUS_RESOURCE_PORT,
    Bus_res
  };

  enum Flags
  {
    F_type_mask    = 0x00ff,
    F_disabled     = 0x0100,
    F_hierarchical = 0x0200,
    F_prefetchable = 0x0400,
    F_size_aligned = 0x0800,
    F_empty        = 0x1000,
    F_rom          = 0x2000,
    F_fixed_size   = 0x4000,
    F_fixed_addr   = 0x8000,

    F_width_64bit   = 0x010000,
    F_relative      = 0x040000,
    Mmio_data_space = 0x080000,

    Irq_info_base   = 0x100000,
    Irq_info_factor = Irq_info_base / 2,
    Irq_level       = L4_IRQ_F_LEVEL * Irq_info_factor, //0x000000,
    Irq_edge        = L4_IRQ_F_EDGE  * Irq_info_factor, //0x100000,
    Irq_high        = L4_IRQ_F_POS   * Irq_info_factor, //0x000000,
    Irq_low         = L4_IRQ_F_NEG   * Irq_info_factor, //0x200000,
  };

  explicit Resource(unsigned long flags = 0)
  : _f(flags | F_fixed_size), _p(0) {}

  unsigned long flags() const { return _f; }
  void add_flags(unsigned long flags) { _f |= flags; }
  void del_flags(unsigned long flags) { _f &= ~flags; }
  bool hierarchical() const { return _f & F_hierarchical; }
  bool disabled() const { return _f & F_disabled; }
  bool prefetchable() const { return _f & F_prefetchable; }
  bool empty() const { return _f & F_empty; }
  bool fixed_addr() const { return _f & F_fixed_addr; }
  bool fixed_size() const { return _f & F_fixed_size; }
  bool relative() const { return _f & F_relative; }
  unsigned type() const { return _f & F_type_mask; }

  void set_empty(bool empty)
  {
    if (empty)
      _f |= F_empty;
    else
      _f &= ~F_empty;
  }

  void disable() { _f |= F_disabled; }
  void enable()  { _f &= ~F_disabled; }

  virtual Resource_space *provided() const { return 0; }
  virtual l4_addr_t alignment() const { return 0; }

  virtual void dump(int indent = 0) const = 0;
  virtual bool compatible(Resource *consumer, bool pref = true) const
  {
    if (type() != consumer->type())
      return false;

    return prefetchable() == (consumer->prefetchable() && pref);
  }

  Resource *parent() const { return _p; }
  void parent(Resource *p) { _p = p; }

  virtual l4_addr_t map_iomem() const { return 0; }

  virtual ~Resource() {}
};


class Adr_resource : public Resource
{
public:
  typedef l4_addr_t Addr;
  typedef l4_int64_t Size;
  class Data
  {
  private:
    Addr _s, _e, _a;

  public:
    Data() {}
    Data(Addr s, Addr e, Addr a) : _s(s), _e(e), _a(a) {}

    Addr start() const { return _s; }
    Addr end() const { return _e; }
    Size size() const { return (Size)_e + 1 - _s; }
    Addr alignment() const { return _a; }

    void start_end(Addr s, Addr e) { _s = s; _e = e; }
    void start(Addr start) { _e = start + (_e - _s); _s = start; }
    void end(Addr end) { _e = end; }
    void alignment(Addr a) { _a = a; }
  };

  enum
  {
    Max_addr = ~Addr(0)
  };


private:
  Data _d;

public:

  explicit Adr_resource(unsigned long flags = 0)
  : Resource(flags), _d(0, 0, 0)
  {}

  Adr_resource(unsigned long flags, Addr start, Addr end)
  : Resource(flags),
    _d(start, end, end - start)
  {}

  void set_empty() { _d.start_end(0, 0); Resource::set_empty(true); }

  void dump(int indent = 0) const;

  void alignment(Size a)
  {
    _d.alignment(a);
    del_flags(F_size_aligned);
  }

  void validate()
  {
    if (!valid())
      disable();
  }

  Data data() const { return _d; }
  Data const &_data() const { return _d; }

  Addr start() const { return _d.start(); }
  Addr end() const { return _d.end(); }
  Size size() const { return _d.size(); }

  bool valid() const { return flags() && _d.start() <= _d.end(); }

  void start(Addr start) { _d.start(start); }
  void end(Addr end)
  {
    _d.end(end);
    Resource::set_empty(false);
  }

  virtual void size(Size size)
  {
    _d.end(_d.start() - 1 + size);
    Resource::set_empty(false);
  }

  void start_size(Addr start, Size s)
  {
    _d.start_end(start, start - 1 + s);
    Resource::set_empty(false);
  }

  void start_end(Addr start, Addr end)
  {
    _d.start_end(start, end);
    Resource::set_empty(false);
  }

  bool is_64bit() const { return flags() & F_width_64bit; }
  l4_addr_t alignment() const
  {
    return  flags() & F_size_aligned ? (_d.end() - _d.start()) : _d.alignment();
  }


  l4_addr_t map_iomem() const
  {
    if (type() != Mmio_res)
      return 0;
    return res_map_iomem(start(), size());
  }

  virtual ~Adr_resource() {}
};



class Adr_resource_provider : public Adr_resource
{
private:
  class _RS : public Resource_space
  {
  private:
    typedef Adr_resource::Addr Addr;
    typedef Adr_resource::Size Size;
    typedef std::list<Adr_resource*> Rl;

    Rl _rl;

  public:
    bool request(Resource *parent, Device *pdev, Resource *child, Device *cdev);
    bool alloc(Resource *parent, Device *pdev, Resource *child, Device *cdev,
               bool resize);

    ~_RS() {}
  };

  mutable _RS _rs;

public:
  explicit Adr_resource_provider(unsigned long flags)
  : Adr_resource(flags), _rs() {}

  Adr_resource_provider(unsigned long flags, Addr s, Addr e)
  : Adr_resource(flags, s, e), _rs() {}

  Resource_space *provided() const
  { return &_rs; }

  ~Adr_resource_provider() {}

};

class Root_resource : public Resource
{
private:
  Resource_space *_rs;

public:
  Root_resource(unsigned long flags, Resource_space *rs)
  : Resource(flags), _rs(rs) {}

  Resource_space *provided() const { return _rs; }
  void dump(int) const {}

  ~Root_resource() {}
};


class Mmio_data_space : public Adr_resource
{
private:
  L4Re::Util::Auto_cap<L4Re::Dataspace>::Cap _ds_ram;

public:
  L4Re::Rm::Auto_region<l4_addr_t> _r;

  Mmio_data_space(Size size, unsigned long alloc_flags = 0)
  : Adr_resource(Mmio_res, 0, size - 1)
  {
    alloc_ram(size, alloc_flags);
  }

  void alloc_ram(Size size, unsigned long alloc_flags);

  l4_addr_t map_iomem() const
  {
    return _r.get();
  }
};
