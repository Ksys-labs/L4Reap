IMPLEMENTATION [vmx && svm]:

#include "ram_quota.h"
#include "svm.h"
#include "vm_svm.h"
#include "vmx.h"
#include "vm_vmx.h"

PRIVATE static inline
template< typename VM >
VM *
Vm_factory::allocate(Ram_quota *quota)
{
  if (void *t = Vm::allocator<VM>()->q_alloc(quota))
    {
      VM *a = new (t) VM(quota);
      if (a->valid())
        return a;

      delete a;
    }

  return 0;
}

IMPLEMENT
Vm *
Vm_factory::create(Ram_quota *quota)
{
  if (Svm::cpus.cpu(current_cpu()).svm_enabled())
    return allocate<Vm_svm>(quota);
  if (Vmx::cpus.cpu(current_cpu()).vmx_enabled())
    return allocate<Vm_vmx>(quota);

  return 0;
}

IMPLEMENTATION [!(vmx && svm)]:

IMPLEMENT
Vm *
Vm_factory::create(Ram_quota *)
{ return 0; }
