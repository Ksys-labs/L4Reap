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

#include "mmsgui/mmsguitools.h"
#include <stdlib.h>

bool getPixelFromSizeHint(int *retpix, string hint, int maxpixel, int secondaxis) {
    std::string::size_type pos;
    pos = hint.rfind("px");
    if (pos == std::string::npos) {
        /* its not px */
        pos = hint.rfind("%");
        if (pos == std::string::npos) {
            /* its not %, failed */
            pos = hint.rfind("$");
            if (pos == std::string::npos) {
                /* its not $, failed */
                return false;
            }
            else {
                /* use factor with secondaxis parameter */
                int pix = (int)(atof(hint.substr(0,pos).c_str()) * (double)secondaxis);
                if (pix > maxpixel) {
                    /* greater than maxpixel, failed */
                    return false;
                }
                if (retpix) *retpix = pix;
                return true;
            }
        }
        else {
            /* calculate percent to pixel */
            int pix = atoi(hint.substr(0,pos).c_str());
            if (pix > 100) {
                /* greater than 100%, failed */
                return false;
            }
            pix = (pix * maxpixel) / 100;

            if (hint.substr(pos+1,1)=="-") {
                /* subtract a value from calculated percent size */
                pix-=atoi(hint.substr(pos+2).c_str());
            }
            else
            if (hint.substr(pos+1,1)=="+") {
                /* add a value to calculated percent size */
                pix+=atoi(hint.substr(pos+2).c_str());
            }

            if (retpix) *retpix = pix;
            return true;
        }
    }
    else {
        /* use pixel values */
        int pix = atoi(hint.substr(0,pos).c_str());
        if (pix > maxpixel) {
            /* greater than maxpixel, failed */
            return false;
        }

        if (hint.substr(pos+2,1)=="-") {
            /* subtract a value from pixel */
            pix-=atoi(hint.substr(pos+3).c_str());
        }
        else
        if (hint.substr(pos+2,1)=="+") {
            /* add a value to pixel */
            pix+=atoi(hint.substr(pos+3).c_str());
        }

        if (retpix) *retpix = pix;
        return true;
    }
}



#ifdef  __HAVE_DIRECTFB__
bool loadImage(IDirectFBImageProvider **image, string path, string filename) {
//    IDirectFB              *mydfb = NULL;
    IDirectFBImageProvider *myimage = NULL;
    string                 imagefile;

    /* free old image */
    if (*image) {
        (*image)->Release(*image);
        *image = NULL;
    }

    /* build filename */
    imagefile = path;
    if (imagefile != "") imagefile+= "/";
    imagefile += filename;

    DEBUGMSG("MMSGUI", "using image file '%s'", imagefile.c_str());

    if (filename == "")
        return false;

    if (filename.substr(filename.size()-1) == "/")
        return false;

    /* open dfb access */
/*    if (!dfb) {
        if(DirectFBCreate(&mydfb)!= DFB_OK)
            return false;
    }
    else
        mydfb = dfb;*/

//    if (mydfb->CreateImageProvider(mydfb, imagefile.c_str(), &myimage) != DFB_OK) {
    if (!mmsfb->createImageProvider(&myimage, imagefile)) {
/*        if (!dfb)
            mydfb->Release(mydfb);*/
        if (myimage)
            myimage->Release(myimage);
        DEBUGMSG("MMSGUI", "cannot load image file '%s'", imagefile.c_str());
        return false;
    }

/*    if (!dfb)
        mydfb->Release(mydfb);*/

    *image = myimage;

    return true;
}
#endif

bool loadFont(MMSFBFont **font, string path, string filename, int width, int height) {
    MMSFBFont	*myfont = NULL;
    string    	fontfile;

    /* build filename */
    fontfile = path;
    if (fontfile != "") fontfile+= "/";
    fontfile += filename;

    DEBUGMSG("MMSGUI", "using font file '%s'", fontfile.c_str());

    if (filename == "")
        return false;

    if (filename.substr(filename.size()-1) == "/")
        return false;

    if (!mmsfb->createFont(&myfont, fontfile, width, height)) {
        if (myfont)
            delete myfont;
        DEBUGMSG("MMSGUI", "cannot load font file '%s'", fontfile.c_str());
        return false;
    }

    if (*font)
        delete *font;

    *font = myfont;

    return true;
}


unsigned int getFrameNum(unsigned int delay_time) {
	// a frame every 50ms
	unsigned int ret = delay_time / 50;
	if (ret < 2) ret = 2;
	return ret;
}

unsigned int getFrameDelay(unsigned int start_ts, unsigned int end_ts) {
	unsigned int diff = getMDiff(start_ts, end_ts);
	if (25 > diff)
		return 25-diff;
	else
		return 0;
}

