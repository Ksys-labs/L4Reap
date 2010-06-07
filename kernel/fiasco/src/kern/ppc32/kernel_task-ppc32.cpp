IMPLEMENTATION[ppc32]:

#include "config.h"
#include "globals.h"
#include "kmem.h"

PRIVATE inline NEEDS["globals.h"]
Kernel_task::Kernel_task()
: Space(Space::Default_factory(), Ram_quota::root, Kmem::kdir())
{
}

