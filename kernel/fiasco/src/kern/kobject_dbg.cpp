INTERFACE:

class Kobject_dbg
{
public:
  Kobject_dbg *dbg_info() { return this; }
};

//----------------------------------------------------------------------------
INTERFACE[debug]:

#include "spin_lock.h"
#include "lock_guard.h"

class Kobject;

EXTENSION class Kobject_dbg
{
  friend class Jdb_kobject;
  friend class Jdb_kobject_list;
  friend class Jdb_mapdb;

public:
  class Dbg_extension
  {
  public:
    virtual ~Dbg_extension() {}
    void operator delete (void *) {}
    Dbg_extension *next() const { return _next; }
    void add(Dbg_extension **head)
    {
      _next = *head;
      *head = this;
    }


  private:
    Dbg_extension *_next;
  };

public:
  Dbg_extension *_jdb_data;

private:
  Mword _dbg_id;

public:
  Mword dbg_id() const { return _dbg_id; }
  virtual Address kobject_start_addr() const = 0;
  virtual Mword kobject_size() const = 0;

private:
  Kobject_dbg *_pref, *_next;
  static Spin_lock_coloc<Kobject_dbg *> _jdb_head;
  static Kobject_dbg *_jdb_tail;
  static unsigned long _next_dbg_id;
};


//----------------------------------------------------------------------------
IMPLEMENTATION[debug]:

Spin_lock_coloc<Kobject_dbg *> Kobject_dbg::_jdb_head;
Kobject_dbg *Kobject_dbg::_jdb_tail;
unsigned long Kobject_dbg::_next_dbg_id;

PUBLIC static inline
Kobject_dbg *
Kobject_dbg::pointer_to_obj(void const *p)
{
  Kobject_dbg *l = _jdb_head.get_unused();
  while (l)
    {
      Mword a = l->kobject_start_addr();
      if (a <= Mword(p) && Mword(p) < (a + l->kobject_size()))
        return l;
      l = l->_next;
    }
  return 0;
}

PUBLIC static inline
unsigned long
Kobject_dbg::pointer_to_id(void const *p)
{
  Kobject_dbg *o = pointer_to_obj(p);
  if (o)
    return o->dbg_id();
  return ~0UL;
}

PUBLIC static
bool
Kobject_dbg::is_kobj(void const *o)
{
  return pointer_to_obj(o);
}

PUBLIC static
Kobject_dbg *
Kobject_dbg::id_to_obj(unsigned long id)
{
  Kobject_dbg *l = _jdb_head.get_unused();
  while (l)
    {
      if (l->dbg_id() == id)
	return l;
      l = l->_next;
    }
  return 0;
}

PUBLIC static
unsigned long
Kobject_dbg::obj_to_id(void const *o)
{
  return pointer_to_id(o);
}


PROTECTED inline
void
Kobject_dbg::enqueue_debug_queue()
{
  Lock_guard<Spin_lock> guard(&_jdb_head);

  _pref = _jdb_tail;
  if (_pref)
    _pref->_next = this;

  _jdb_tail = this;
  if (!_jdb_head.get_unused())
    _jdb_head.set_unused(this);
}

PRIVATE inline
void
Kobject_dbg::init_debug_info()
{
  _next = _pref = 0;
  _jdb_data = 0;

  Lock_guard<Spin_lock> guard(&_jdb_head);

  _dbg_id = _next_dbg_id++;
  _pref = _jdb_tail;
  if (_pref)
    _pref->_next = this;

  _jdb_tail = this;
  if (!_jdb_head.get_unused())
    _jdb_head.set_unused(this);
}

PROTECTED inline
void
Kobject_dbg::dequeue_debug_queue()
{
    {
      Lock_guard<Spin_lock> guard(&_jdb_head);

      if (_pref)
	_pref->_next = _next;

      if (_next)
	_next->_pref = _pref;

      if (_jdb_head.get_unused() == this)
	_jdb_head.set_unused(_next);

      if (_jdb_tail == this)
	_jdb_tail = _pref;
    }
  _pref = 0;
  _next = 0;
}

PRIVATE inline
void
Kobject_dbg::fini_debug_info()
{
  dequeue_debug_queue();

  if (_jdb_data)
    delete _jdb_data;

  _jdb_data = 0;
}

PROTECTED
Kobject_dbg::Kobject_dbg()
{
  init_debug_info();
}

PROTECTED virtual
Kobject_dbg::~Kobject_dbg()
{
  fini_debug_info();
}


//---------------------------------------------------------------------------
IMPLEMENTATION [!debug]:

PUBLIC inline
unsigned long
Kobject_dbg::dbg_id() const
{ return 0; }

PUBLIC static inline
unsigned long
Kobject_dbg::dbg_id(void const *)
{ return ~0UL; }

PUBLIC static inline
Kobject_dbg *
Kobject_dbg::pointer_to_obj(void const *)
{ return 0; }

PUBLIC static inline
unsigned long
Kobject_dbg::pointer_to_id(void const *)
{ return ~0UL; }

PUBLIC static
bool
Kobject_dbg::is_kobj(void const *)
{ return false; }

PUBLIC static
Kobject_dbg *
Kobject_dbg::id_to_obj(unsigned long)
{ return 0; }

PUBLIC static
unsigned long
Kobject_dbg::obj_to_id(void const *)
{ return ~0UL; }
