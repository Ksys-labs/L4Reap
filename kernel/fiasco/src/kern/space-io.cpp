INTERFACE [io]:

#include "config.h"
#include "io_space.h"
#include "l4_types.h"

class Space;

EXTENSION class Space
{
private:
  typedef Space_container<Io_space> Io_space_container;
  Io_space_container _io_space;

public:
  /** The interface for Io_space to get the surrounding Space. */
  static Space *space(Io_space const *os)
  {
    return reinterpret_cast<Space*>(Address(os) - Address(&reinterpret_cast<Space*>(32)->_io_space) + 32);
  }

};


//----------------------------------------------------------------------------
IMPLEMENTATION [io]:

// 
// Utilities for map<Io_space> and unmap<Io_space>
// 

PUBLIC inline
Io_space*
Space::io_space()
{
  return _io_space.get();
}

PUBLIC inline
bool
Space::lookup_space (Io_space** out_io_space)
{
  *out_io_space = io_space();
  return true;
}

//----------------------------------------------------------------------------
IMPLEMENTATION [io && iopl3]:

/// Is this task a privileged one?
PUBLIC inline NEEDS ["l4_types.h", "config.h"]
bool
Space::has_io_privileges()
{
  // A task is privileged if it has all the IO ports mapped.
  return (!Config::enable_io_protection
	  || (io_space()->get_io_counter() == Mem_layout::Io_port_max));
}

//----------------------------------------------------------------------------
IMPLEMENTATION [io && !iopl3]:

/// Is this task a privileged one?
PUBLIC inline NEEDS ["l4_types.h", "config.h"]
bool 
Space::has_io_privileges()
{ return false; }
