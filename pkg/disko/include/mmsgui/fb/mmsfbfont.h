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

#ifndef MMSFBFONT_H_
#define MMSFBFONT_H_

#include "mmsgui/fb/mmsfbbase.h"

//! descibes a loaded glyph
typedef struct {
	//! pointer to data
	unsigned char	*buffer;
	//! pitch in byte of one row in the buffer
	int 			pitch;
	//! x-offset
	int				left;
	//! y-offset
	int				top;
	//! width in pixel of the glyph bitmap
	int 			width;
	//! height in pixel of the glyph bitmap
	int 			height;
	//! width in pixel of the whole character
	int				advanceX;
#ifdef  __HAVE_OPENGL__
	//! OpenGL texture for this glyph
	unsigned int	texture;
#endif
} MMSFBFont_Glyph;

//! Font rendering class.
/*!
\author Jens Schneider
*/
class MMSFBFont {
    private:
        //! true if initialized
        bool 		initialized;

        //! to make it thread-safe
        MMSMutex  	Lock;

    	//! pointer to the directfb font
    	void 		*dfbfont;

    	//! static pointer to the freetype library
        static void *ft_library;

        //! pointer to the loaded freetype face
        void 		*ft_face;

        //! font file
        string 	filename;

        //! input width
    	int		w;

    	//! input height
    	int 	h;

    	//! ascender
    	int 	ascender;

    	//! descender
    	int		descender;

    	//! real height of one line
    	int 	height;

    	//! maps a character id to a already loaded glyph (see glyphpool)
    	std::map<unsigned int, MMSFBFont_Glyph> charmap;

    	//! glyph pool
    	unsigned char *glyphpool;

    	//! size of glyph pool
    	unsigned int glyphpool_size;

    	//! pointer to next free memory
    	unsigned char *glyphpool_ptr;

        void lock();
        void unlock();

    public:
        MMSFBFont(string filename, int w, int h);
        virtual ~MMSFBFont();

        bool isInitialized();

        bool getStringWidth(string text, int len, int *width);
        bool getHeight(int *height);

        bool getAscender(int *ascender);
        bool getDescender(int *descender);

        bool getGlyph(unsigned int character, MMSFBFont_Glyph *glyph);

	friend class MMSFBSurface;
};

#define MMSFBFONT_GET_UNICODE_CHAR(text, len) \
	for (int cnt = 0; cnt < len; cnt++) { \
		unsigned char c = text[cnt]; \
		unsigned int character; \
		if(c >= 0xf0) /* 11110000 -> 4 bytes */ { \
			if(len < (cnt + 3)) { DEBUGMSG("MMSFBFONT", "invalid unicode string"); break; } \
			character = (unsigned int)((c & 0x07 /* 00000111 */) << 18); \
			character |= ((text[++cnt] & 0x3f /* 00111111 */) << 12); \
			character |= ((text[++cnt] & 0x3f) << 6); \
			character |= (text[++cnt] & 0x3f); \
		} else if(c >= 0xe0)  /* 11100000 -> 3 bytes */ { \
			if(len < (cnt + 2)) { DEBUGMSG("MMSFBFONT", "invalid unicode string"); break; } \
			character = (unsigned int)((c & 0x0f /* 00001111 */) << 12); \
			character |= ((text[++cnt] & 0x3f) << 6); \
			character |= (text[++cnt] & 0x3f); \
		} else if(c >= 0xc0)  /* 11000000 -> 2 bytes */ { \
			if(len < (cnt + 1)) { DEBUGMSG("MMSFBFONT", "invalid unicode string"); break; } \
			character = (unsigned int)(((c & 0x1f /* 00011111 */) << 6) | (text[++cnt] & 0x3f)); \
		} else  /* 1 byte */ { \
			character = (unsigned int)c; \
		}

#endif /*MMSFBFONT_H_*/
