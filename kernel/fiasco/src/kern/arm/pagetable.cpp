INTERFACE:

#include "paging.h"
#include "mapped_alloc.h"

class Pte;

class Page_table //: public Page_table_defs
{
public:

  enum Status {
    E_OK = 0,
    E_NOMEM,
    E_EXISTS,
    E_UPGRADE,
    E_INVALID,
  };


  void * operator new( size_t );
  void operator delete( void * );

  static void init(Page_table *current);

  Page_table();
  
  void copy_in(void *my_base, Page_table *o, 
	       void *base, size_t size = 0, unsigned long asid = ~0UL);

  void *dir() const;

  static Page_table *current();

  static size_t num_page_sizes();
  static size_t const * page_sizes();
  static size_t const * page_shifts();

  static void set_allocator( Mapped_allocator * );
  static Mapped_allocator *alloc();

private:
  static Mapped_allocator *allocator;

};

IMPLEMENTATION:

Mapped_allocator *Page_table::allocator = 0;


IMPLEMENT 
void Page_table::set_allocator( Mapped_allocator *a )
{
  allocator = a;
}


IMPLEMENT 
Mapped_allocator *Page_table::alloc()
{
  return allocator;
}

