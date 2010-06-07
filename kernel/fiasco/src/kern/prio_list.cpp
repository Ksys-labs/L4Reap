INTERFACE:

#include <cassert>
#include "member_offs.h"
#include "spin_lock.h"
#include "types.h"

/**
 * Priority sorted list with insert complexity O(n) n = number of available 
 * priorities (256 in Fiasco).
 */


class Prio_list_elem;

/**
 * Priority sorted list.
 *
 * The list is organized in a way that the highest priority member can be
 * found with O(1). Ever dequeue operation is also O(1).
 *
 * There is a forward iteratable list with at most one element per priority.
 * Elements with the same priority are handled in a double-linked circular
 * list for each priority. This double-linked list implements FIFO policy for
 * finding the next element.
 */
class Prio_list
{
  MEMBER_OFFSET();
private:
  Prio_list_elem *_head;
};

class Iteratable_prio_list : public Prio_list
{
private:
  Prio_list_elem *_cursor;
};


class Locked_prio_list : public Prio_list, public Spin_lock
{
};

/**
 * Single element of a priority sorted list.
 */
class Prio_list_elem
{
  MEMBER_OFFSET();
  friend class Prio_list;
  friend class Jdb_semaphore;
  friend class Jdb_sender_list;
private:
  /**
   * Priority list pointers. optimized for fast dequeue and insert. However
   * not reverse iteratable.
   */
  Prio_list_elem *_p_next, **_p_prev_next;

  /**
   * Pointers for FIFO queue for equal priorities.
   */
  Prio_list_elem *_s_next, *_s_prev;

  /**
   * Priority, the higher the better.
   */
  unsigned short _prio;
};



IMPLEMENTATION:

/**
 * Create a stand alone (not queued) list element.
 */
PUBLIC inline
Prio_list_elem::Prio_list_elem() 
: _s_next(0) 
{}

/**
 * Setup pointers for enqueue.
 */
PRIVATE inline
void
Prio_list_elem::init(unsigned short p)
{ 
  _prio = p; 
  _s_next = _s_prev = this;
  _p_next = 0;
  _p_prev_next = 0;
}

/**
 * Dequeue from the priority list.
 */
PRIVATE inline
void
Prio_list_elem::p_dequeue()
{
  *_p_prev_next = _p_next;
  if (_p_next)
    _p_next->_p_prev_next = _p_prev_next;
}

/**
 * Dequeue from the equal-prio FIFO.
 */
PRIVATE inline
void
Prio_list_elem::s_dequeue()
{
  _s_next->_s_prev = _s_prev;
  _s_prev->_s_next = _s_next;
}

/**
 * Replace an element in the priority list with an element with the
 * same priority.
 */
PRIVATE inline
void
Prio_list_elem::replace_with(Prio_list_elem *e)
{
  e->_p_next = _p_next;
  e->_p_prev_next = _p_prev_next;
  if (_p_next)
    _p_next->_p_prev_next = &e->_p_next;

  *_p_prev_next = e;
}

/**
 * Insert into priority list.
 */
PRIVATE inline
void
Prio_list_elem::p_insert(Prio_list_elem **e)
{
  _p_next = *e;
  _p_prev_next = e;
  if (*e)
    (*e)->_p_prev_next = &_p_next;

  *e = this;
}

/**
 * Insert into equal-prio FIFO.
 */
PRIVATE inline
void
Prio_list_elem::s_insert(Prio_list_elem *succ)
{ 
  _s_prev = succ->_s_prev;
  _s_next = succ;
  if (_s_prev)
    _s_prev->_s_next = this;
  succ->_s_prev = this;
}

/**
 * Get the priority.
 */
PUBLIC inline
unsigned short
Prio_list_elem::prio() const
{ return _prio; }

PUBLIC inline
Prio_list_elem *
Prio_list_elem::next() const
{
  if (_s_next->is_head())
    return _p_next;
  return _s_next;
}


/**
 * Create an empty Prio_list.
 */
PUBLIC inline
Prio_list::Prio_list() : _head(0) 
{}


/**
 * Insert a new element into the priority list.
 * @param e the element to insert
 * @param prio the priority for the element
 */
PUBLIC inline NEEDS[Prio_list_elem::init, Prio_list_elem::s_insert,
                    Prio_list_elem::p_insert]
void
Prio_list::insert(Prio_list_elem *e, unsigned short prio)
{
  assert (e);
  e->init(prio);

  Prio_list_elem **pos = &_head;

  while (*pos && (*pos)->prio() > prio)
    pos = &(*pos)->_p_next;

  if (*pos && (*pos)->prio() == prio)
    e->s_insert(*pos);
  else
    e->p_insert(pos);
}

/**
 * Are there element with the same priority?
 * @return true if there exist other elements with the same priority.
 */
PUBLIC inline
bool
Prio_list_elem::has_sibling() const
{ return _s_next != this; }

/**
 * Is the element actually enqueued?
 * @return true if the element is actaully enqueued in a list.
 */
PUBLIC inline
bool
Prio_list_elem::in_list() const
{ return _s_next; }

/**
 * Is the element the first of its priority?
 * @return true if the element is the first element of it's priority.
 */
PUBLIC inline
bool
Prio_list_elem::is_head() const
{ return _p_next || _p_prev_next; }

/**
 * Get the highest priority element from the list.
 * @return a pointer to the higest priority element (enqueued).
 */
PUBLIC inline
Prio_list_elem *
Prio_list::head() const
{
  return _head;
}

/**
 * Dequeue a given element from the list.
 * @param e the element to dequeue
 */
PUBLIC inline NEEDS[Prio_list_elem::has_sibling, Prio_list_elem::is_head,
                    Prio_list_elem::s_dequeue, Prio_list_elem::replace_with,
		    Prio_list_elem::p_dequeue]
void
Prio_list::dequeue(Prio_list_elem *e, Prio_list_elem **next = 0)
{
  if (e->is_head())
    {
      if (e->has_sibling())
	{
	  e->s_dequeue();
	  e->replace_with(e->_s_next);
	  if (next) *next = e->_s_next;
	}
      else
	{
	  e->p_dequeue();
	  if (next) *next = e->_p_next;
	}
    }
  else
    {
      e->s_dequeue();
      if (next)
	{
	  if (e->_s_next->is_head())
	    *next = e->_p_next;
	  else
	    *next = e->_s_next;
	}
    }

  e->_s_next = 0;
}

PUBLIC inline
Iteratable_prio_list::Iteratable_prio_list() : _cursor(0) {}

/**
 * Dequeue a given element from the list.
 * @param e the element to dequeue
 */
PUBLIC inline NEEDS[Prio_list::dequeue]
void
Iteratable_prio_list::dequeue(Prio_list_elem *e)
{
  Prio_list_elem **c = 0;
  if (EXPECT_FALSE(_cursor != 0) && EXPECT_FALSE(_cursor == e))
    c = &_cursor;

  Prio_list::dequeue(e, c);
}

PUBLIC inline
void
Iteratable_prio_list::cursor(Prio_list_elem *e)
{ _cursor = e; }

PUBLIC inline
Prio_list_elem *
Iteratable_prio_list::cursor() const
{ return _cursor; }

