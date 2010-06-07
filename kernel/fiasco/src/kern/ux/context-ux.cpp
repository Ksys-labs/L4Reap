INTERFACE[ux]:

EXTENSION class Context
{
protected:
  bool _is_native; // thread can call Linux host system calls
};

INTERFACE[ux && segments]:

EXTENSION class Context
{
protected:
  enum { Gdt_user_entries = 3 };
  struct { Unsigned64 v[2]; } _gdt_user_entries[Gdt_user_entries]; // type struct Ldt_user_desc
  Unsigned32                  _es, _fs, _gs;
};

// ---------------------------------------------------------------------
IMPLEMENTATION[ux]:

IMPLEMENT inline
void
Context::switch_fpu (Context *)
{}

PUBLIC inline
bool
Context::is_native()
{ return _is_native; }

// ---------------------------------------------------------------------
IMPLEMENTATION[ux && segments]:

PROTECTED inline
void
Context::switch_gdt_user_entries(Context *to)
{
  Mword *trampoline_page = (Mword *) Kmem::phys_to_virt(Mem_layout::Trampoline_frame);
  Space *tos = to->vcpu_aware_space();

  if (EXPECT_FALSE(!tos))
    return;

  for (int i = 0; i < 3; i++)
    if (to == this
	|| _gdt_user_entries[i].v[0] != to->_gdt_user_entries[i].v[0]
        || _gdt_user_entries[i].v[1] != to->_gdt_user_entries[i].v[1])
      {
        memcpy(trampoline_page + 1, &to->_gdt_user_entries[i],
               sizeof(_gdt_user_entries[0]));
        Trampoline::syscall(tos->pid(), 243,
                            Mem_layout::Trampoline_page + sizeof(Mword));
      }
}
