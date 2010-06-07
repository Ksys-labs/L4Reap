/* __cxa_finalize and _Jv_RegisterClasses come from crtBeginS which defines
 * those weak, they end up as relocation entries but the number of relocs
 * does not include them, so, defining them avoids this
 */

void __cxa_finalize(void);
void _Jv_RegisterClasses(void);
int __aeabi_unwind_cpp_pr0(void);
int __aeabi_unwind_cpp_pr1(void);
enum { _URC_FAILURE  = 9 };

void __cxa_finalize(void) {}
void _Jv_RegisterClasses(void) {}

#ifdef ARCH_arm
int __aeabi_unwind_cpp_pr0(void) { return _URC_FAILURE; }
int __aeabi_unwind_cpp_pr1(void) { return _URC_FAILURE; }
#endif
