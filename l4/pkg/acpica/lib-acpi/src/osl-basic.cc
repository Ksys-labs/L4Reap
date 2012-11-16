#include <l4/sys/compiler.h>

__BEGIN_DECLS
#include "acpi.h"
#include "acpiosxf.h"
#include "actypes.h"
__END_DECLS

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>

#include <l4/util/util.h>
#if defined(ARCH_amd64) || defined(ARCH_x86)
#include <l4/util/port_io.h>
#endif

#include <l4/re/env>
#include <l4/re/rm>

#include <l4/re/util/cap_alloc>

#define DEBUG_OSL_PORT_IO 0


ACPI_STATUS
AcpiOsInitialize (void)
{
  return AE_OK;
}

ACPI_STATUS
AcpiOsTerminate (void)
{
  return AE_OK;
}

void *
AcpiOsAllocate (ACPI_SIZE size)
{
  return malloc(size);
}

void
AcpiOsFree (void * memory)
{
  free(memory);
  return;
}

#ifndef HAVE_NOT_IMPLEMENTED_AcpiOsInstallInterruptHandler
ACPI_STATUS
AcpiOsInstallInterruptHandler (
	uint32_t                        interrupt_number,
	ACPI_OSD_HANDLER                service_routine,
	void                            *context)
{
  printf("%s:%d:%s(%d, %p, %p): UNINPLEMENTED\n",
         __FILE__, __LINE__, __func__,
         interrupt_number, service_routine, context);
  return AE_OK;
}
#endif

ACPI_STATUS
AcpiOsRemoveInterruptHandler (
	uint32_t                        interrupt_number,
	ACPI_OSD_HANDLER                service_routine)
{
  printf("%s:%d:%s(%d, %p): UNINPLEMENTED\n",
         __FILE__, __LINE__, __func__,
         interrupt_number, service_routine);
  return AE_OK;
}

ACPI_STATUS
AcpiOsExecute (
	ACPI_EXECUTE_TYPE                type,
	ACPI_OSD_EXEC_CALLBACK           function,
	void                            *context)
{
  printf("%s:%d:%s(%d, %p, %p): UNINPLEMENTED\n",
         __FILE__, __LINE__, __func__, type, function, context);
  return !AE_OK;
}

void
AcpiOsSleep (ACPI_INTEGER milliseconds)
{
  l4_sleep(milliseconds);
}

void
AcpiOsStall (uint32_t microseconds)
{
  l4_usleep(microseconds);
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsPredefinedOverride
 *
 * PARAMETERS:  InitVal     - Initial value of the predefined object
 *              NewVal      - The new value for the object
 *
 * RETURN:      Status, pointer to value.  Null pointer returned if not
 *              overriding.
 *
 * DESCRIPTION: Allow the OS to override predefined names
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsPredefinedOverride (
    const ACPI_PREDEFINED_NAMES *InitVal,
    ACPI_STRING                 *NewVal)
{

    if (!InitVal || !NewVal)
    {
        return (AE_BAD_PARAMETER);
    }

    *NewVal = NULL;
    return (AE_OK);
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsTableOverride
 *
 * PARAMETERS:  ExistingTable   - Header of current table (probably firmware)
 *              NewTable        - Where an entire new table is returned.
 *
 * RETURN:      Status, pointer to new table.  Null pointer returned if no
 *              table is available to override
 *
 * DESCRIPTION: Return a different version of a table if one is available
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsTableOverride (
    ACPI_TABLE_HEADER       *ExistingTable,
    ACPI_TABLE_HEADER       **NewTable)
{

    if (!ExistingTable || !NewTable)
    {
        return (AE_BAD_PARAMETER);
    }

    *NewTable = NULL;

#ifdef ACPI_EXEC_APP

    /* This code exercises the table override mechanism in the core */

    if (ACPI_COMPARE_NAME (ExistingTable->Signature, ACPI_SIG_DSDT))
    {
        /* override DSDT with itself */

        *NewTable = AcpiGbl_DbTablePtr;
    }
    return (AE_OK);
#else
    return AE_NO_ACPI_TABLES;
#endif
}

/*
 * ACPI Table interfaces
 */
ACPI_PHYSICAL_ADDRESS
AcpiOsGetRootPointer (void)
{
  ACPI_SIZE table_address = 0;
  printf("Find root Pointer\n");
  AcpiFindRootPointer(&table_address);
  printf("Find root Pointer: %x\n", table_address);
  return (ACPI_PHYSICAL_ADDRESS)table_address;
}

/******************************************************************************
 *
 * FUNCTION:    AcpiOsSignal
 *
 * PARAMETERS:  Function            ACPI CA signal function code
 *              Info                Pointer to function-dependent structure
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Miscellaneous functions
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsSignal (
    UINT32                  Function,
    void                    *Info)
{

    switch (Function)
    {
    case ACPI_SIGNAL_FATAL:
        break;

    case ACPI_SIGNAL_BREAKPOINT:

        if (Info)
        {
            AcpiOsPrintf ("AcpiOsBreakpoint: %s ****\n", Info);
        }
        else
        {
            AcpiOsPrintf ("At AcpiOsBreakpoint ****\n");
        }

        break;
    }


    return (AE_OK);
}

/******************************************************************************
 *
 * FUNCTION:    AcpiOsGetTimer
 *
 * PARAMETERS:  None
 *
 * RETURN:      Current time in 100 nanosecond units
 *
 * DESCRIPTION: Get the current system time
 *
 *****************************************************************************/

UINT64
AcpiOsGetTimer (void)
{
    struct timeval  time;

    gettimeofday(&time, NULL);

    /* Seconds * 10^7 = 100ns(10^-7), Microseconds(10^-6) * 10^1 = 100ns */

    return (((UINT64) time.tv_sec * 10000000) + ((UINT64) time.tv_usec * 10));
}


// from: acpica/source/os_specific/service_layers/osunixxf.c
/******************************************************************************
 *
 * FUNCTION:    AcpiOsReadMemory
 *
 * PARAMETERS:  Address             - Physical Memory Address to read
 *              Value               - Where value is placed
 *              Width               - Number of bits (8,16,32, or 64)
 *
 * RETURN:      Value read from physical memory address. Always returned
 *              as a 64-bit integer, regardless of the read width.
 *
 * DESCRIPTION: Read data from a physical memory address
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsReadMemory (
    ACPI_PHYSICAL_ADDRESS   Address,
    UINT64                  *Value,
    UINT32                  Width)
{

    printf("%s:%d:%s(%x, %p, %u): UNINPLEMENTED\n",
           __FILE__, __LINE__, __func__, Address, Value, Width);
    switch (Width)
    {
    case 8:
    case 16:
    case 32:
    case 64:
        *Value = 0;
        break;

    default:
        return (AE_BAD_PARAMETER);
    }
    return (AE_OK);
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsWriteMemory
 *
 * PARAMETERS:  Address             - Physical Memory Address to write
 *              Value               - Value to write
 *              Width               - Number of bits (8,16,32, or 64)
 *
 * RETURN:      None
 *
 * DESCRIPTION: Write data to a physical memory address
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsWriteMemory (
    ACPI_PHYSICAL_ADDRESS   Address,
    UINT64                  Value,
    UINT32                  Width)
{
  printf("%s:%d:%s(%x, %llu, %u): UNINPLEMENTED\n",
         __FILE__, __LINE__, __func__, Address, Value, Width);
    return (AE_OK);
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsPhysicalTableOverride
 *
 * PARAMETERS:  ExistingTable       - Header of current table (probably firmware)
 *              NewAddress          - Where new table address is returned
 *                                    (Physical address)
 *              NewTableLength      - Where new table length is returned
 *
 * RETURN:      Status, address/length of new table. Null pointer returned
 *              if no table is available to override.
 *
 * DESCRIPTION: Returns AE_SUPPORT, function not used in user space.
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsPhysicalTableOverride (
    ACPI_TABLE_HEADER       *ExistingTable,
    ACPI_PHYSICAL_ADDRESS   *NewAddress,
    UINT32                  *NewTableLength)
{
  printf("%s:%d:%s(%p, %p, %p): UNINPLEMENTED\n",
         __FILE__, __LINE__, __func__, ExistingTable, NewAddress, NewTableLength);

    return (AE_SUPPORT);
}

/******************************************************************************
 *
 * FUNCTION:    AcpiOsWaitEventsComplete
 *
 * PARAMETERS:  None
 *
 * RETURN:      None
 *
 * DESCRIPTION: Wait for all asynchronous events to complete. This
 *              implementation does nothing.
 *
 *****************************************************************************/

void
AcpiOsWaitEventsComplete (
    void)
{
    return;
}
