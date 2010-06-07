//-----------------------------------------------------------------------------
INTERFACE [arm && imx]:

enum {
  Cache_flush_area = 0xe0000000,
};

//-----------------------------------------------------------------------------
IMPLEMENTATION [arm && imx]:
void
map_hw(void *pd)
{
  // map devices
  map_1mb(pd, Mem_layout::Device_map_base, Mem_layout::Device_phys_base, false, false);
}
