/*
 * (c) 2010 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include <l4/sys/l4int.h>
#include <l4/vbus/vbus_types.h>
#include <l4/cxx/minmax>
#include <l4/cxx/string>

#include "resource.h"

#include <cstdio> 

template< typename D >
class Device_tree
{
private:
  D *_n;
  D *_p;
  D *_c;
  int _depth;

public:
  Device_tree() : _n(0), _p(0), _c(0), _depth(0) {}

  D *parent() const { return _p; }
  D *children() const { return _c; }
  D *next() const { return _n; }
  int depth() const { return _depth; }

  void set_parent(D *p) { _p = p; }

  void add_sibling(D *s)
  { _n = s; }

  void add_child(D *d, D *self)
  {
    for (iterator i = iterator(0, d, L4VBUS_MAX_DEPTH); i != iterator(); ++i)
      i->set_depth(i->depth() + depth() + 1);

    d->set_parent(self);

    if (!_c)
      _c = d;
    else
      {
	D *p;
	for (p = _c; p->next(); p = p->next())
	  ;
	p->add_sibling(d);
      }
  }

  void set_depth(int d) { _depth = d; }

  class iterator
  {
  public:
    iterator(D *p, D *c, int depth = 0)
    : _p(p), _c(c), _d(depth + (p ? p->depth() : 0))
    {}

    iterator(D const *p, int depth = 0)
    : _p(p), _c(p->children()), _d(depth + p->depth())
    {}

    iterator()
    : _c(0)
    {}

    bool operator == (iterator const &i) const
    {
      if (!_c && !i._c)
	return true;

      return _p == i._p && _c == i._c && _d == i._d;
    }

    bool operator != (iterator const &i) const
    { return !operator == (i); }

    D *operator -> () const { return _c; }
    D *operator * () const { return _c; }

    iterator operator ++ ()
    {
      if (_d > _c->depth() && _c->children())
	// go to a child if not at max depth and there are children
	_c = _c->children();
      else if (_c->next())
	// go to the next sibling
	_c = _c->next();
      else
	{
	  for (D *x = _c->parent(); x && x != _p; x = x->parent())
	    if (x->next())
	      {
		_c = x->next();
		return *this;
	      }
	  _c = 0;
	}

      return *this;
    }

    iterator operator ++ (int)
    {
      iterator o = *this;
      ++(*this);
      return o;
    }

  private:
    D const *_p;
    D *_c;
    int _d;
  };
};

template< typename D >
class Device_tree_mixin
{
protected:
  Device_tree<D> _dt;

public:
  typedef typename Device_tree<D>::iterator iterator;

  iterator begin(int depth = 0) const
  { return iterator(static_cast<D const*>(this), depth); }

  static iterator end() { return iterator(); }

  void set_depth(int d) { return _dt.set_depth(d); }
  void set_parent(D *p) { _dt.set_parent(p); }
  void add_sibling(D *s) { _dt.add_sibling(s); }
  virtual void add_child(D *c) { _dt.add_child(c, static_cast<D*>(this)); }
  virtual ~Device_tree_mixin() {}
};

class Resource_container
{
public:
  virtual Resource_list const *resources() const = 0;
  virtual bool resource_allocated(Resource const *) const  = 0;
  virtual ~Resource_container() {}
};


class Device : public Resource_container
{
public:
  virtual Device *parent() const = 0;
  virtual Device *children() const = 0;
  virtual Device *next() const = 0;
  virtual int depth() const = 0;

  virtual bool request_child_resource(Resource *, Device *) = 0;
  virtual bool alloc_child_resource(Resource *, Device *)  = 0;

  void request_resource(Resource *r);
  void request_resources();
  void request_child_resources();
  void allocate_pending_child_resources();
  void allocate_pending_resources();
  virtual void setup_resources() = 0;

  virtual char const *name() const = 0;
  virtual char const *hid() const = 0;
  virtual bool name(cxx::String const &)  = 0;

  virtual void dump(int) const {};

  virtual ~Device() {}

  typedef Device_tree<Device>::iterator iterator;

  iterator begin(int depth = 0) const { return iterator(this, depth); }
  static iterator end() { return iterator(); }

};



class Generic_device : public Device
{
private:
  Resource_list _resources;

public:
  //typedef gen_iterator<Generic_device> iterator;

  Resource_list const *resources() const { return &_resources; }
  void add_resource(Resource *r)
  { _resources.push_back(r); }

  virtual bool match_cid(cxx::String const &) const { return false; }
  virtual char const *name() const { return "(noname)"; }
  virtual char const *hid() const { return 0; }
  virtual bool name(cxx::String const &) { return false; }


  virtual bool request_child_resource(Resource *, Device *);
  virtual bool alloc_child_resource(Resource *, Device *);
  virtual void setup_resources();

  virtual ~Generic_device() {}

};


