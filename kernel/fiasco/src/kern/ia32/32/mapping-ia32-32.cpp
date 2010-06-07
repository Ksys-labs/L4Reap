INTERFACE [ia32,ux]:

#include "types.h"

class Treemap;

class Mapping_entry
{
public:
  enum { Alignment = 1 };

  union 
    {
      struct 
	{
	  unsigned long _space:32;	///< Address-space number
//	  unsigned long _pad1:1;        // Pad to 16-bit boundary to avoid
	  unsigned long address:20;	///< Virtual address in address space
	  unsigned long tag:11;		///< Unmap tag
	  unsigned long _pad2:5;	// Pad to 16-bit boundary to avoid
	                                // compiler bugs
        } __attribute__((packed)) data;
      Treemap *_submap;
    } __attribute__((packed));
  Unsigned8 _depth;

  void set_space(Space *s) { data._space = (unsigned long)s; }
  Space *space() const { return (Space *)data._space; }
} __attribute__((packed));


