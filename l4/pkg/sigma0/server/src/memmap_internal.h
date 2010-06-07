/*
 * (c) 2008-2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include <l4/sys/utcb.h>
#include <l4/sys/types.h>

enum
{
  L4_fpage_cached   = 0x30,
  L4_fpage_uncached = 0x10,
};

struct Answer
{
  l4_utcb_t *utcb;
  l4_msgtag_t tag;

  Answer(l4_utcb_t *utcb) : utcb(utcb), tag(l4_msgtag(0,0,0,0)) {}

  unsigned long snd_base() const
  { return l4_utcb_mr_u(utcb)->mr[0] & (~0UL << 10); }

  void snd_base(unsigned long base)
  { l4_utcb_mr_u(utcb)->mr[0] = (base & (~0UL << 10)) | 8; }

  void clear()
  { l4_utcb_mr_u(utcb)->mr[0] = 0; }

  void snd_fpage(l4_fpage_t const &fp)
  {
    l4_utcb_mr_u(utcb)->mr[1] = fp.raw;
  }

  void snd_fpage(unsigned long addr, unsigned size, bool ro = false,
                 bool cache = true)
  {
    if (cache)
      l4_utcb_mr_u(utcb)->mr[0] |= L4_fpage_cached;
    else
      l4_utcb_mr_u(utcb)->mr[0] |= L4_fpage_uncached;

    l4_utcb_mr_u(utcb)->mr[1] = l4_fpage(addr, size,
	ro ? L4_FPAGE_RX : L4_FPAGE_RWX).raw;
  }

  bool failed() const
  { return l4_utcb_mr_u(utcb)->mr[0] == 0; }

  void do_grant()
  { l4_utcb_mr_u(utcb)->mr[0] |= 2; }
};

