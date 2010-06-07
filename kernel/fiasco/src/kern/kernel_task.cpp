INTERFACE:

#include "space.h"

class Kernel_thread;

class Kernel_task : public Space
{
  friend class Kernel_thread;
};

IMPLEMENTATION[!(arm || ppc32)]:

#include "config.h"
#include "globals.h"
#include "kmem.h"

PRIVATE inline NEEDS["globals.h"]
Kernel_task::Kernel_task()
: Space(Space::Default_factory(), Ram_quota::root, Kmem::kdir)
{}


IMPLEMENTATION:

PUBLIC static Space*
Kernel_task::kernel_task()
{
  static Kernel_task task;
  return &task;
}

PUBLIC static inline
void
Kernel_task::init()
{
  // Make sure the kernel task is initialized.
  kernel_task();
}
