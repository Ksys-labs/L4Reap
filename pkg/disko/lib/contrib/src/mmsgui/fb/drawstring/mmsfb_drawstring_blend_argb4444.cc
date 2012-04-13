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

void mmsfb_drawstring_blend_argb4444(MMSFBSurfacePlanes *dst_planes, MMSFBFont *font,
									 MMSFBRegion &clipreg, string &text, int len, int x, int y, MMSFBColor &color) {
	// first time?
	static bool firsttime = true;
	if (firsttime) {
		printf("DISKO: Using blend text ARGB4444.\n");
		firsttime = false;
	}

	// get the first destination ptr/pitch
	void *dst_ptr = dst_planes->ptr;
	int dst_pitch = dst_planes->pitch;

	// lock font and destination surface
	MMSFBSURFACE_BLIT_TEXT_INIT(1);

	// for all characters
	unsigned int OLDDST = 0;
	unsigned int OLDSRC = 0;
	register unsigned short int d = 0;
	register unsigned short int SRCPIX = 0xf000 | (((unsigned int)color.r >> 4) << 8) | (((unsigned int)color.g >> 4) << 4) | ((unsigned int)color.b >> 4);
	MMSFBFONT_GET_UNICODE_CHAR(text, len) {
		// load the glyph
		MMSFBSURFACE_BLIT_TEXT_LOAD_GLYPH(font, character);

		// start rendering of glyph to destination
		MMSFBSURFACE_BLIT_TEXT_START_RENDER(unsigned short int);

		// through the pixels
		while (src < src_end) {
			while (src < line_end) {
				// load pixel from memory
				register unsigned int SRC = *src;

				// is the source alpha channel 0x00 or 0xff?
				register unsigned int A = SRC;
				if (A == 0xff) {
					// source pixel is not transparent, copy it directly to the destination
					*dst = SRCPIX;
				}
				else
				if (A) {
					// source alpha is > 0x00 and < 0xff
					register unsigned short int DST = *dst;

					if ((DST==OLDDST)&&(SRC==OLDSRC)) {
						// same pixel, use the previous value
						*dst = d;
					    dst++;
					    src++;
						continue;
					}
					OLDDST = DST;
					OLDSRC = SRC;

					register unsigned int SA= 0x100 - A;
					unsigned int a = DST >> 12;
					unsigned int r = DST & 0x0f00;
					unsigned int g = DST & 0x00f0;
					unsigned int b = DST & 0x000f;

					// invert src alpha
				    a = (SA * a) >> 4;
				    r = (SA * r) >> 12;
				    g = (SA * g) >> 8;
				    b = (SA * b) >> 4;

				    // add src to dst
				    a += A;
				    A++;
				    r += (A * color.r) >> 8;
				    g += (A * color.g) >> 8;
				    b += (A * color.b) >> 8;
				    d =   ((a >> 8) ? 0xf000 : ((a >> 4) << 12))
						| ((r >> 8) ? 0x0f00 : ((r >> 4) << 8))
						| ((g >> 8) ? 0xf0   : ((g >> 4) << 4))
				    	| ((b >> 8) ? 0x0f   :  (b >> 4));
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

