/*
 * (c) 2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include <cstddef>
#include <cstdio>
#include <gc.h>

namespace Moe {


class Finalized
{
public:
  virtual ~Finalized()
  { GC_register_finalizer_ignore_self(GC_base(this), 0, 0, 0, 0); }

  static void GC_CALLBACK finalizer(void *obj, void *displ)
  { reinterpret_cast<Finalized*>((char *)obj + (ptrdiff_t)displ)->~Finalized(); }

  Finalized()
  {
    void *base = GC_base((void*)this);
    if (base != 0)
      {
	GC_register_finalizer_ignore_self(base, finalizer, (void *)((char *)this - (char *)base), 0, 0);
      }
  }
};

class Quota
{
public:
  explicit Quota(size_t limit) : _limit(limit), _used(0) {}
  bool alloc(size_t s)
  {
    if (_limit && (s > _limit || _used > _limit - s))
      return false;

    _used += s;
    // printf("Q: alloc(%zx) -> %zx\n", s, _used);
    return true;
  }

  void free(size_t s)
  {
    _used -= s;
    //printf("Q: free(%zx) -> %zx\n", s, _used);
  }

  size_t limit() const { return _limit; }
  size_t used() const { return _used; }

private:
  size_t _limit;
  size_t _used;
};

struct Quota_guard
{
  Quota *q;
  size_t amount;

  Quota_guard(Quota *q, size_t amount) : q(q), amount(amount)
  {
    if (!q->alloc(amount))
      throw L4::Out_of_memory();
  }

  ~Quota_guard()
  {
    if (q)
      q->free(amount);
  }

  void done()
  {
    q = 0;
  }

  template< typename T >
  T done(T t)
  {
    q = 0;
    return t;
  }
};

namespace Q_object_tr {
  template< typename T, bool B >
  struct Qtr;
}

class Q_object : public Finalized
{
protected:
  Quota *quota() const { return _quota; }

private:
  template< typename T, bool B >
  friend class Q_object_tr::Qtr;
  Quota *_quota;
};

namespace Q_object_tr {
  template< typename T, bool B >
  struct Qtr
  {
    struct Obj : public T
    {
      Quota *_q;
    };
    static Quota *quota(T *o) { return static_cast<Obj*>(o)->_q; }
    static void quota(T *o, Quota *q) { static_cast<Obj*>(o)->_q = q; }
  };

  template< typename T >
  struct Qtr<T, true>
  {
    typedef T Obj;
    static Quota *quota(T *o) { return static_cast<Q_object*>(o)->_quota; }
    static void quota(T *o, Quota *q) { static_cast<Q_object*>(o)->_quota = q; }
  };
}

template< typename Type, template <typename T > class Alloc >
class Q_alloc
{
private:
  static int __is_qobject__(Q_object *) { return 0; }
  static char __is_qobject__(...) { return 0; }
  typedef Q_object_tr::Qtr<Type, sizeof(__is_qobject__((Type*)0)) == sizeof(int)> Qtr;
  typedef typename Qtr::Obj Obj;
  typedef Alloc<Obj> Allocator;

public:
  typedef Type Obj_type;

  Obj_type *alloc(Quota *quota, size_t extra = 0)
  {
    Quota_guard g(quota, sizeof(Obj) + extra);

    Obj *o = _alloc.alloc();
    if (o)
      {
	Qtr::quota(o, quota);
	g.done();
	return o;
      }

    throw L4::Out_of_memory();
  }

  void free(Obj_type *o) throw()
  {
    Obj *io = static_cast<Obj*>(o);
    Qtr::quota(o)->free(sizeof(Obj));
    _alloc.free(io);
  }

private:
  Allocator _alloc;
};


template< typename Alloc >
struct Q_alloc_static
{
  static void *_alloc(Quota *q, unsigned long size, unsigned long align)
  {
    Quota_guard g(q, size);
    return g.done(Alloc::_alloc(size, align));
  }

  static void _free(Quota *q, void *p, unsigned long size) throw()
  {
    Alloc::_free(p, size);
    q->free(size);
  }
};

}
