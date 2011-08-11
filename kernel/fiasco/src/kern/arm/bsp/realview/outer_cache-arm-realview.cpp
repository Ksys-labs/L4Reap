// ------------------------------------------------------------------------
IMPLEMENTATION [arm && realview && outer_cache_l2cxx0 && armca9]:

IMPLEMENT
void
Outer_cache::platform_init()
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
IMPLEMENTATION [arm && realview && outer_cache_l2cxx0 && mpcore]:

IMPLEMENT
void
Outer_cache::platform_init()
{
  Mword aux_control = Io::read<Mword>(AUX_CONTROL);
  aux_control &= 0xfe000fff; // keep latencies, keep reserved, keep NS bits
  aux_control |= 8 << 13; // 8-way associative
  aux_control |= 4 << 17; // 128kb Way size
  aux_control |= 1 << 22; // shared bit ignore
  Io::write<Mword>(aux_control, AUX_CONTROL);
}
