#include <pthread.h>
#include <semaphore.h>

#include "acpi.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

ACPI_THREAD_ID
AcpiOsGetThreadId(void)
{
  return 10; //(unsigned long)pthread_self();
}

ACPI_STATUS
AcpiOsCreateSemaphore (
	uint32_t                        max_units,
	uint32_t                        initial_units,
	ACPI_SEMAPHORE                  *out_handle)
{
  if(max_units!=1 && initial_units!=0)
    {
      printf("%s:%d:%s: ERROR: (%d,%d)\n", __FILE__, __LINE__, __func__,
	     max_units, initial_units);
      exit(127);
    }

  pthread_mutex_t *m = malloc(sizeof(pthread_mutex_t));
//  pthread_mutex_init(m, NULL);
  *out_handle = m;
  return AE_OK;
}

ACPI_STATUS
AcpiOsDeleteSemaphore (
	ACPI_SEMAPHORE                  handle)
{
//  pthread_mutex_destroy((pthread_mutex_t*)handle);
  free(handle);
  return AE_OK;
}

ACPI_STATUS
AcpiOsWaitSemaphore (
	ACPI_SEMAPHORE                  handle,
	uint32_t                        units,
	uint16_t                        timeout)
{
#if 0
  if(units!=1 || timeout != 0xffff)
    printf("%s:%d:%s: ERROR handle=%p, (%d,%d)\n", __FILE__, __LINE__, __func__,
	   handle, units, (int)timeout);
  // enter the critical section
//  pthread_mutex_lock((pthread_mutex_t*)handle);
#endif
  return AE_OK;
}


ACPI_STATUS
AcpiOsSignalSemaphore (
	ACPI_SEMAPHORE                  handle,
	uint32_t                        units)
{
#if 0
  if(units!=1)
    printf("%s:%d:%s: ERROR: handle=%p, units=%d", __FILE__, __LINE__, __func__,
	   handle, units);
  // leave the critical section
//  pthread_mutex_unlock((pthread_mutex_t*)handle);
#endif
  return AE_OK;
}




ACPI_STATUS
AcpiOsCreateLock (ACPI_SPINLOCK *out_handle)
{
  pthread_mutex_t *m = malloc(sizeof(pthread_mutex_t));
//  pthread_mutex_init(m, NULL);
  *out_handle = (ACPI_SPINLOCK)m;
  return AE_OK;
}

void
AcpiOsDeleteLock (ACPI_SPINLOCK handle)
{
//  pthread_mutex_destroy((pthread_mutex_t*)handle);
  free(handle);
  return;
}

ACPI_CPU_FLAGS
AcpiOsAcquireLock (ACPI_SPINLOCK handle)
{
//  pthread_mutex_lock((pthread_mutex_t*)handle);
  return ACPI_NOT_ISR;
}

void
AcpiOsReleaseLock (
	ACPI_SPINLOCK                   handle,
	ACPI_CPU_FLAGS                  flags)
{
  // leave the critical section
//  pthread_mutex_unlock((pthread_mutex_t *)handle);
  return;
}

