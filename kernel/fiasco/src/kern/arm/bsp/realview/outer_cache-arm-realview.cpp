INTERFACE [arm && realview && outer_cache]:

#include "mem_layout.h"
#include "spin_lock.h"

EXTENSION class Outer_cache
{
private:
  enum
  {
    Cache_line_shift = 5,


    CACHE_ID                       = Mem_layout::L220_map_base + 0x000,
    CACHE_TYPE                     = Mem_layout::L220_map_base + 0x004,
    CONTROL                        = Mem_layout::L220_map_base + 0x100,
    AUX_CONTROL                    = Mem_layout::L220_map_base + 0x104,
    TAG_RAM_CONTROL                = Mem_layout::L220_map_base + 0x108,
    DATA_RAM_CONTROL               = Mem_layout::L220_map_base + 0x10c,
    EVENT_COUNTER_CONTROL          = Mem_layout::L220_map_base + 0x200,
    EVENT_COUTNER1_CONFIG          = Mem_layout::L220_map_base + 0x204,
    EVENT_COUNTER0_CONFIG          = Mem_layout::L220_map_base + 0x208,
    EVENT_COUNTER1_VALUE           = Mem_layout::L220_map_base + 0x20c,
    EVENT_COUNTER0_VALUE           = Mem_layout::L220_map_base + 0x210,
    INTERRUPT_MASK                 = Mem_layout::L220_map_base + 0x214,
    MASKED_INTERRUPT_STATUS        = Mem_layout::L220_map_base + 0x218,
    RAW_INTERRUPT_STATUS           = Mem_layout::L220_map_base + 0x21c,
    INTERRUPT_CLEAR                = Mem_layout::L220_map_base + 0x220,
    CACHE_SYNC                     = Mem_layout::L220_map_base + 0x730,
    INVALIDATE_LINE_BY_PA          = Mem_layout::L220_map_base + 0x770,
    INVALIDATE_BY_WAY              = Mem_layout::L220_map_base + 0x77c,
    CLEAN_LINE_BY_PA               = Mem_layout::L220_map_base + 0x7b0,
    CLEAN_LINE_BY_INDEXWAY         = Mem_layout::L220_map_base + 0x7bb,
    CLEAN_BY_WAY                   = Mem_layout::L220_map_base + 0x7bc,
    CLEAN_AND_INV_LINE_BY_PA       = Mem_layout::L220_map_base + 0x7f0,
    CLEAN_AND_INV_LINE_BY_INDEXWAY = Mem_layout::L220_map_base + 0x7f8,
    CLEAN_AND_INV_BY_WAY           = Mem_layout::L220_map_base + 0x7fc,
    LOCKDOWN_BY_WAY_D_SIDE         = Mem_layout::L220_map_base + 0x900,
    LOCKDOWN_BY_WAY_I_SIDE         = Mem_layout::L220_map_base + 0x904,
    TEST_OPERATION                 = Mem_layout::L220_map_base + 0xf00,
    LINE_TAG                       = Mem_layout::L220_map_base + 0xf30,
    DEBUG_CONTROL_REGISTER         = Mem_layout::L220_map_base + 0xf40,

  };

  static Spin_lock<> _lock;

public:
  enum
  {
    Cache_line_size = 1 << Cache_line_shift,
    Cache_line_mask = Cache_line_size - 1,
  };
};

// ------------------------------------------------------------------------
IMPLEMENTATION [arm && realview && outer_cache]:

#include "io.h"
#include "static_init.h"
#include "lock_guard.h"

#include <cstdio>

Spin_lock<> Outer_cache::_lock;

IMPLEMENT inline NEEDS ["io.h"]
void
Outer_cache::sync()
{
  while (Io::read<Mword>(CACHE_SYNC))
    ;
}

PRIVATE static inline NEEDS ["io.h", "lock_guard.h"]
void
Outer_cache::write(Address reg, Mword val)
{
  Lock_guard<Spin_lock<> > guard(&_lock);
  Io::write<Mword>(val, reg);
  while (Io::read<Mword>(reg) & 1)
    ;
}

IMPLEMENT inline NEEDS[Outer_cache::write]
void
Outer_cache::clean()
{
  write(CLEAN_BY_WAY, 0xff);
  sync();
}

IMPLEMENT inline NEEDS[Outer_cache::write]
void
Outer_cache::clean(Mword phys_addr, bool do_sync = true)
{
  write(CLEAN_LINE_BY_PA, phys_addr & (~0UL << Cache_line_shift));
  if (do_sync)
    sync();
}

IMPLEMENT inline NEEDS[Outer_cache::write]
void
Outer_cache::flush()
{
  write(CLEAN_BY_WAY, 0xff);
  sync();
}

IMPLEMENT inline NEEDS[Outer_cache::write]
void
Outer_cache::flush(Mword phys_addr, bool do_sync = true)
{
  write(CLEAN_AND_INV_LINE_BY_PA, phys_addr & (~0UL << Cache_line_shift));
  if (do_sync)
    sync();
}

IMPLEMENT inline NEEDS[Outer_cache::write]
void
Outer_cache::invalidate()
{
  write(INVALIDATE_BY_WAY, 0xff);
  sync();
}

IMPLEMENT inline NEEDS[Outer_cache::write]
void
Outer_cache::invalidate(Address phys_addr, bool do_sync = true)
{
  write(INVALIDATE_LINE_BY_PA, phys_addr & (~0UL << Cache_line_shift));
  if (do_sync)
    sync();
}

// ------------------------------------------------------------------------
IMPLEMENTATION [arm && realview && outer_cache && armca9]:

PRIVATE static inline
void
Outer_cache::init_plat()
{
  Io::write<Mword>(0 , TAG_RAM_CONTROL);
  Io::write<Mword>(0 , DATA_RAM_CONTROL);
  Mword aux_control = Io::read<Mword>(AUX_CONTROL);
  aux_control &= 0xc0000fff;
  aux_control |= 1 << 17; // 16kb way size
  aux_control |= 1 << 20; // event monitor bus enable
  aux_control |= 1 << 22; // shared attribute ovr enable
  aux_control |= 1 << 28; // data prefetch
  aux_control |= 1 << 29; // insn prefetch
  Io::write<Mword>(aux_control, AUX_CONTROL);
}

// ------------------------------------------------------------------------
IMPLEMENTATION [arm && realview && outer_cache && mpcore]:

PRIVATE static inline
void
Outer_cache::init_plat()
{
  Mword aux_control = Io::read<Mword>(AUX_CONTROL);
  aux_control &= 0xfe000fff; // keep latencies, keep reserved, keep NS bits
  aux_control |= 8 << 13; // 8-way associative
  aux_control |= 4 << 17; // 128kb Way size
  aux_control |= 1 << 22; // shared bit ignore
  Io::write<Mword>(aux_control, AUX_CONTROL);
}

// ------------------------------------------------------------------------
IMPLEMENTATION [arm && realview && outer_cache]:

PUBLIC static
void
Outer_cache::init()
{
  printf("L2: ID=%08lx Type=%08lx\n",
         Io::read<Mword>(CACHE_ID),
         Io::read<Mword>(CACHE_TYPE));

  // disable
  Io::write(0, CONTROL);

  init_plat();

  Io::write<Mword>(0, INTERRUPT_MASK);

  invalidate();
  Io::write<Mword>(~0UL, INTERRUPT_CLEAR);

  // enable
  Io::write(1, CONTROL);

  printf("L2 cache enabled\n");
}

STATIC_INITIALIZE_P(Outer_cache, STARTUP_INIT_PRIO);
