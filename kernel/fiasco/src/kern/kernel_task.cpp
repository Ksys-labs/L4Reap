INTERFACE:

#include "task.h"

class Kernel_thread;

class Kernel_task : public Task
{
  friend class Kernel_thread;
};

IMPLEMENTATION[!(arm || ppc32)]:

#include "config.h"
#include "globals.h"
#include "kmem.h"

PRIVATE inline NEEDS["globals.h"]
Kernel_task::Kernel_task()
: Task(Space::Default_factory(), Ram_quota::root, Kmem::kdir)
{}


IMPLEMENTATION:

PUBLIC static Task*
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
