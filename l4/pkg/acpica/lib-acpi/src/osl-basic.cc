#include <l4/sys/compiler.h>

__BEGIN_DECLS
#include "acpi.h"
#include "acpiosxf.h"
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

ACPI_STATUS
AcpiOsInstallInterruptHandler (
	uint32_t                        /* interrupt_number */,
	ACPI_OSD_HANDLER                /* service_routine */,
	void                            * /*context*/)
{
  printf("%s:%d:%s: UNINPLEMENTED\n", __FILE__, __LINE__, __func__);
  return AE_OK;
}

ACPI_STATUS
AcpiOsRemoveInterruptHandler (
	uint32_t                        /* interrupt_number */,
	ACPI_OSD_HANDLER                /* service_routine */)
{
  printf("%s:%d:%s: UNINPLEMENTED\n", __FILE__, __LINE__, __func__);
  return AE_OK;
}

ACPI_STATUS
AcpiOsExecute (
	ACPI_EXECUTE_TYPE                /*type */,
	ACPI_OSD_EXEC_CALLBACK           /*function */,
	void                            * /*context */)
{
  printf("%s:%d:%s: UNINPLEMENTED\n", __FILE__, __LINE__, __func__);
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


/*
 * Platform and hardware-independent physical memory interfaces
 */
ACPI_STATUS
AcpiOsReadMemory (
	ACPI_PHYSICAL_ADDRESS           /*address*/,
	uint32_t                             * /*value*/,
	uint32_t                             /*width*/)
{
  printf("%s:%d:%s: UNINPLEMENTED\n", __FILE__, __LINE__, __func__);
  return !AE_OK;
}

ACPI_STATUS
AcpiOsWriteMemory (
	ACPI_PHYSICAL_ADDRESS            /*address */,
	uint32_t                            /*  value */,
	uint32_t                             /* width */)
{
  printf("%s:%d:%s: UNINPLEMENTED\n", __FILE__, __LINE__, __func__);
  return !AE_OK;
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
 * FUNCTION:    AcpiOsValidateInterface
 *
 * PARAMETERS:  Interface           - Requested interface to be validated
 *
 * RETURN:      AE_OK if interface is supported, AE_SUPPORT otherwise
 *
 * DESCRIPTION: Match an interface string to the interfaces supported by the
 *              host. Strings originate from an AML call to the _OSI method.
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsValidateInterface (
    char                    * /*Interface*/)
{

    return (AE_SUPPORT);
}


/* TEMPORARY STUB FUNCTION */
void
AcpiOsDerivePciId(
    ACPI_HANDLE             /* rhandle */,
    ACPI_HANDLE             /* chandle */,
    ACPI_PCI_ID             ** /* PciId */)
{

}

/******************************************************************************
 *
 * FUNCTION:    AcpiOsValidateAddress
 *
 * PARAMETERS:  SpaceId             - ACPI space ID
 *              Address             - Physical address
 *              Length              - Address length
 *
 * RETURN:      AE_OK if Address/Length is valid for the SpaceId. Otherwise,
 *              should return AE_AML_ILLEGAL_ADDRESS.
 *
 * DESCRIPTION: Validate a system address via the host OS. Used to validate
 *              the addresses accessed by AML operation regions.
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsValidateAddress (
    UINT8                   /* SpaceId */,
    ACPI_PHYSICAL_ADDRESS   /* Address */,
    ACPI_SIZE               /* Length */)
{

    return (AE_OK);
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


