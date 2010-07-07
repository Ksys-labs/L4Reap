/**
 * \file
 * \note The C interface of L4Re::Video does _NOT_ reflect the full C++
 *       interface on purpose. Use the C++ where possible.
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
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
#pragma once

#include <l4/sys/types.h>
#include <l4/re/c/dataspace.h>
#include <l4/re/c/video/colors.h>

/**
 * \brief Flags of information on a view.
 * \ingroup api_l4re_c_video
 */
enum l4re_video_view_info_flags_t
{
  F_l4re_video_view_none               = 0x00, ///< everything for this view is static (the VESA-FB case)
  F_l4re_video_view_set_buffer         = 0x01, ///< buffer object for this view can be changed
  F_l4re_video_view_set_buffer_offset  = 0x02, ///< buffer offset can be set
  F_l4re_video_view_set_bytes_per_line = 0x04, ///< bytes per line can be set
  F_l4re_video_view_set_pixel          = 0x08, ///< pixel type can be set
  F_l4re_video_view_set_position       = 0x10, ///< position on screen can be set
  F_l4re_video_view_dyn_allocated      = 0x20, ///< View is dynamically allocated

  F_l4re_video_view_fully_dynamic      =   F_l4re_video_view_set_buffer
                                         | F_l4re_video_view_set_buffer_offset
                                         | F_l4re_video_view_set_bytes_per_line
                                         | F_l4re_video_view_set_pixel
                                         | F_l4re_video_view_set_position
                                         | F_l4re_video_view_dyn_allocated,
};

/**
 * \brief View information structure
 * \ingroup api_l4re_c_video
 */
typedef struct l4re_video_view_info_t
{
  unsigned                flags;                     ///< Flags
  unsigned                view_index;                ///< Number of view in the goos
  unsigned long           xpos, ypos, width, height; ///< Position in goos and size of view
  unsigned long           buffer_offset;             ///< Memory offset in goos buffer
  unsigned long           bytes_per_line;            ///< Size of line in view
  l4re_video_pixel_info_t pixel_info;                ///< Pixel info
  unsigned                buffer_index;              ///< Number of buffer of goos
} l4re_video_view_info_t;
