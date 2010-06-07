//-------------------------------------------------------------------------
IMPLEMENTATION [ppc32]:

IMPLEMENT inline
Utcb *
Utcb_support::current()
{
  Utcb *u;
  asm volatile ("mr %0, %%r2" : "=r" (u));
  return u;
}

IMPLEMENT inline
void
Utcb_support::current(Utcb *utcb)
{
  asm volatile ("mr %%r2, %0" : : "r" (utcb) : "memory");
}
