
#include <l4/arm_drivers/io.h>
#include <l4/arm_drivers/amba.h>


void amba_read_id(l4_addr_t address, uint32_t *periphid, uint32_t *cellid)
{
  *periphid =   ((io_read_mword((l4_addr_t)address +  0) & 0xff) << 0)
              | ((io_read_mword((l4_addr_t)address +  4) & 0xff) << 8)
              | ((io_read_mword((l4_addr_t)address +  8) & 0xff) << 16)
              | ((io_read_mword((l4_addr_t)address + 12) & 0xff) << 24);

  *cellid   =   ((io_read_mword((l4_addr_t)address + 16) & 0xff) << 0)
              | ((io_read_mword((l4_addr_t)address + 20) & 0xff) << 8)
              | ((io_read_mword((l4_addr_t)address + 24) & 0xff) << 16)
              | ((io_read_mword((l4_addr_t)address + 28) & 0xff) << 24);
}
