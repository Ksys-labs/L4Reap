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

void mmsfb_drawstring_blend_rgb16(MMSFBSurfacePlanes *dst_planes, MMSFBFont *font,
								  MMSFBRegion &clipreg, string &text, int len, int x, int y, MMSFBColor &color) {
	// first time?
	static bool firsttime = true;
	if (firsttime) {
		printf("DISKO: Using blend text RGB16.\n");
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
	register unsigned short int SRCPIX = (((unsigned int)color.r >> 3) << 11) | (((unsigned int)color.g >> 2) << 5) | ((unsigned int)color.b >> 3);
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
					unsigned int r = DST >> 11;
					unsigned int g = DST & 0x07e0;
					unsigned int b = DST & 0x1f;

					// invert src alpha
				    r = SA * r;
				    g = SA * g;
				    b = (SA * b) >> 5;

				    // add src to dst
				    r += (A * color.r) >> 3;
					g += (A * color.g) << 3;
					b += (A * color.b) >> 8;
					d =   ((r & 0xffe000)   ? 0xf800 : ((r >> 8) << 11))
				    	| ((g & 0xfff80000) ? 0x07e0 : ((g >> 13) << 5))
				    	| ((b & 0xff00)     ? 0x1f 	 : (b >> 3));
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

