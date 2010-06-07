/*
 * (c) 2010 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */


#include <l4/re/video/colors>

#include <cstdlib>
#include <cstring>
#include <cstdio>

#include "splash.h"

void splash_display(L4Re::Video::Goos::Info *fb_info, l4_addr_t fbaddr)
{

  if (fb_info->width < SPLASHNAME.width)
    return;
  if (fb_info->height < SPLASHNAME.height)
    return;

  int size = SPLASHNAME.width * SPLASHNAME.height;
  unsigned char *buf = (unsigned char *)malloc(size * SPLASHNAME.bytes_per_pixel);
  if (!buf)
    return;

  SPLASHNAME_RUN_LENGTH_DECODE(buf, SPLASHNAME.rle_pixel_data, size,
                               SPLASHNAME.bytes_per_pixel);

  int buf_idx = 0;
  int off_x = (fb_info->width / 2) - (SPLASHNAME.width / 2);
  int off_y = (fb_info->height / 2) - (SPLASHNAME.height / 2);
  for (unsigned y = 0; y < SPLASHNAME.height; ++y)
    for (unsigned x = 0; x < SPLASHNAME.width; ++x)
      {
        unsigned valr = buf[buf_idx];
        unsigned valg = buf[buf_idx + 1];
        unsigned valb = buf[buf_idx + 2];
        unsigned v;

        v  = (((valr <<  0) >> (8  - fb_info->pixel_info.r().size())) & ((1 << fb_info->pixel_info.r().size()) - 1)) << fb_info->pixel_info.r().shift();
        v |= (((valg <<  8) >> (16 - fb_info->pixel_info.g().size())) & ((1 << fb_info->pixel_info.g().size()) - 1)) << fb_info->pixel_info.g().shift();
        v |= (((valb << 16) >> (24 - fb_info->pixel_info.b().size())) & ((1 << fb_info->pixel_info.b().size()) - 1)) << fb_info->pixel_info.b().shift();

        char *_fb = (char *)fbaddr;
        unsigned fb_off = (y + off_y) * fb_info->width + off_x + x;
        fb_off *= fb_info->pixel_info.bytes_per_pixel();
        char *a = &_fb[fb_off];

        switch (fb_info->pixel_info.bits_per_pixel())
          {
          case 8: *(unsigned char  *)a= v; break;
          case 15:
          case 16: *(unsigned short *)a = v; break;
          case 32: *(unsigned int   *)a = v; break;
          };

        buf_idx += SPLASHNAME.bytes_per_pixel;
      }


  free(buf);
}
