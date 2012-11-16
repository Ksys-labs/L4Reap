/*
 * (c) 2010 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <l4/sys/compiler.h>
#include <l4/sigma0/sigma0.h>

#include "debug.h"

__BEGIN_DECLS
#include "acpi.h"
#include "acpiosxf.h"
__END_DECLS

#include "pci.h"
#include "res.h"

#if defined(ARCH_amd64) || defined(ARCH_x86)

#include <l4/util/port_io.h>
#define DEBUG_OSL_PORT_IO 0
/*
 * Platform and hardware-independent I/O interfaces
 */
ACPI_STATUS
AcpiOsReadPort (
	ACPI_IO_ADDRESS                 address,
	UINT32                         *value,
	UINT32                          width)
{
  if(DEBUG_OSL_PORT_IO)
    d_printf(DBG_ALL, "IN: adr=0x%x, width=%i\n", address, width);

  if (address == 0x80)
    return AE_OK;

  switch(width)
    {
    case 8:
      if (res_get_ioport(address, 0) < 0)
	return AE_BAD_PARAMETER;
      *value = l4util_in8((l4_uint16_t)address);
      break;
    case 16:
      if (res_get_ioport(address, 1) < 0)
	return AE_BAD_PARAMETER;
      *value = l4util_in16((l4_uint16_t)address);
      break;
    case 32:
      if (res_get_ioport(address, 2) < 0)
	return AE_BAD_PARAMETER;
      *value = l4util_in32((l4_uint16_t)address);
      break;
    default :
      return AE_BAD_PARAMETER;
    }
  if(DEBUG_OSL_PORT_IO)
    d_printf(DBG_ALL, "\tport(0x%x)=>0x%x\n",address,*value);
  return AE_OK;
}

ACPI_STATUS
AcpiOsWritePort (
	ACPI_IO_ADDRESS                 address,
	UINT32                          value,
	UINT32                          width)
{
  if(DEBUG_OSL_PORT_IO)
    d_printf(DBG_ALL, "\tport(0x%x)<=0x%x\n",address,value);

  if (address == 0x80)
    return AE_OK;

  switch(width)
    {
    case 8:
      if (res_get_ioport(address, 0) < 0)
	return AE_BAD_PARAMETER;
      l4util_out8((l4_uint8_t)value,(l4_uint16_t)address);
      break;
    case 16:
      if (res_get_ioport(address, 1) < 0)
	return AE_BAD_PARAMETER;
      l4util_out16((l4_uint16_t)value,(l4_uint16_t)address);
      break;
    case 32:
      if (res_get_ioport(address, 2) < 0)
	return AE_BAD_PARAMETER;
      l4util_out32((l4_uint32_t)value,(l4_uint32_t)address);
      break;
    default :
      return AE_BAD_PARAMETER;
    }
  return AE_OK;
}
#else

ACPI_STATUS
AcpiOsReadPort (
	ACPI_IO_ADDRESS                 /*address*/,
	UINT32                        * /*value*/,
	UINT32                        /*width*/)
{
  return AE_NO_MEMORY;
}

ACPI_STATUS
AcpiOsWritePort (
	ACPI_IO_ADDRESS                 /*address*/,
	UINT32                        /*value*/,
	UINT32                        /*width*/)
{
  return AE_NO_MEMORY;
}
#endif

void *
AcpiOsMapMemory (
	ACPI_PHYSICAL_ADDRESS           where,
	ACPI_SIZE                       length)
{
  void *virt = (void*)res_map_iomem(where, length);

  d_printf(DBG_DEBUG, "%s(%x, %x) = %lx\n", __func__, where, length, (unsigned long)virt);

  return virt;
}

void
AcpiOsUnmapMemory (
	void                            *logical_address,
	ACPI_SIZE                       size)
{
  (void)logical_address;
  (void)size;
//  l4io_release_iomem((l4_addr_t)logical_address, size);
  return;
}



/******************************************************************************
 *
 * FUNCTION:    AcpiOsReadPciConfiguration
 *
 * PARAMETERS:  PciId               Seg/Bus/Dev
 *              Register            Device Register
 *              Value               Buffer where value is placed
 *              Width               Number of bits
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Read data from PCI configuration space
 *
 *****************************************************************************/

extern inline
l4_uint32_t
pci_conf_addr(l4_uint32_t bus, l4_uint32_t dev, l4_uint32_t fn, l4_uint32_t reg)
{ return 0x80000000 | (bus << 16) | (dev << 11) | (fn << 8) | (reg & ~3); }

ACPI_STATUS
AcpiOsReadPciConfiguration (
    ACPI_PCI_ID             *PciId,
    UINT32                  Register,
    UINT64                  *Value,
    UINT32                  Width)
{
  //printf("%s: ...\n", __func__);
  Pci_root_bridge *rb = pci_root_bridge(PciId->Segment);
  if (!rb)
    return AE_BAD_PARAMETER;

  int r = rb->cfg_read(PciId->Bus, (PciId->Device << 16) | PciId->Function,
      Register, (l4_uint32_t *)Value, Hw::Pci::cfg_w_to_o(Width));

  if (r < 0)
    return AE_BAD_PARAMETER;

  return AE_OK;
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsWritePciConfiguration
 *
 * PARAMETERS:  PciId               Seg/Bus/Dev
 *              Register            Device Register
 *              Value               Value to be written
 *              Width               Number of bits
 *
 * RETURN:      Status.
 *
 * DESCRIPTION: Write data to PCI configuration space
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsWritePciConfiguration (
    ACPI_PCI_ID             *PciId,
    UINT32                  Register,
    ACPI_INTEGER            Value,
    UINT32                  Width)
{
  //printf("%s: ...\n", __func__);
  Pci_root_bridge *rb = pci_root_bridge(PciId->Segment);
  if (!rb)
    return AE_BAD_PARAMETER;

  int r = rb->cfg_write(PciId->Bus, (PciId->Device << 16) | PciId->Function,
      Register, Value, Hw::Pci::cfg_w_to_o(Width));

  if (r < 0)
    return AE_BAD_PARAMETER;

  return AE_OK;

  return (AE_OK);
}

