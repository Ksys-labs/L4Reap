#include <l4/re/c/namespace.h>
#include <l4/re/c/dataspace.h>
#include <l4/re/c/rm.h>
#include <l4/re/c/util/cap_alloc.h>
#include <l4/re/c/util/video/goos_fb.h>
#include <l4/libpng/l4png_wrap.h>
#include <l4/util/util.h>

#include <stdio.h>

int main(int argc, char **argv)
{
  l4_addr_t bildmem;
  void *vidmem;
  l4re_util_video_goos_fb_t gfb;
  l4re_video_view_info_t fbi;

  if (argc < 2)
    {
      printf("Need to give PNG picture to display\n");
      return 1;
    }

  if (l4re_util_video_goos_fb_setup_name(&gfb, "fb"))
    return 45;

  if (!(vidmem = l4re_util_video_goos_fb_attach_buffer(&gfb)))
    return 46;

  printf("size: %ld\n", l4re_ds_size(l4re_util_video_goos_fb_buffer(&gfb)));
  printf("Vidmem attached to %p\n", vidmem);

  if (l4re_util_video_goos_fb_view_info(&gfb, &fbi))
    {
      printf("l4re_fb_open failed\n");
      return 1;
    }

  l4re_ds_t bild = l4re_util_cap_alloc();
  if (l4_is_invalid_cap(bild))
    return 1;
  if (l4re_ns_query_srv(l4re_get_env_cap("rom"), argv[1], bild))
    return -1;

  printf("Picture size: %ld\n", l4re_ds_size(bild));

  bildmem = 0;
  if (l4re_rm_attach((void **)&bildmem, l4re_ds_size(bild),
	             L4RE_RM_SEARCH_ADDR, bild, 0, L4_PAGESHIFT))
    return 1;


  int png_w, png_h;
  png_get_size_mem((void *)bildmem, l4re_ds_size(bild), &png_w, &png_h);

  printf("PNG: %dx%d\n", png_w, png_h);

  if (png_w < 0 || png_h < 0)
    {
      printf("Error with picture. Is it one?\n");
      return 1;
    }

  if ((unsigned)png_w > fbi.width || (unsigned)png_h > fbi.height)
    {
      printf("Picture too large, cannot display\n");
      return 1;
    }

  if (fbi.pixel_info.bytes_per_pixel == 2)
    png_convert_RGB16bit_mem((void *)bildmem, (void *)vidmem,
	                     l4re_ds_size(bild),
                             png_w*png_h*fbi.pixel_info.bytes_per_pixel,
                             fbi.width);
  else
    png_convert_ARGB_mem((void *)bildmem, (void *)vidmem, l4re_ds_size(bild),
                         png_w*png_h*fbi.pixel_info.bytes_per_pixel);

  l4re_util_video_goos_fb_refresh(&gfb, 0, 0, png_w, png_h);

  l4_sleep_forever();

  return 0;
}
