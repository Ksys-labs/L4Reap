/*
 * (c) 2010 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
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

template< unsigned O_SIZE, unsigned N = 10 >
struct Simple_store_sz
{
  enum
  {
    Item_size = (O_SIZE + sizeof(unsigned long) - 1) / sizeof(unsigned long)
  };

  struct Item
  {
    unsigned long _str[Item_size];
    Item *next;
  };

  enum { MAX = N };

  Item *first;
  Item _o[MAX];

  Simple_store_sz() throw();

  void *alloc() throw();
  void free(void *b) throw();

};

template< unsigned O, unsigned N >
Simple_store_sz<O,N>::Simple_store_sz() throw() : first(_o)
{
  for (unsigned i = 0; i < MAX - 1; ++i)
    _o[i].next = _o + i + 1;

  _o[MAX - 1].next = 0;
}

template< unsigned O, unsigned N >
inline
void *
Simple_store_sz<O,N>::alloc() throw()
{
  Item *i = first;

  if (i)
    {
      first = i->next;
      return (void*)(i->_str);
    }

  return 0;
}

template< unsigned O, unsigned N >
inline
void
Simple_store_sz<O,N>::free(void *b) throw()
{
  Item *i = reinterpret_cast<Item*>(b);
  i->next = first;
  first = i;
}


template< typename O, unsigned N = 10 >
struct Simple_store : public Simple_store_sz<sizeof(O), N> {};

