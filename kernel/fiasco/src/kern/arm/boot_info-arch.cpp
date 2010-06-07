/* ARM specific boot_info */

INTERFACE [arm]:

class Kip;

EXTENSION class Boot_info
{
public:
  static void set_kip(Kip *kip);
  static Kip *kip();
};

//---------------------------------------------------------------------------
IMPLEMENTATION [arm]:

#include <cstring>
#include <cstdio> // for debug printf's

static Kip *boot_info_kip;

IMPLEMENT
void Boot_info::set_kip(Kip *kip)
{
  boot_info_kip = kip;
} 

IMPLEMENT
Kip *Boot_info::kip()
{
  return boot_info_kip;
} 

extern "C" char _etext, _sstack, _stack, _edata, _end;

IMPLEMENT static 
void Boot_info::init()
{
    
  // Dependent on how we've been booted, the BSS might not have been
  // cleaned out.  Do this now.  This is safe because we made sure in
  // crt0.S that our stack is not in the BSS.
  //  memset(&_edata, 0, &_sstack - &_edata);
  //  memset(&_stack, 0, &_end - &_stack);

  // We save the checksum for read-only data to be able to compare it against
  // the kernel image later (in jdb::enter_kdebug())
  //saved_checksum_ro = boot_info::get_checksum_ro();

#if 0
  kmbi = *(multiboot_info *)(phys_to_virt(get_mbi_pa()));
  
   Cmdline::init (kmbi.flags & MULTIBOOT_CMDLINE ?
                 static_cast<char*>(phys_to_virt (kmbi.cmdline)) : "");
#endif
}

PUBLIC static
void
Boot_info::reset_checksum_ro(void)
{}
