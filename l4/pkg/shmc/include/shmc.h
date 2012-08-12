/**
 * \file
 * \brief Shared memory library header file.
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Björn Döbel <doebel@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
#pragma once

#include <l4/sys/linkage.h>
#include <l4/sys/types.h>
#include <l4/sys/err.h>

/**
 * \defgroup api_l4shm Shared Memory Library
 *
 * L4SHM provides a shared memory infrastructure that establishes a
 * shared memory area between multiple parties and uses a fast notification
 * mechanism.
 *
 * A shared memory area consists of chunks and signals. A chunk is a
 * defined chunk of memory within the memory area with a maximum size. A
 * chunk is filled (written) by a producer and read by a consumer. When a
 * producer has finished writing to the chunk it signals a data ready
 * notification to the consumer.
 *
 * A consumer attaches to a chunk and waits for the producer to fill the
 * chunk. After reading out the chunk it marks the chunk free again.
 *
 * A shared memory area can have multiple chunks.
 *
 * The interface is divided in three roles.
 * - The master role, reponsible for setting up a shared memory area.
 * - A producer, generating data into a chunk
 * - A consumer, receiving data.
 *
 *
 * A signal can be connected with a chunk or can be used independently
 * (e.g. for multiple chunks).
 *
 * \example examples/libs/shmc/prodcons.c
 * Simple shared memory example.
 *
 */
/**
 * \defgroup api_l4shmc_chunk Chunks
 * \ingroup api_l4shm
 *
 * \defgroup api_l4shmc_chunk_prod Producer
 * \ingroup api_l4shmc_chunk
 *
 * \defgroup api_l4shmc_chunk_cons Consumer
 * \ingroup api_l4shmc_chunk
 *
 * \defgroup api_l4shmc_signal Signals
 * \ingroup api_l4shm
 *
 * \defgroup api_l4shmc_signal_prod Producer
 * \ingroup api_l4shmc_signal
 *
 * \defgroup api_l4shmc_signal_cons Consumer
 * \ingroup api_l4shmc_signal
 */

#define __INCLUDED_FROM_L4SHMC_H__
#include <l4/shmc/types.h>

__BEGIN_DECLS

/**
 * \brief Create a shared memory area.
 * \ingroup api_l4shm
 *
 * \param shmc_name   Name of the shared memory area.
 * \param shm_size    Size of the whole shared memory area.
 *
 * \return 0 on success, <0 on error
 */
L4_CV long
l4shmc_create(const char *shmc_name, l4_umword_t shm_size);

/**
 * \brief Attach to a shared memory area.
 * \ingroup api_l4shm
 *
 * \param  shmc_name  Name of the shared memory area.
 * \retval shmarea    Pointer to shared memory area descriptor to be filled
 *                    with information for the shared memory area.
 * \return 0 on success, <0 on error
 */
L4_CV L4_INLINE long
l4shmc_attach(const char *shmc_name, l4shmc_area_t *shmarea);

/**
 * \brief Attach to a shared memory area, with limited waiting.
 * \ingroup api_l4shm
 *
 * \param  shmc_name  Name of the shared memory area.
 * \param  timeout_ms Timeout to wait for shm area in milliseconds.
 * \retval shmarea    Pointer to shared memory area descriptor to be filled
 *                    with information for the shared memory area.
 * \return 0 on success, <0 on error
 */
L4_CV long
l4shmc_attach_to(const char *shmc_name, l4_umword_t timeout_ms,
                 l4shmc_area_t *shmarea);

/**
 * \brief Add a chunk in the shared memory area.
 * \ingroup api_l4shmc_chunk
 *
 * \param shmarea         The shared memory area to put the chunk in.
 * \param chunk_name      Name of the chunk.
 * \param chunk_capacity  Capacity for payload of the chunk in bytes.
 * \retval chunk          Chunk structure to fill in.
 *
 * \return 0 on success, <0 on error
 */
L4_CV long
l4shmc_add_chunk(l4shmc_area_t *shmarea,
                 const char *chunk_name,
                 l4_umword_t chunk_capacity,
                 l4shmc_chunk_t *chunk);

/**
 * \brief Add a signal for the shared memory area.
 * \ingroup api_l4shmc_signal
 *
 * \param shmarea         The shared memory area to put the chunk in.
 * \param signal_name     Name of the signal.
 * \retval signal         Signal structure to fill in.
 *
 * \return 0 on success, <0 on error
 */
L4_CV long
l4shmc_add_signal(l4shmc_area_t *shmarea,
                  const char *signal_name,
                  l4shmc_signal_t *signal);

/**
 * \brief Trigger a signal.
 * \ingroup api_l4shmc_signal_prod
 *
 * \param signal  Signal to trigger.
 * \return 0 on success, <0 on error
 */
L4_CV L4_INLINE long
l4shmc_trigger(l4shmc_signal_t *signal);

/**
 * \brief Try to mark chunk busy.
 * \ingroup api_l4shmc_chunk_prod
 *
 * \param chunk  chunk to mark.
 * \return 0 if chunk could be taken, <0 if not (try again then)
 */
L4_CV L4_INLINE long
l4shmc_chunk_try_to_take(l4shmc_chunk_t *chunk);

/**
 * \brief Mark chunk as filled (ready).
 * \ingroup api_l4shmc_chunk_prod
 *
 * \param chunk   chunk.
 * \param size     Size of data in the chunk, in bytes.
 * \return 0 on success, <0 on error
 */
L4_CV L4_INLINE long
l4shmc_chunk_ready(l4shmc_chunk_t *chunk, l4_umword_t size);

/**
 * \brief Mark chunk as filled (ready) and signal consumer.
 * \ingroup api_l4shmc_chunk_prod
 *
 * \param chunk   chunk.
 * \param size     Size of data in the chunk, in bytes.
 * \return 0 on success, <0 on error
 */
L4_CV L4_INLINE long
l4shmc_chunk_ready_sig(l4shmc_chunk_t *chunk, l4_umword_t size);

/**
 * \brief Get chunk out of shared memory area.
 * \ingroup api_l4shmc_chunk
 *
 * \param shmarea     Shared memory area.
 * \param chunk_name  Name of the chunk.
 * \retval chunk      Chunk data structure to fill.
 * \return 0 on success, <0 on error
 */
L4_CV L4_INLINE long
l4shmc_get_chunk(l4shmc_area_t *shmarea,
                 const char *chunk_name,
                 l4shmc_chunk_t *chunk);

/**
 * \brief Get chunk out of shared memory area, with timeout.
 * \ingroup api_l4shmc_chunk
 *
 * \param shmarea     Shared memory area.
 * \param chunk_name  Name of the chunk.
 * \param timeout_ms  Timeout in milliseconds to wait for the chunk to appear
 *                    in the shared memory area.
 * \retval chunk     chunk data structure to fill.
 * \return 0 on success, <0 on error
 */
L4_CV long
l4shmc_get_chunk_to(l4shmc_area_t *shmarea,
                    const char *chunk_name,
                    l4_umword_t timeout_ms,
                    l4shmc_chunk_t *chunk);

/**
 * \brief Iterate over names of all existing chunks
 * \ingroup api_l4shmc_chunk
 *
 * \param shmarea     Shared memory area.
 * \param chunk_name  Where the name of the current chunk will be stored
 * \param offs        0 to start iteration, return value of previous
 *                    call to l4shmc_iterate_chunk() to get next chunk
 * \return <0 on error, 0 if no more chunks, >0 iterator value for next call
 */
L4_CV long
l4shmc_iterate_chunk(l4shmc_area_t *shmarea, const char **chunk_name,
                     long offs);

/**
 * \brief Attach to signal.
 * \ingroup api_l4shmc_signal
 *
 * \param shmarea     Shared memory area.
 * \param signal_name Name of the signal.
 * \param thread      Thread capability index to attach the signal to.
 * \retval signal     Signal data structure to fill.
 * \return 0 on success, <0 on error
 */
L4_CV L4_INLINE long
l4shmc_attach_signal(l4shmc_area_t *shmarea,
                     const char *signal_name,
                     l4_cap_idx_t thread,
                     l4shmc_signal_t *signal);

/**
 * \brief Attach to signal, with timeout.
 * \ingroup api_l4shmc_signal
 *
 * \param shmarea     Shared memory area.
 * \param signal_name Name of the signal.
 * \param thread      Thread capability index to attach the signal to.
 * \param timeout_ms  Timeout in milliseconds to wait for the chunk to appear
 *                    in the shared memory area.
 * \retval signal     Signal data structure to fill.
 * \return 0 on success, <0 on error
 */
L4_CV long
l4shmc_attach_signal_to(l4shmc_area_t *shmarea,
                        const char *signal_name,
                        l4_cap_idx_t thread,
                        l4_umword_t timeout_ms,
                        l4shmc_signal_t *signal);

/**
 * \brief Get signal object from the shared memory area.
 * \ingroup api_l4shmc_signal
 *
 * \param
 */
L4_CV long
l4shmc_get_signal_to(l4shmc_area_t *shmarea,
                     const char *signal_name,
                     l4_umword_t timeout_ms,
                     l4shmc_signal_t *signal);

L4_CV L4_INLINE long
l4shmc_get_signal(l4shmc_area_t *shmarea,
                  const char *signal_name,
                  l4shmc_signal_t *signal);


/**
 * \brief Enable a signal.
 * \ingroup api_l4shmc_signal_cons
 *
 * \param signal  Signal to enable.
 * \return 0 on success, <0 on error
 *
 * A signal must be enabled before waiting when the consumer waits on any
 * signal. Enabling is not needed if the consumer waits for a specific
 * signal or chunk.
 */
L4_CV long
l4shmc_enable_signal(l4shmc_signal_t *signal);

/**
 * \brief Enable a signal connected with a chunk.
 * \ingroup api_l4shmc_chunk_cons
 *
 * \param chunk  Chunk to enable.
 * \return 0 on success, <0 on error
 *
 * A signal must be enabled before waiting when the consumer waits on any
 * signal. Enabling is not needed if the consumer waits for a specific
 * signal or chunk.
 */
L4_CV long
l4shmc_enable_chunk(l4shmc_chunk_t *chunk);

/**
 * \brief Wait on any signal.
 * \ingroup api_l4shmc_signal_cons
 *
 * \retval retsignal Signal received.
 * \return 0 on success, <0 on error
 */
L4_CV L4_INLINE long
l4shmc_wait_any(l4shmc_signal_t **retsignal);

/**
 * \brief Check whether any waited signal has an event pending.
 * \ingroup api_l4shmc_signal_cons
 *
 * \retval retsignal Signal that has the event pending if any.
 * \return 0 on success, <0 on error
 *
 * The return code indicates whether an event was pending or not. Success
 * means an event was pending, if an receive timeout error is returned no
 * event was pending.
 */
L4_CV L4_INLINE long
l4shmc_wait_any_try(l4shmc_signal_t **retsignal);

/**
 * \brief Wait for any signal with timeout.
 * \ingroup api_l4shmc_signal_cons
 *
 * \param  timeout   Timeout.
 * \retval retsignal Signal that has the event pending if any.
 * \return 0 on success, <0 on error
 *
 * The return code indicates whether an event was pending or not. Success
 * means an event was pending, if an receive timeout error is returned no
 * event was pending.
 */
L4_CV long
l4shmc_wait_any_to(l4_timeout_t timeout, l4shmc_signal_t **retsignal);

/**
 * \brief Wait on a specific signal.
 * \ingroup api_l4shmc_signal_cons
 *
 * \param signal Signal to wait for.
 * \return 0 on success, <0 on error
 */
L4_CV L4_INLINE long
l4shmc_wait_signal(l4shmc_signal_t *signal);

/**
 * \brief Wait on a specific signal, with timeout.
 * \ingroup api_l4shmc_signal_cons
 *
 * \param signal Signal to wait for.
 * \param timeout Timeout.
 * \return 0 on success, <0 on error
 */
L4_CV long
l4shmc_wait_signal_to(l4shmc_signal_t *signal, l4_timeout_t timeout);

/**
 * \brief Check whether a specific signal has an event pending.
 * \ingroup api_l4shmc_signal_cons
 *
 * \param signal Signal to check.
 * \return 0 on success, <0 on error
 *
 * The return code indicates whether an event was pending or not. Success
 * means an event was pending, if an receive timeout error is returned no
 * event was pending.
 */
L4_CV L4_INLINE long
l4shmc_wait_signal_try(l4shmc_signal_t *signal);

/**
 * \brief Wait on a specific chunk.
 * \ingroup api_l4shmc_chunk_cons
 *
 * \param chunk Chunk to wait for.
 * \return 0 on success, <0 on error
 */
L4_CV L4_INLINE long
l4shmc_wait_chunk(l4shmc_chunk_t *chunk);

/**
 * \brief Check whether a specific chunk has an event pending, with timeout.
 * \ingroup api_l4shmc_chunk_cons
 *
 * \param chunk Chunk to check.
 * \param timeout Timeout.
 * \return 0 on success, <0 on error
 *
 * The return code indicates whether an event was pending or not. Success
 * means an event was pending, if an receive timeout error is returned no
 * event was pending.
 */
L4_CV long
l4shmc_wait_chunk_to(l4shmc_chunk_t *chunk, l4_timeout_t timeout);

/**
 * \brief Check whether a specific chunk has an event pending.
 * \ingroup api_l4shmc_chunk_cons
 *
 * \param chunk Chunk to check.
 * \return 0 on success, <0 on error
 *
 * The return code indicates whether an event was pending or not. Success
 * means an event was pending, if an receive timeout error is returned no
 * event was pending.
 */
L4_CV L4_INLINE long
l4shmc_wait_chunk_try(l4shmc_chunk_t *chunk);

/**
 * \brief Mark a chunk as free.
 * \ingroup api_l4shmc_chunk_cons
 *
 * \param chunk Chunk to mark as free.
 * \return 0 on success, <0 on error
 */
L4_CV L4_INLINE long
l4shmc_chunk_consumed(l4shmc_chunk_t *chunk);

/**
 * \brief Connect a signal with a chunk.
 * \ingroup api_l4shm
 *
 * \param chunk  Chunk to attach the signal to.
 * \param signal Signal to attach.
 * \return 0 on success, <0 on error
 */
L4_CV long
l4shmc_connect_chunk_signal(l4shmc_chunk_t *chunk,
                            l4shmc_signal_t *signal);

/**
 * \brief Check whether data is available.
 * \ingroup api_l4shmc_chunk_cons
 *
 * \param chunk Chunk to check.
 * \return 0 on success, <0 on error
 */
L4_CV L4_INLINE long
l4shmc_is_chunk_ready(l4shmc_chunk_t *chunk);

/**
 * \brief Check whether chunk is free.
 * \ingroup api_l4shmc_chunk_prod
 *
 * \param chunk Chunk to check.
 * \return 0 on success, <0 on error
 */
L4_CV L4_INLINE long
l4shmc_is_chunk_clear(l4shmc_chunk_t *chunk);

/**
 * \brief Get data pointer to chunk.
 * \ingroup api_l4shmc_chunk
 *
 * \param chunk Chunk.
 * \return 0 on success, <0 on error
 */
L4_CV L4_INLINE void *
l4shmc_chunk_ptr(l4shmc_chunk_t *chunk);

/**
 * \brief Get current size of a chunk.
 * \ingroup api_l4shmc_chunk_cons
 *
 * \param chunk Chunk.
 * \return 0 on success, <0 on error
 */
L4_CV L4_INLINE long
l4shmc_chunk_size(l4shmc_chunk_t *chunk);

/**
 * \brief Get capacity of a chunk.
 * \ingroup api_l4shmc_chunk
 *
 * \param chunk Chunk.
 * \return 0 on success, <0 on error
 */
L4_CV L4_INLINE long
l4shmc_chunk_capacity(l4shmc_chunk_t *chunk);

/**
 * \brief Get the signal of a chunk.
 * \ingroup api_l4shmc_chunk
 *
 * \param chunk Chunk.
 * \return 0 if no signal has been register with this chunk,
 *         signal otherwise
 */
L4_CV L4_INLINE l4shmc_signal_t *
l4shmc_chunk_signal(l4shmc_chunk_t *chunk);

/**
 * \brief Get the signal capability of a signal.
 * \ingroup api_l4shmc_signal
 *
 * \param signal Signal.
 * \return Capability of the signal object.
 */
L4_CV L4_INLINE l4_cap_idx_t
l4shmc_signal_cap(l4shmc_signal_t *signal);

/**
 * \brief Check magic value of a chunk.
 * \ingroup api_l4shmc_signal
 *
 * \param chunk Chunk.
 * \return True if chunk is ok (magic value valid), false if not.
 */
L4_CV L4_INLINE long
l4shmc_check_magic(l4shmc_chunk_t *chunk);

/**
 * \brief Get size of shared memory area.
 * \ingroup api_l4shm
 *
 * \param shmarea Shared memory area.
 * \return <0 on error, otherwise: size of the shared memory area
 */
L4_CV L4_INLINE long
l4shmc_area_size(l4shmc_area_t *shmarea);

/**
 * \brief Get free size of shared memory area. To get the max size to
 * pass to l4shmc_add_chunk, substract l4shmc_chunk_overhead().
 * \ingroup api_l4shm
 *
 * \param shmarea Shared memory area.
 * \return <0 on error, otherwise: free capacity in the area.
 *
 */
L4_CV long
l4shmc_area_size_free(l4shmc_area_t *shmarea);

/**
 * \brief Get memory overhead per area that is not available for chunks
 * \ingroup api_l4shm
 *
 * \return size of the overhead in bytes
 */
L4_CV long
l4shmc_area_overhead(void);

/**
 * \brief Get memory overhead required in addition to the chunk capacity
 * for adding one chunk
 * \ingroup api_l4shm
 *
 * \return size of the overhead in bytes
 */
L4_CV long
l4shmc_chunk_overhead(void);

#include <l4/shmc/internal.h>

__END_DECLS
