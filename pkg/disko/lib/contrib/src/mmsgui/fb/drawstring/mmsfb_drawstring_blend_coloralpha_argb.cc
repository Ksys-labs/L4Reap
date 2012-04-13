/***************************************************************************
 *   Copyright (C) 2005-2007 Stefan Schwarzer, Jens Schneider,             *
 *                           Matthias Hardt, Guido Madaus                  *
 *                                                                         *
 *   Copyright (C) 2007-2008 BerLinux Solutions GbR                        *
 *                           Stefan Schwarzer & Guido Madaus               *
 *                                                                         *
 *   Copyright (C) 2009-2011 BerLinux Solutions GmbH                       *
 *                                                                         *
 *   Authors:                                                              *
 *      Stefan Schwarzer   <stefan.schwarzer@diskohq.org>,                 *
 *      Matthias Hardt     <matthias.hardt@diskohq.org>,                   *
 *      Jens Schneider     <pupeider@gmx.de>,                              *
 *      Guido Madaus       <guido.madaus@diskohq.org>,                     *
 *      Patrick Helterhoff <patrick.helterhoff@diskohq.org>,               *
 *      René Bählkow       <rene.baehlkow@diskohq.org>                     *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License version 2.1 as published by the Free Software Foundation.     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 **************************************************************************/

#include "mmsgui/fb/mmsfbconv.h"
#include "mmstools/mmstools.h"

void mmsfb_drawstring_blend_coloralpha_argb(MMSFBSurfacePlanes *dst_planes, MMSFBFont *font,
											MMSFBRegion &clipreg, string &text, int len, int x, int y, MMSFBColor &color) {
	// check for full alpha value
	if (color.a == 0xff) {
		// max alpha is specified, so i can ignore it and use faster routine
		mmsfb_drawstring_blend_argb(dst_planes, font, clipreg, text, len, x, y, color);
		return;
	}

	// get the first destination ptr/pitch
	void *dst_ptr = dst_planes->ptr;
	int dst_pitch = dst_planes->pitch;

	// first time?
	static bool firsttime = true;
	if (firsttime) {
		printf("DISKO: Using blend text coloralpha ARGB.\n");
		firsttime = false;
	}

	// something to do?
	if (!color.a)
		// source should blitted full transparent, so leave destination as is
		return;

	// lock font and destination surface
	MMSFBSURFACE_BLIT_TEXT_INIT(2);

	// for all characters
	unsigned int OLDDST = 0;
	unsigned int OLDSRC = 0;
	register unsigned int d = 0;
	register unsigned int ALPHA = color.a;
	ALPHA++;
	MMSFBFONT_GET_UNICODE_CHAR(text, len) {
		// load the glyph
		MMSFBSURFACE_BLIT_TEXT_LOAD_GLYPH(font, character);

		// start rendering of glyph to destination
		MMSFBSURFACE_BLIT_TEXT_START_RENDER(unsigned int);

		// through the pixels
		while (src < src_end) {
			while (src < line_end) {
				// load pixel from memory
				register unsigned int SRC = *src;

				// is the source alpha channel 0x00 or 0xff?
				register unsigned int A = SRC;
				if (A) {
					// source alpha is > 0x00 and < 0xff
					register unsigned int DST = *dst;

					if ((DST==OLDDST)&&(SRC==OLDSRC)) {
						// same pixel, use the previous value
						*dst = d;
					    dst++;
					    src++;
						continue;
					}
					OLDDST = DST;
					OLDSRC = SRC;

				    A = (ALPHA * A) >> 8;
					register unsigned int SA= 0x100 - A;
					unsigned int a = DST >> 24;
					unsigned int r = (DST << 8) >> 24;
					unsigned int g = (DST << 16) >> 24;
					unsigned int b = DST & 0xff;

					// invert src alpha
				    a = (SA * a) >> 8;
				    r = (SA * r) >> 8;
				    g = (SA * g) >> 8;
				    b = (SA * b) >> 8;

				    // add src to dst
				    a += A;
				    A++;
				    r += (A * color.r) >> 8;
				    g += (A * color.g) >> 8;
				    b += (A * color.b) >> 8;
				    d =   ((a >> 8) ? 0xff000000 : (a << 24))
						| ((r >> 8) ? 0xff0000   : (r << 16))
						| ((g >> 8) ? 0xff00     : (g << 8))
				    	| ((b >> 8) ? 0xff 		 :  b);
					*dst = d;
				}

				src++;
				dst++;
			}
			line_end+= src_pitch_pix;
			src     += src_pitch_pix_diff;
			dst     += dst_pitch_pix_diff;
		}

		// prepare for next loop
		MMSFBSURFACE_BLIT_TEXT_END_RENDER;
	}}
}

