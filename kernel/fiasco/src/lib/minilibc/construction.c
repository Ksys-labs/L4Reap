
#include "initfini.h"
#include "types.h"

typedef void (*ctor_t)(void);

extern ctor_t __CTOR_END__[];
extern ctor_t __DTOR_END__[];
extern ctor_t __CTOR_LIST__[];
extern ctor_t __DTOR_LIST__[];


static int construction_done = 0;

void static_construction()
{
  ctor_t *cons = __CTOR_LIST__;
  while(cons != __CTOR_END__)
    if(*(--cons))
      (*cons)();

  construction_done = 1;
}


void static_destruction()
{
  ctor_t *cons = __DTOR_LIST__;
  if(!construction_done)
    return;

  while(cons != __DTOR_END__)
    if(*(--cons))
      (*cons)();
}
