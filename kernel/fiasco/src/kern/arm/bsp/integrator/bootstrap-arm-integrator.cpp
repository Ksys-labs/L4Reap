//-----------------------------------------------------------------------------
INTERFACE [arm && integrator]:

enum {
  Cache_flush_area = 0xe0000000,
};

//-----------------------------------------------------------------------------
IMPLEMENTATION [arm && integrator]:
void
map_hw(void *pd)
{
  map_dev<Mem_layout::Devices0_phys_base>(pd, 0);
  map_dev<Mem_layout::Devices1_phys_base>(pd, 1);
  map_dev<Mem_layout::Devices2_phys_base>(pd, 2);
  map_dev<Mem_layout::Devices3_phys_base>(pd, 3);
}