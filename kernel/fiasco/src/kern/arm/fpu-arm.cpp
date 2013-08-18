INTERFACE [arm && fpu]:

#include <cxx/bitfield>

EXTENSION class Fpu
{
public:
  struct Exception_state_user
  {
    Mword fpexc;
    Mword fpinst;
    Mword fpinst2;
  };

  enum
  {
    FPEXC_EN  = 1 << 30,
    FPEXC_EX  = 1 << 31,
  };

  struct Fpu_regs
  {
    Mword fpexc, fpscr;
    Mword state[64];
  };

  struct Fpsid
  {
    Mword v;

    Fpsid() = default;
    explicit Fpsid(Mword v) : v(v) {}

    CXX_BITFIELD_MEMBER(0, 3, rev, v);
    CXX_BITFIELD_MEMBER(4, 7, variant, v);
    CXX_BITFIELD_MEMBER(8, 15, part_number, v);
    CXX_BITFIELD_MEMBER(16, 19, arch_version, v);
    CXX_BITFIELD_MEMBER(20, 20, precision, v);
    CXX_BITFIELD_MEMBER(21, 22, format, v);
    CXX_BITFIELD_MEMBER(23, 23, hw_sw, v);
    CXX_BITFIELD_MEMBER(24, 31, implementer, v);
  };

  Fpsid fpsid() const { return _fpsid; }

private:
  Fpsid _fpsid;
  static bool save_32r;
};

// ------------------------------------------------------------------------
INTERFACE [arm && !fpu]:

EXTENSION class Fpu
{
public:
  struct Exception_state_user
  {
  };
};

// ------------------------------------------------------------------------
IMPLEMENTATION [arm && !fpu]:

#include "trap_state.h"

PUBLIC static inline NEEDS["trap_state.h"]
void
Fpu::save_user_exception_state(Trap_state *, Exception_state_user *)
{}

// ------------------------------------------------------------------------
IMPLEMENTATION [arm && fpu && !armv6plus]:

PRIVATE static inline
void
Fpu::copro_enable()
{}

// ------------------------------------------------------------------------
IMPLEMENTATION [arm && fpu && armv6plus]:

PRIVATE static inline
void
Fpu::copro_enable()
{
  asm volatile("mrc  p15, 0, %0, c1, c0, 2\n"
               "orr  %0, %0, %1           \n"
               "mcr  p15, 0, %0, c1, c0, 2\n"
               : : "r" (0), "I" (0x00f00000));
  Mem::isb();
}

// ------------------------------------------------------------------------
IMPLEMENTATION [arm && fpu]:

#include <cassert>
#include <cstdio>
#include <cstring>

#include "fpu_state.h"
#include "mem.h"
#include "processor.h"
#include "static_assert.h"
#include "trap_state.h"

bool Fpu::save_32r;

PUBLIC static inline
Mword
Fpu::fpsid_read()
{
  Mword v;
  asm volatile("mrc p10, 7, %0, cr0, cr0" : "=r" (v));
  return v;
}

PUBLIC static inline
Mword
Fpu::mvfr0()
{
  Mword v;
  asm volatile("mrc p10, 7, %0, cr7, cr0" : "=r" (v));
  return v;
}

PUBLIC static inline
Mword
Fpu::mvfr1()
{
  Mword v;
  asm volatile("mrc p10, 7, %0, cr6, cr0" : "=r" (v));
  return v;
}


PRIVATE static inline
void
Fpu::fpexc(Mword v)
{
  asm volatile("mcr p10, 7, %0, cr8, cr0" : : "r" (v));
}

PUBLIC static inline
Mword
Fpu::fpexc()
{
  Mword v;
  asm volatile("mrc p10, 7, %0, cr8, cr0" : "=r" (v));
  return v;
}

PUBLIC static inline
Mword
Fpu::fpinst()
{
  Mword i;
  asm volatile("mcr p10, 7, %0, cr9,  cr0" : "=r" (i));
  return i;
}

PUBLIC static inline
Mword
Fpu::fpinst2()
{
  Mword i;
  asm volatile("mcr p10, 7, %0, cr10,  cr0" : "=r" (i));
  return i;
}

PUBLIC static inline
bool
Fpu::exc_pending()
{
  return fpexc() & FPEXC_EX;
}

IMPLEMENT
void
Fpu::enable()
{
  fpexc((fpexc() | FPEXC_EN) & ~FPEXC_EX);
}

IMPLEMENT
void
Fpu::disable()
{
  fpexc(fpexc() & ~FPEXC_EN);
}

PUBLIC static inline
int
Fpu::is_emu_insn(Mword opcode)
{
  return (opcode & 0x0ff00f90) == 0x0ef00a10;
}

PUBLIC static inline
bool
Fpu::emulate_insns(Mword opcode, Trap_state *ts)
{
  unsigned reg = (opcode >> 16) & 0xf;
  unsigned rt  = (opcode >> 12) & 0xf;
  Fpsid fpsid = Fpu::fpu.current().fpsid();
  switch (reg)
    {
    case 0: // FPSID
      ts->r[rt] = fpsid.v;
      break;
    case 6: // MVFR1
      if (fpsid.arch_version() < 2)
        return false;
      ts->r[rt] = Fpu::mvfr1();
      break;
    case 7: // MVFR0
      if (fpsid.arch_version() < 2)
        return false;
      ts->r[rt] = Fpu::mvfr0();
      break;
    default:
      break;
    }

  if (ts->psr & Proc::Status_thumb)
    ts->pc += 2;

  return true;
}

PRIVATE static
void
Fpu::show(Cpu_number cpu)
{
  const Fpsid s = fpu.cpu(cpu)._fpsid;
  unsigned arch = s.arch_version();
  printf("FPU%d: Arch: %s(%x), Part: %s(%x), r: %x, v: %x, i: %x, t: %s, p: %s\n",
         cxx::int_value<Cpu_number>(cpu),
         arch == 1 ? "VFPv2"
                   : (arch == 3 ? "VFPv3"
                                : (arch == 4 ? "VFPv4"
                                             : "Unkn")),
         arch,
         (int)s.part_number() == 0x20
           ? "VFP11"
           : (s.part_number() == 0x30 ?  "VFPv3" : "Unkn"),
         (int)s.part_number(),
         (int)s.rev(), (int)s.variant(), (int)s.implementer(),
         (int)s.hw_sw() ? "soft" : "hard",
         (int)s.precision() ? "sngl" : "dbl/sngl");
}


IMPLEMENT
void
Fpu::init(Cpu_number cpu, bool resume)
{
  copro_enable();

  fpu.cpu(cpu)._fpsid = Fpsid(fpsid_read());
  if (cpu == Cpu_number::boot_cpu())
    save_32r = (mvfr0() & 0xf) == 2;

  if (!resume)
    show(cpu);

  disable();

  fpu.cpu(cpu).set_owner(0);
}

IMPLEMENT inline NEEDS ["fpu_state.h", "mem.h", "static_assert.h"]
void
Fpu::init_state(Fpu_state *s)
{
  Fpu_regs *fpu_regs = reinterpret_cast<Fpu_regs *>(s->state_buffer());
  static_assert(!(sizeof (*fpu_regs) % sizeof(Mword)),
                "Non-mword size of Fpu_regs");
  Mem::memset_mwords(fpu_regs, 0, sizeof (*fpu_regs) / sizeof(Mword));
}

PRIVATE static inline
void
Fpu::save_fpu_regs(Fpu_regs *r)
{
  Mword tmp;
  asm volatile("stc p11, cr0, [%0], #128     \n"
               "cmp    %1, #0                \n"
               "stcnel p11, cr0, [%0], #128  \n"
               : "=r" (tmp) : "0" (r->state), "r" (save_32r));
}

PRIVATE static inline
void
Fpu::restore_fpu_regs(Fpu_regs *r)
{
  Mword tmp;
  asm volatile("ldc    p11, cr0, [%0], #128 \n"
               "cmp    %1, #0               \n"
               "ldcnel p11, cr0, [%0], #128 \n"
               : "=r" (tmp) : "0" (r->state), "r" (save_32r));
}

IMPLEMENT
void
Fpu::save_state(Fpu_state *s)
{
  Fpu_regs *fpu_regs = reinterpret_cast<Fpu_regs *>(s->state_buffer());

  assert(fpu_regs);

  save_fpu_regs(fpu_regs);

  asm volatile ("mrc p10, 7, %0, cr8,  cr0, 0  \n"
                "mrc p10, 7, %1, cr1,  cr0, 0  \n"
                : "=r" (fpu_regs->fpexc),
                  "=r" (fpu_regs->fpscr));
}

IMPLEMENT
void
Fpu::restore_state(Fpu_state *s)
{
  Fpu_regs *fpu_regs = reinterpret_cast<Fpu_regs *>(s->state_buffer());

  assert(fpu_regs);

  restore_fpu_regs(fpu_regs);

  asm volatile ("mcr p10, 7, %0, cr8,  cr0, 0  \n"
                "mcr p10, 7, %1, cr1,  cr0, 0  \n"
                :
                : "r" (fpu_regs->fpexc | FPEXC_EN),
                  "r" (fpu_regs->fpscr));
}

IMPLEMENT inline
unsigned
Fpu::state_size()
{ return sizeof (Fpu_regs); }

IMPLEMENT inline
unsigned
Fpu::state_align()
{ return 4; }

PUBLIC static
bool
Fpu::is_enabled()
{
  return fpexc() & FPEXC_EN;
}

PUBLIC static inline NEEDS["trap_state.h"]
void
Fpu::save_user_exception_state(Trap_state *ts, Exception_state_user *esu)
{
  if ((ts->error_code & 0x01f00000) == 0x01100000)
    {
      esu->fpexc = Fpu::fpexc();
      if (ts->error_code == 0x03100000)
	{
	  esu->fpinst  = Fpu::fpinst();
	  esu->fpinst2 = Fpu::fpinst2();
	}
    }
}
