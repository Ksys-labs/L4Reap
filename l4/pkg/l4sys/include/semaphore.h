/*
 * \file
 * \brief   User-Level Semaphores.
 * \ingroup l4_sem_api
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 *
 * As a special exception, you may use this file as part of a free software
 * library without restriction.  Specifically, if other files instantiate
 * templates or use macros or inline functions from this file, or you compile
 * this file and link it with other files to produce an executable, this
 * file does not by itself cause the resulting executable to be covered by
 * the GNU General Public License.  This exception does not however
 * invalidate any other reasons why the executable file might be covered by
 * the GNU General Public License.
 */
/*****************************************************************************/
#pragma once

#include <l4/sys/types.h>

/*****************************************************************************
 *** Prototypes
 *****************************************************************************/

/*
 * \defgroup l4_sem_api Semaphore
 * \ingroup l4_kernel_object_api
 *
 * \brief Class definition of the Semaphore object.
 *
 * <c>\#include <l4/sys/semaphore.h> </c>
 *
 * The semaphore class provides a fast user-level
 * semaphore implementation. The main idea is to only invoke the micro kernel
 * in the contention case and to do everything else with atomic
 * read-modify-write instructions on user level.
 *
 * The semaphore currently implements a priority sorted list of threads that are
 * blocked on a semaphore. However, the semaphore does not implement measures
 * to prevent priority inversion. Therefore a more pessimistic implementation
 * would be necessary.
 *
 * Every semaphore kernel object has to be accompanied by a user-level data
 * structure that contains a counter.
 *
 * To create a semaphore kernel object see \ref l4_factory_api
 * (l4_factory_create_semaphore()).
 */


/*
 * \brief Semaphore operations.
 * \ingroup l4_sem_api
 * \internal
 */
enum l4_sem_ops_t
{
  L4_USEM_SLEEP = 6,
  L4_USEM_WAKEUP = 7,
};

/*
 * \brief Result values of the semaphore down operation.
 * \ingroup l4_sem_api
 */
enum l4_u_semaphore_ret_t
{
  L4_USEM_OK      = 0, ///< OK, got the semaphore
  L4_USEM_RETRY   = 1, ///< never seen by user
  L4_USEM_TIMEOUT = 2, ///< The timeout has hit
  L4_USEM_INVALID = 3  ///< The Semaphore/Lock was destroyed
};

/*
 * \brief Semaphore structure.
 * \ingroup l4_sem_api
 */
typedef struct l4_u_semaphore_t
{
  l4_mword_t counter; /*< Counter */
  l4_umword_t flags;  /*< Flags */
} __attribute__((aligned(sizeof(l4_umword_t)*2))) l4_u_semaphore_t;

/*
 * \brief Initialize semaphore data structure.
 * \ingroup l4_sem_api
 * \param count   Initial counter value
 * \param sem     Semaphore data structure pointer
 *
 * The counter value should be greater than zero and indicates how
 * many threads can enter the critical section.
 *
 * \note After initialization this data stucture should never be
 *       touched by any application.
 */
L4_INLINE void
l4_usem_init(long count, l4_u_semaphore_t *sem) L4_NOTHROW;

/*
 * \brief Semaphore down operation with timeout.
 * \ingroup l4_sem_api
 * \param ksem    Capability selector to semaphore object
 * \param sem     Pointer to semaphore data structure
 * \param timeout Absolute or relative timeout
 *
 * The timeout specifies the maximum time a thread blocks
 * while trying to acquire the semaphore.
 *
 * \return See #l4_u_semaphore_t
 */
L4_INLINE l4_msgtag_t
l4_usem_down_to(l4_cap_idx_t ksem, l4_u_semaphore_t *sem,
                l4_timeout_s timeout) L4_NOTHROW;

/*
 * \brief Semaphore down operation with infinite timeout.
 * \ingroup l4_sem_api
 * \param ksem    Capability selector to semaphore object
 * \param sem     Pointer to semaphore data structure
 *
 * The thread blocks without any timeout while waiting 
 * for the semaphore.
 *
 * \return See #l4_u_semaphore_t
 */
//todo can this operation fail???
L4_INLINE l4_msgtag_t
l4_usem_down(l4_cap_idx_t ksem, l4_u_semaphore_t *sem) L4_NOTHROW;

/*
 * \brief Semaphore up operation.
 * \ingroup l4_sem_api
 * \param ksem    Capability selector to semaphore object
 * \param sem     Pointer to semaphore data structure
 *
 * \pre This function should only be called after a successful l4_sem_down
 *      operation.
 */
L4_INLINE l4_msgtag_t
l4_usem_up(l4_cap_idx_t ksem, l4_u_semaphore_t *sem) L4_NOTHROW;



/*****
 * Implementations
 */
#include <l4/sys/__semaphore_impl.h>


L4_INLINE void
l4_usem_init(long count, l4_u_semaphore_t *sem) L4_NOTHROW
{
  sem->counter = count;
  sem->flags = 0;
}


L4_INLINE l4_msgtag_t
l4_usem_down(l4_cap_idx_t ksem, l4_u_semaphore_t *sem) L4_NOTHROW
{
  return l4_usem_down_to(ksem, sem, L4_IPC_TIMEOUT_NEVER);
}

