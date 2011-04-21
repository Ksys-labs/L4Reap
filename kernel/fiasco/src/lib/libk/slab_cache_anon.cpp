INTERFACE:

#include <spin_lock.h>

// The anonymous slab allocator.  You can specialize this allocator by
// providing your own initialization functions and your own low-level
// allocation functions.

class slab_cache_anon;

class slab
{
private:
  slab(const slab&);	// copy constructors remain undefined

  struct slab_entry
  {
    slab_entry *_next_free;
  };

  slab_cache_anon *_cache;
  slab_entry *_first_free;
  slab *_next, *_prev;
  unsigned short _in_use;
};

class slab_cache_anon
{
protected:
  friend class slab;

  // Low-level allocator functions:

  // Allocate/free a block.  "size" is always a multiple of PAGE_SIZE.
  virtual void *block_alloc(unsigned long size, unsigned long alignment) = 0;
  virtual void block_free(void *block, unsigned long size) = 0;

private:
  slab_cache_anon();
  slab_cache_anon(const slab_cache_anon&); // default constructor is undefined

  //
  // data declaration follows
  // 

  // The slabs of this cache are held in a partially-sorted
  // doubly-linked list.  First come the fully-active slabs (all
  // elements in use), then the partially active slabs, then empty
  // slabs.
  slab *_first_slab, *_first_available_slab, *_last_slab;
  unsigned long _slab_size;
  unsigned _entry_size, _elem_num;
  typedef Spin_lock<> Lock;
  Lock lock;
  char const *_name;
};

IMPLEMENTATION:

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <lock_guard.h>

// default deallocator must not be called -- must use explicit destruction
inline NOEXPORT
void 
slab::operator delete(void* /*block*/)
{
  assert (!"slab::operator delete called");
}

PUBLIC
slab::slab(slab_cache_anon *cache, void *mem)
: _cache(cache), _next(0), _prev(0), _in_use(0)
{
  // Compute pointer to first data element, now taking into account
  // the latest colorization offset
  char *data = reinterpret_cast<char*>(mem);

  // Initialize the cache elements
  slab_entry *e = 0, *e_prev = 0;

  for (unsigned i = 0; i < cache->_elem_num; i++)
    {
      e = reinterpret_cast<slab_entry *>(data);

      e->_next_free = e_prev;
      data += cache->_entry_size;
      e_prev = e;
    }

  _first_free = e;
}

PUBLIC
void *
slab::alloc()
{
  slab_entry *e = _first_free;

  if (! e)
    return 0;

  _first_free = e->_next_free;
  ++_in_use;

  return e;
}

PUBLIC
void
slab::free(void *entry)
{
  slab_entry *e = reinterpret_cast<slab_entry *>(entry);
  e->_next_free = _first_free;
  _first_free = e;

  assert(_in_use);
  --_in_use;
}

PUBLIC
inline bool
slab::is_empty() const
{
  return _in_use == 0;
}

PUBLIC
inline bool
slab::is_full() const
{
  return _in_use == _cache->_elem_num;
}

PUBLIC
inline unsigned
slab::in_use() const
{
  return _in_use;
}

PUBLIC
void
slab::enqueue(slab *prev)
{
  assert(prev);

  if ((_next = prev->_next))
    _next->_prev = this;
  _prev = prev;
  _prev->_next = this;
}

PUBLIC
void
slab::dequeue()
{
  if (_prev)
    _prev->_next = _next;
  if (_next)
    _next->_prev = _prev;

  _prev = _next = 0;
}

PUBLIC
inline slab *
slab::prev() const
{
  return _prev;
}

PUBLIC
inline slab *
slab::next() const
{
  return _next;
}

PUBLIC
inline void *
slab::operator new(size_t, void *block) throw()
{
  // slabs must be size-aligned so that we can compute their addresses
  // from element addresses
  return block;
}


PUBLIC static inline
unsigned
slab_cache_anon::entry_size(unsigned elem_size, unsigned alignment)
{ return (elem_size + alignment - 1) & ~(alignment - 1); }

// 
// slab_cache_anon
// 
PUBLIC inline NEEDS[slab_cache_anon::entry_size]
slab_cache_anon::slab_cache_anon(unsigned elem_size, 
				 unsigned alignment,
				 char const * name, 
				 unsigned long min_size,
				 unsigned long max_size)
  : _first_slab(0), _first_available_slab(0), _last_slab(0),
    _entry_size(entry_size(elem_size, alignment)),
    _name (name)
{
  lock.init();

  for (
      _slab_size = min_size;
      (_slab_size - sizeof(slab)) / _entry_size < 8
        && _slab_size < max_size;
      _slab_size <<= 1) ;

  _elem_num = (_slab_size - sizeof(slab)) / _entry_size;
}

//
// slab_cache_anon
//
PUBLIC inline
slab_cache_anon::slab_cache_anon(unsigned long slab_size,
				 unsigned elem_size,
				 unsigned alignment,
				 char const * name)
  : _first_slab(0), _first_available_slab(0), _last_slab(0),
    _slab_size(slab_size), _entry_size(entry_size(elem_size, alignment)),
    _name (name)
{
  lock.init();
  _elem_num = (_slab_size - sizeof(slab)) / _entry_size;
}

PUBLIC
virtual
slab_cache_anon::~slab_cache_anon()
{
  // the derived class should call destroy() before deleting us.
  // assert(_first_slab == 0);
}

PROTECTED inline
void
slab_cache_anon::destroy()	// descendant should call this in destructor
{
}

PUBLIC
virtual void *
slab_cache_anon::alloc()	// request initialized member from cache
{
  void *unused_block = 0;
  void *ret;
    {
      Lock_guard<Lock> guard(&lock);

      if (EXPECT_FALSE(!_first_available_slab))
	{
	  guard.release();

	  char *m = (char*)block_alloc(_slab_size, _slab_size);
	  if (!m)
	    return 0;

	  slab *s = new (m + _slab_size - sizeof(slab)) slab(this, m);

	  guard.lock(&lock);

	  if (EXPECT_TRUE(!_first_available_slab))
	    {
	      _first_available_slab = s;

	      if (_last_slab)
		{
		  assert(_last_slab->is_full());

		  _first_available_slab->enqueue(_last_slab);
		  _last_slab = _first_available_slab;
		}
	      else			// this was the first slab we allocated
		_first_slab = _last_slab = _first_available_slab;
	    }
	  else
	    unused_block = m;
	}

      assert(_first_available_slab && ! _first_available_slab->is_full());
      assert(! _first_available_slab->prev() || _first_available_slab->prev()->is_full());

      ret = _first_available_slab->alloc();
      assert(ret);

      if (_first_available_slab->is_full())
	_first_available_slab = _first_available_slab->next();
    }

  if (unused_block)
    block_free(unused_block, _slab_size);

  return ret;
}

PUBLIC template< typename Q >
inline
void *
slab_cache_anon::q_alloc(Q *quota)
{
  if (EXPECT_FALSE(!quota->alloc(_entry_size)))
    return 0;

  void *r;
  if (EXPECT_FALSE(!(r=alloc())))
    {
      quota->free(_entry_size);
      return 0;
    }

  return r;
}

PUBLIC
virtual void
slab_cache_anon::free(void *cache_entry) // return initialized member to cache
{
  Lock_guard<Lock> guard(&lock);

  slab *s = reinterpret_cast<slab*>
    ((reinterpret_cast<unsigned long>(cache_entry) & ~(_slab_size - 1)) + _slab_size - sizeof(slab));

  bool was_full = s->is_full();

  s->free(cache_entry);

  if (was_full)
    {
      if (s->next() == 0)	// have no right neighbor?
	{
	  assert(! _first_available_slab);
	}
      else if (s->next()->is_full()) // right neigbor full?
	{
	  // We requeue to become the first non-full slab in the queue
	  // so that all full slabs are at the beginning of the queue.

	  if (s == _first_slab)
	    _first_slab = s->next();
	  // don't care about _first_available_slab, _last_slab --
	  // they cannot point to s because we were full and have a
	  // right neighbor

	  s->dequeue();

	  if (_first_available_slab)
	    {
	      // _first_available_slab->prev() is defined because we
	      // had a right neighbor which is full, that is,
	      // _first_available_slab wasn't our right neighbor and
	      // now isn't the first slab in the queue
	      assert(_first_available_slab->prev()->is_full());
	      s->enqueue(_first_available_slab->prev());
	    }
	  else
	    {
	      // all slabs were full
	      assert(_last_slab->is_full());
	      s->enqueue(_last_slab);
	      _last_slab = s;
	    }
	}

      _first_available_slab = s;

    }
  else if (s->is_empty())
    {
      if (s->next() && (! s->next()->is_empty())) // right neighbor not empty?
	{
	  // Move to tail of list

	  if (s == _first_slab)
	    _first_slab = s->next();
	  if (s == _first_available_slab)
	    _first_available_slab = s->next();
	  // don't care about _last_slab because we know we have a
	  // right neighbor

	  s->dequeue();

	  s->enqueue(_last_slab);
	  _last_slab = s;
	}
    }
  else
    {
      // We weren't either full or empty; we already had free
      // elements.  This changes nothing in the queue, and there
      // already must have been a _first_available_slab.
    }

  assert(_first_available_slab);
}

PUBLIC template< typename Q >
inline
void
slab_cache_anon::q_free(Q *quota, void *obj)
{
  free(obj);
  quota->free(_entry_size);
}

PUBLIC
virtual unsigned long
slab_cache_anon::reap()		// request that cache returns memory to system
{
  Lock_guard<Lock> guard(&lock);

  if (! _first_slab)
    return 0;			// haven't allocated anything yet

  // never delete first slab, even if it is empty
  if (_last_slab == _first_slab
      || (! _last_slab->is_empty()))
    return 0;

  slab *s = _last_slab;

  if (_first_available_slab == s)
    _first_available_slab = 0;

  _last_slab = s->prev();
  s->dequeue();

  // explicitly call destructor to delete s;
  s->~slab();
  block_free(s, _slab_size);

  return _slab_size;
}

// Debugging output

#include <cstdio>

PUBLIC
virtual void
slab_cache_anon::debug_dump()
{
  printf ("%s: %lu-KB slabs (elems per slab=%d ",
	  _name, _slab_size / 1024, _elem_num);

  unsigned count, total = 0, total_elems = 0;
  slab* s = _first_slab;

  for (count = 0;
       s && s->is_full();
       s = s->next())
    {
      count++;
      total_elems += s->in_use();
    }

  total += count;

  printf ("%u full, ", count);

  for (count = 0;
       s && ! s->is_empty();
       s = s->next())
    {
      if (s->is_full())
	printf ("\n*** wrongly-enqueued full slab found\n");
      count++;
      total_elems += s->in_use();
    }

  total += count;

  printf ("%u used, ", count);

  for (count = 0;
       s;
       s = s->next())
    {
      if (! s->is_empty())
	printf ("\n*** wrongly-enqueued nonempty slab found\n");
      count++;
      total_elems += s->in_use();
    }

  unsigned total_used = total;
  total += count;

  printf ("%u empty = %u total) = %lu KB,\n  %u elems (size=%u)",
	  count, total, total * _slab_size / 1024,
	  total_elems, _entry_size);

  if (total_elems)
    printf (", overhead = %lu B (%lu B)  = %lu%% (%lu%%) \n",
	    total * _slab_size - total_elems * _entry_size,
	    total_used * _slab_size - total_elems * _entry_size,
	    100 - total_elems * _entry_size * 100 / (total * _slab_size),
	    100 - total_elems * _entry_size * 100 / (total_used * _slab_size));
  else
    printf ("\n");
}
