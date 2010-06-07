/*
 * (c) 2008-2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
#include <l4/shmc/shmc.h>

#include <l4/sys/err.h>
#include <l4/sys/factory.h>
#include <l4/sys/task.h>
#include <l4/re/env.h>
#include <l4/re/c/util/cap_alloc.h>
#include <l4/re/c/rm.h>
#include <l4/re/c/mem_alloc.h>
#include <l4/re/c/namespace.h>

#include <l4/util/util.h>
#include <l4/util/atomic.h>

#include <string.h>
#include <stdio.h>

/* Head of a shared memory data memory, which has a size of multiple pages
 * No task local data must be in here (pointers, caps)
 */
typedef struct {
  l4_umword_t  lock;         // lock for handling chunks
  l4_addr_t    _first_chunk; // offset to first chunk
} shared_mem_t;

enum {
  SHMAREA_LOCK_FREE, SHMAREA_LOCK_TAKEN,
};


static inline l4shmc_chunk_desc_t *
chunk_get(l4_addr_t o, void *shm_local_addr)
{
  return (l4shmc_chunk_desc_t *)(o + (l4_addr_t)shm_local_addr);
}

L4_CV long
l4shmc_create(const char *shm_name, l4_umword_t shm_size)
{
  shared_mem_t *s;
  l4re_ds_t shm_ds = L4_INVALID_CAP;
  l4re_namespace_t tmp = L4_INVALID_CAP;
  long r = -L4_ENOMEM;

  if (l4_is_invalid_cap(shm_ds = l4re_util_cap_alloc()))
    goto out;

  if ((r = l4re_ma_alloc(shm_size, shm_ds, 0)))
    goto out;

  if ((r = l4re_rm_attach((void **)&s, shm_size, L4RE_RM_SEARCH_ADDR, shm_ds,
                          0, L4_PAGESHIFT)))
    goto out;

  s->_first_chunk = 0;

  r =  -L4_ENOMEM;
  if (l4_is_invalid_cap(tmp = l4re_util_cap_alloc()))
    goto out;

  tmp = l4re_get_env_cap(shm_name);
  if (l4_is_invalid_cap(tmp))
    {
      r = -L4_ENOENT;
      goto out;
    }

  if ((r = l4re_ns_register_obj_srv(tmp, "shm", shm_ds, L4RE_NS_REGISTER_RW)))
    goto out;

  l4re_rm_detach_unmap((l4_addr_t)s, L4RE_THIS_TASK_CAP);

  r = L4_EOK;

out:
  if (!l4_is_invalid_cap(tmp))
    l4re_util_cap_free(tmp);
  return r;
}


L4_CV long
l4shmc_attach_to(const char *shm_name, l4_umword_t timeout_ms,
                 l4shmc_area_t *shmarea)
{
  long r;
  l4re_namespace_t nssrv;

  strncpy(shmarea->_name, shm_name, sizeof(shmarea->_name));
  shmarea->_name[sizeof(shmarea->_name) - 1] = 0;

  if (l4_is_invalid_cap(shmarea->_shm_ds = l4re_util_cap_alloc()))
    return -L4_ENOMEM;

  nssrv = l4re_get_env_cap(shm_name);
  if (l4_is_invalid_cap(nssrv))
    {
      printf("shm: did not find '%s' namespace\n", shm_name);
      return -L4_ENOENT;
    }

  if (l4re_ns_query_to_srv(nssrv, "shm", shmarea->_shm_ds, timeout_ms))
    {
      printf("shm: did not find shm-ds 'shm'\n");
      return -L4_ENOENT;
    }

  shmarea->_local_addr = 0;
  if ((r = l4re_rm_attach(&shmarea->_local_addr,
                          l4shmc_area_size(shmarea),
                          L4RE_RM_SEARCH_ADDR, shmarea->_shm_ds,
                          0, L4_PAGESHIFT)))
    return r;

  return L4_EOK;
}


L4_CV long
l4shmc_add_chunk(l4shmc_area_t *shmarea,
                const char *chunk_name,
                l4_umword_t chunk_capacity,
                l4shmc_chunk_t *chunk)
{
  shared_mem_t *shm_addr = (shared_mem_t *)shmarea->_local_addr;

  l4shmc_chunk_desc_t *p;
  l4shmc_chunk_desc_t *prev = NULL;

  shm_addr->lock = 0;

  while (!l4util_cmpxchg(&shm_addr->lock, SHMAREA_LOCK_FREE,
	                 SHMAREA_LOCK_TAKEN))
    l4_sleep(1);
  asm volatile ("" : : : "memory");
  {
    l4_addr_t offs;
    long shm_sz;
    if (shm_addr->_first_chunk)
      {
        offs = shm_addr->_first_chunk;
        p = chunk_get(offs, shmarea->_local_addr);
        do
          {
            offs = p->_offset + p->_capacity + sizeof(*p);
            prev = p;
            p = chunk_get(p->_next, shmarea->_local_addr);
          }
        while (prev->_next);
      }
    else
      // first chunk starts right after shm-header
      offs = sizeof(shared_mem_t);

    if ((shm_sz = l4shmc_area_size(shmarea)) < 0)
      return -L4_ENOMEM;

    if (offs + chunk_capacity + sizeof(*p) >= (unsigned long)shm_sz)
      return -L4_ENOMEM; // no more free memory in this shm

    p = chunk_get(offs, shmarea->_local_addr);
    p->_offset = offs;
    p->_next = 0;
    p->_capacity = chunk_capacity;

    if (prev)
      prev->_next = offs;
    else
      shm_addr->_first_chunk = offs;
  }
  asm volatile ("" : : : "memory");
  shm_addr->lock = SHMAREA_LOCK_FREE;

  p->_size = 0;
  p->_status = L4SHMC_CHUNK_CLEAR;
  p->_magic = L4SHMC_CHUNK_MAGIC;
  strncpy(p->_name, chunk_name, sizeof(p->_name));
  p->_name[sizeof(p->_name) - 1] = 0;

  chunk->_chunk = p;
  chunk->_shm    = shmarea;
  chunk->_sig    = NULL;

  return L4_EOK;
}

L4_CV long
l4shmc_add_signal(l4shmc_area_t *shmarea,
                 const char *signal_name,
                 l4shmc_signal_t *signal)
{
  /* Now that the chunk is allocated in the shm, lets get the UIRQ
   * and register it */
  long r;
  l4re_namespace_t tmp;
  char b[L4SHMC_NAME_STRINGLEN + L4SHMC_SIGNAL_NAME_SIZE + 5]; // strings + "sig-"
  signal->_sigcap = l4re_util_cap_alloc();
  if (l4_is_invalid_cap(signal->_sigcap))
    return -L4_ENOMEM;

  if ((r = l4_error(l4_factory_create_irq(l4re_env()->factory,
                                          signal->_sigcap))))
    return r;

  snprintf(b, sizeof(b) - 1, "sig-%s", signal_name);
  b[sizeof(b) - 1] = 0;

  tmp = l4re_get_env_cap(shmarea->_name);
  if (l4_is_invalid_cap(tmp))
    return -L4_ENOENT;

  if (l4re_ns_register_obj_srv(tmp, b, signal->_sigcap, 0))
    {
      l4re_util_cap_free(tmp);
      return -L4_ENOENT;
    }

  l4re_util_cap_free(tmp);

  return L4_EOK;
}

L4_CV long
l4shmc_get_chunk_to(l4shmc_area_t *shmarea,
                    const char *chunk_name,
                    l4_umword_t timeout_ms,
                    l4shmc_chunk_t *chunk)
{
  l4_kernel_clock_t try_until = l4re_kip()->clock + (timeout_ms * 1000);
  shared_mem_t *shm_addr = (shared_mem_t *)shmarea->_local_addr;

  do
    {
      l4_addr_t offs = shm_addr->_first_chunk;
      while (offs)
        {
          l4shmc_chunk_desc_t *p;
          p = chunk_get(offs, shmarea->_local_addr);

          if (!strcmp(p->_name, chunk_name))
            { // found it!
               chunk->_shm    = shmarea;
               chunk->_chunk = p;
               chunk->_sig    = NULL;
               return L4_EOK;
            }
          offs = p->_next;
        }

      if (!timeout_ms)
        break;

      l4_sleep(100); // sleep 100ms
    }
  while (l4re_kip()->clock < try_until);

  return -L4_ENOENT;
}

L4_CV long
l4shmc_attach_signal_to(l4shmc_area_t *shmarea,
                       const char *signal_name,
                       l4_cap_idx_t thread,
                       l4_umword_t timeout_ms,
                       l4shmc_signal_t *signal)
{
  long r = L4_EOK;

  r = l4shmc_get_signal_to(shmarea, signal_name, timeout_ms, signal);
  if (r)
    goto out;

  if ((r = l4_error(l4_irq_attach(signal->_sigcap,
                                  (l4_umword_t)signal, thread))))
    {
      l4re_util_cap_free(signal->_sigcap);
      goto out;
    }

out:
  return r;
}

L4_CV long
l4shmc_get_signal_to(l4shmc_area_t *shmarea,
                    const char *signal_name,
                    l4_umword_t timeout_ms,
                    l4shmc_signal_t *signal)
{
  char b[L4SHMC_NAME_STRINGLEN + L4SHMC_SIGNAL_NAME_SIZE + 5]; // strings + "sig-"
  l4re_namespace_t ns;

  signal->_sigcap = l4re_util_cap_alloc();

  if (l4_is_invalid_cap(signal->_sigcap))
    return -L4_ENOMEM;

  ns = l4re_get_env_cap(shmarea->_name);
  if (l4_is_invalid_cap(ns))
    return -L4_ENOENT;

  snprintf(b, sizeof(b) - 1, "sig-%s", signal_name);
  b[sizeof(b) - 1] = 0;

  if (l4re_ns_query_to_srv(ns, b, signal->_sigcap, timeout_ms))
    return -L4_ENOENT;

  return L4_EOK;
}



L4_CV long
l4shmc_connect_chunk_signal(l4shmc_chunk_t *chunk,
                            l4shmc_signal_t *signal)
{
  chunk->_sig = signal;
  return L4_EOK;
}

L4_CV long
l4shmc_enable_signal(l4shmc_signal_t *s)
{
  return l4_error(l4_irq_unmask(s->_sigcap));
}

L4_CV long
l4shmc_enable_chunk(l4shmc_chunk_t *p)
{
  return l4shmc_enable_signal(p->_sig);
}

L4_CV long
l4shmc_wait_any_to(l4_timeout_t timeout, l4shmc_signal_t **p)
{
  l4_umword_t l;
  long r;

  if ((r = l4_ipc_error(l4_ipc_wait(l4_utcb(), &l, timeout), l4_utcb())))
    return r;

  *p = (l4shmc_signal_t *)l;

  return L4_EOK;
}

L4_CV long
l4shmc_wait_signal_to(l4shmc_signal_t *s, l4_timeout_t timeout)
{
  long r;

  if ((r = l4_ipc_error(l4_irq_receive(s->_sigcap, timeout), l4_utcb())))
    return r;

  return L4_EOK;
}

L4_CV long
l4shmc_wait_chunk_to(l4shmc_chunk_t *p, l4_timeout_t to)
{
  return l4shmc_wait_signal_to(p->_sig, to);
}
