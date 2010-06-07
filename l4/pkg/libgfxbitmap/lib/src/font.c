/**
 * \file
 * \brief     Font functions
 *
 * \author    Christian Helmuth <ch12@os.inf.tu-dresden.de>
 *            Frank Mehnert <fm3@os.inf.tu-dresden.de>
 *            Adam Lackorzynski <adam@os.inf.tu-dresden.de> */

/*
 * (c) 2001-2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

#include <l4/libgfxbitmap/font.h>
#include <l4/libgfxbitmap/bitmap.h>
#include <l4/sys/l4int.h>
#include <string.h>
#include <stdio.h>

extern const char _binary_fontfile_psf_start[];

static l4_uint8_t  FONT_XRES, FONT_YRES;
static l4_uint32_t FONT_CHRS;



unsigned
gfxbitmap_font_width(gfxbitmap_font_t font)
{
  (void)font;
  return FONT_XRES;
}

unsigned
gfxbitmap_font_height(gfxbitmap_font_t font)
{
  (void)font;
  return FONT_YRES;
}

void *
gfxbitmap_font_data(gfxbitmap_font_t font, unsigned c)
{
  (void)font;
  return (void *)&_binary_fontfile_psf_start[FONT_YRES * c + 4];
}

void
gfxbitmap_font_text(void *fb, l4re_video_view_info_t *vi,
                    gfxbitmap_font_t font, const char *text, unsigned len,
                    unsigned x, unsigned y,
                    gfxbitmap_color_pix_t fg, gfxbitmap_color_pix_t bg)
{
  unsigned i, j;
  struct gfxbitmap_offset offset = {0,0,0};

  if (len == GFXBITMAP_USE_STRLEN)
    len = strlen(text);

  for (i = 0; i < len; i++, text++)
    {
      /* optimization: collect spaces */
      for (j = 0; (i < len) && (*text == ' '); i++, j++, text++)
        ;

      if (j > 0)
        {
          gfxbitmap_fill(fb, vi, x, y, j*FONT_XRES, FONT_YRES, bg);
          x += j*FONT_XRES;
          i--; text--;
          continue;
        }

      gfxbitmap_bmap(fb, vi, x, y, FONT_XRES, FONT_YRES,
                     gfxbitmap_font_data(font, *(unsigned char *)text), fg, bg,
                     &offset, pSLIM_BMAP_START_MSB);
      x += FONT_XRES;
    }
}


void
gfxbitmap_font_text_scale(void *fb, l4re_video_view_info_t *vi,
                          gfxbitmap_font_t font, const char *text, unsigned len,
                          unsigned x, unsigned y,
                          gfxbitmap_color_pix_t fg, gfxbitmap_color_pix_t bg,
                          int scale_x, int scale_y)
{
  int pix_x, pix_y;
  unsigned rect_x = x;
  unsigned rect_y = y;
  unsigned rect_w = gfxbitmap_font_width(font) * scale_x;
  unsigned i;

  pix_x = scale_x;
  if (scale_x >= 5)
    pix_x = scale_x * 14/15;
  pix_y = scale_y;
  if (scale_y >= 5)
    pix_y = scale_y * 14/15;

  if (len == GFXBITMAP_USE_STRLEN)
    len = strlen(text);

  for (i=0; i < len; i++, text++)
    {
      unsigned lrect_x = rect_x;
      unsigned lrect_y = rect_y;
      unsigned lrect_w = pix_x;
      unsigned lrect_h = pix_y;
      const char *bmap = gfxbitmap_font_data(font, *text);
      int j;

      for (j=0; j<FONT_YRES; j++)
        {
          unsigned char mask = 0x80;
          int k;

          for (k=0; k<FONT_XRES; k++)
            {
              unsigned color = (*bmap & mask) ? fg : bg;
              gfxbitmap_fill(fb, vi, lrect_x, lrect_y, lrect_w, lrect_h, color);
              lrect_x += scale_x;
              bmap += (mask &  1);
              mask  = (mask >> 1) | (mask << 7);
            }
          lrect_x -= rect_w;
          lrect_y += scale_y;
        }
      rect_x += rect_w;
    }
}




/** Init lib */
int
gfxbitmap_font_init(void)
{
  /* check magic number of .psf */
  if (_binary_fontfile_psf_start[0] != 0x36 || _binary_fontfile_psf_start[1] != 0x04)
    return 1; // psf magic number failed

  FONT_XRES = 8;
  FONT_YRES = _binary_fontfile_psf_start[3];

  /* check file mode */
  switch (_binary_fontfile_psf_start[2])
    {
    case 0:
    case 2:
      FONT_CHRS = 256;
      break;
    case 1:
    case 3:
      FONT_CHRS = 512;
      break;
    default:
      return 2; // "bad psf font file magic %02x!", _binary_fontfile_psf_start[2]
    }

  printf("Font: Character size is %dx%d, font has %d characters\n",
         FONT_XRES, FONT_YRES, FONT_CHRS);

  return 0;
}

