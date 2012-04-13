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

#include "mmsgui/mmsborder.h"
#include "mmsgui/mmsguitools.h"
#include <math.h>

void drawBorder(unsigned int borderThickness, bool borderRCorners, MMSFBSurface *borderimages[],
                MMSFBRectangle bordergeom[], bool *bordergeomset, MMSFBSurface *surface,
                unsigned int x, unsigned int y, unsigned int width, unsigned int height, MMSFBColor color,
                MMSImageManager *im, unsigned char brightness, unsigned char opacity) {
    int bic = 8;

    /* draw border? */
    if (!borderThickness)
        return;

    /* lock */
    surface->lock();

    /* set the blitting flags */
    unsigned char alpha = 255;
    if (color.a) alpha = color.a;
    surface->setBlittingFlagsByBrightnessAlphaAndOpacity(brightness, alpha, opacity);

    /* draw images around the widget */
    /* FIRST: corners */
    /* image #1 (top-left) */
    if (borderimages[0]) {
        if (!*bordergeomset) {
            int ww, hh;
            borderimages[0]->getSize(&ww, &hh);
            bordergeom[0].x = x;
            bordergeom[0].y = y;
            bordergeom[0].w = ww;
            bordergeom[0].h = hh;
        }
        surface->stretchBlit(borderimages[0], NULL, &bordergeom[0]);
        bic--;
    }
    else {
        if (borderRCorners) {
            bordergeom[0].x = x;
            bordergeom[0].y = y;
            bordergeom[0].w = 2*borderThickness;
            bordergeom[0].h = 2*borderThickness;
        }
        else {
            bordergeom[0].x = x;
            bordergeom[0].y = y;
            bordergeom[0].w = borderThickness;
            bordergeom[0].h = borderThickness;
        }
    }
    /* image #3 (top-right) */
    if (borderimages[2]) {
        if (!*bordergeomset) {
            int ww, hh;
            borderimages[2]->getSize(&ww, &hh);
            bordergeom[2].x = x + width - ww;
            bordergeom[2].y = y;
            bordergeom[2].w = ww;
            bordergeom[2].h = hh;
        }
        surface->stretchBlit(borderimages[2], NULL, &bordergeom[2]);
        bic--;
    }
    else {
        if (borderRCorners) {
            bordergeom[2].x = x + width - 2*borderThickness;
            bordergeom[2].y = y;
            bordergeom[2].w = 2*borderThickness;
            bordergeom[2].h = 2*borderThickness;
        }
        else {
            bordergeom[2].x = x + width - borderThickness;
            bordergeom[2].y = y;
            bordergeom[2].w = borderThickness;
            bordergeom[2].h = borderThickness;
        }
    }
    /* image #5 (bottom-right) */
    if (borderimages[4]) {
        if (!*bordergeomset) {
            int ww, hh;
            borderimages[4]->getSize(&ww, &hh);
            bordergeom[4].x = x + width - ww;
            bordergeom[4].y = y + height - hh;
            bordergeom[4].w = ww;
            bordergeom[4].h = hh;
        }
        surface->stretchBlit(borderimages[4], NULL, &bordergeom[4]);
        bic--;
    }
    else {
        if (borderRCorners) {
            bordergeom[4].x = x + width - 2*borderThickness;
            bordergeom[4].y = y + height - 2*borderThickness;
            bordergeom[4].w = 2*borderThickness;
            bordergeom[4].h = 2*borderThickness;
        } else {
            bordergeom[4].x = x + width - borderThickness;
            bordergeom[4].y = y + height - borderThickness;
            bordergeom[4].w = borderThickness;
            bordergeom[4].h = borderThickness;
        }
    }
    /* image #7 (bottom-left) */
    if (borderimages[6]) {
        if (!*bordergeomset) {
            int ww, hh;
            borderimages[6]->getSize(&ww, &hh);
            bordergeom[6].x = x;
            bordergeom[6].y = y + height - hh;
            bordergeom[6].w = ww;
            bordergeom[6].h = hh;
        }
        surface->stretchBlit(borderimages[6], NULL, &bordergeom[6]);
        bic--;
    }
    else {
        if (borderRCorners) {
            bordergeom[6].x = x;
            bordergeom[6].y = y + height - 2*borderThickness;
            bordergeom[6].w = 2*borderThickness;
            bordergeom[6].h = 2*borderThickness;
        } else {
            bordergeom[6].x = x;
            bordergeom[6].y = y + height - borderThickness;
            bordergeom[6].w = borderThickness;
            bordergeom[6].h = borderThickness;
        }
    }

    /* SECOND: horizontal/vertical lines */
    /* image #2 (top) */
    if (borderimages[1]) {
        if (!*bordergeomset) {
            int ww, hh;
            borderimages[1]->getSize(&ww, &hh);
            bordergeom[1].x = bordergeom[0].x + bordergeom[0].w;
            bordergeom[1].y = y;
            bordergeom[1].w = bordergeom[2].x - bordergeom[1].x;
            bordergeom[1].h = hh;
        }
        surface->stretchBlit(borderimages[1], NULL, &bordergeom[1]);
        bic--;
    }
    else {
        bordergeom[1].x = bordergeom[0].x + bordergeom[0].w;
        bordergeom[1].y = y;
        bordergeom[1].w = bordergeom[2].x - bordergeom[1].x;
        bordergeom[1].h = borderThickness;
    }
    /* image #4 (right) */
    if (borderimages[3]) {
        if (!*bordergeomset) {
            int ww, hh;
            borderimages[3]->getSize(&ww, &hh);
            bordergeom[3].x = x + width - ww;
            bordergeom[3].y = bordergeom[2].y + bordergeom[2].h;
            bordergeom[3].w = ww;
            bordergeom[3].h = bordergeom[4].y - bordergeom[3].y;
        }
        surface->stretchBlit(borderimages[3], NULL, &bordergeom[3]);
        bic--;
    }
    else {
        bordergeom[3].x = x + width - borderThickness;
        bordergeom[3].y = bordergeom[2].y + bordergeom[2].h;
        bordergeom[3].w = borderThickness;
        bordergeom[3].h = bordergeom[4].y - bordergeom[3].y;
    }
    /* image #6 (bottom) */
    if (borderimages[5]) {
        if (!*bordergeomset) {
            int ww, hh;
            borderimages[5]->getSize(&ww, &hh);
            bordergeom[5].x = bordergeom[6].x + bordergeom[6].w;
            bordergeom[5].y = y + height - hh;
            bordergeom[5].w = bordergeom[4].x - bordergeom[5].x;
            bordergeom[5].h = hh;
        }
        surface->stretchBlit(borderimages[5], NULL, &bordergeom[5]);
        bic--;
    }
    else {
        bordergeom[5].x = bordergeom[6].x + bordergeom[6].w;
        bordergeom[5].y = y + height - borderThickness;
        bordergeom[5].w = bordergeom[4].x - bordergeom[5].x;
        bordergeom[5].h = borderThickness;
    }
    /* image #8 (left) */
    if (borderimages[7]) {
        if (!*bordergeomset) {
            int ww, hh;
            borderimages[7]->getSize(&ww, &hh);
            bordergeom[7].x = x;
            bordergeom[7].y = bordergeom[0].y + bordergeom[0].h;
            bordergeom[7].w = ww;
            bordergeom[7].h = bordergeom[6].y - bordergeom[7].y;
        }
        surface->stretchBlit(borderimages[7], NULL, &bordergeom[7]);
        bic--;
    }
    else {
        bordergeom[7].x = x;
        bordergeom[7].y = bordergeom[0].y + bordergeom[0].h;
        bordergeom[7].w = borderThickness;
        bordergeom[7].h = bordergeom[6].y - bordergeom[7].y;
    }

    /* border geom is set */
    *bordergeomset = true;

    /* if bic not 0 then at least one borderimage is not set  */
    if (bic) {
        /* draw lines around the widget */
        /* here we create surfaces and draw lines only once */

        /* get the pixelformat of windows surface */
/*        DFBSurfacePixelFormat pixelformat;
        surface->GetPixelFormat(surface, &pixelformat);
pixelformat=DSPF_ALUT44;
*/
        /* FIRST: corners */
        /* (top-left) */
        if (!borderimages[0]) {
            /* create newImage surface */
            if ((borderimages[0] = im->newImage("", bordergeom[0].w, bordergeom[0].h))) {
                borderimages[0]->clear();
                if (color.a) {
                    borderimages[0]->setColor(color.r, color.g, color.b, 255);
                    if (borderRCorners) {
                        /* draw a round corner with circles */
                        if (bordergeom[0].w > 1) {
                            int j1 = bordergeom[0].w - 1;
                            int j2 = j1-1;
                            borderimages[0]->drawCircle(j1, j1, j1, 6, 7);
                            for (unsigned int i = 1; i < borderThickness; i++) {
                                int j3 = j1-i;
                                borderimages[0]->drawCircle(j2, j1, j3, 6, 7);
                                borderimages[0]->drawCircle(j1, j2, j3, 6, 7);
                                borderimages[0]->drawCircle(j1, j1, j3, 6, 7);
                            }
                        }
                    }
                    else {
                        // the corner is like an rectangle
                        borderimages[0]->fillRectangle(0, 0, bordergeom[0].w, bordergeom[0].h);
                    }
                }

                /* set the blitting flags */
                surface->setBlittingFlagsByBrightnessAlphaAndOpacity(brightness, alpha, opacity);

                /* blit the first time */
                surface->stretchBlit(borderimages[0], NULL, &bordergeom[0]);
            }
        }
        /* (top-right) */
        if (!borderimages[2]) {
            /* create newImage surface */
            if ((borderimages[2] = im->newImage("", bordergeom[2].w, bordergeom[2].h))) {
                borderimages[2]->clear();
                if (color.a) {
                    borderimages[2]->setColor(color.r, color.g, color.b, 255);
                    if (borderRCorners) {
                        /* draw a round corner with circles */
                        if (bordergeom[2].w > 1) {
                            int j1 = bordergeom[2].w - 1;
                            int j2 = j1-1;
                            borderimages[2]->drawCircle(0, j1, j1, 0, 1);
                            for (unsigned int i = 1; i < borderThickness; i++) {
                                int j3 = j1-i;
                                borderimages[2]->drawCircle(1, j1, j3, 0, 1);
                                borderimages[2]->drawCircle(0, j2, j3, 0, 1);
                                borderimages[2]->drawCircle(0, j1, j3, 0, 1);
                            }
                        }
                    }
                    else {
                        // the corner is like an rectangle
                        borderimages[2]->fillRectangle(0, 0, bordergeom[2].w, bordergeom[2].h);
                    }
                }

                /* set the blitting flags */
                surface->setBlittingFlagsByBrightnessAlphaAndOpacity(brightness, alpha, opacity);

                /* blit the first time */
                surface->stretchBlit(borderimages[2], NULL, &bordergeom[2]);
            }
        }
        /* (bottom-right) */
        if (!borderimages[4]) {
            /* create newImage surface */
            if ((borderimages[4] = im->newImage("", bordergeom[4].w, bordergeom[4].h))) {
                borderimages[4]->clear();
                if (color.a) {
                    borderimages[4]->setColor(color.r, color.g, color.b, 255);
                    if (borderRCorners) {
                        /* draw a round corner with circles */
                        if (bordergeom[4].w > 1) {
                            int j1 = bordergeom[4].w - 1;
                            borderimages[4]->drawCircle(0, 0, j1, 2, 3);
                            for (unsigned int i = 1; i < borderThickness; i++) {
                                int j3 = j1-i;
                                borderimages[4]->drawCircle(1, 0, j3, 2, 3);
                                borderimages[4]->drawCircle(0, 1, j3, 2, 3);
                                borderimages[4]->drawCircle(0, 0, j3, 2, 3);
                            }
                        }
                    }
                    else {
                        // the corner is like an rectangle
                        borderimages[4]->fillRectangle(0, 0, bordergeom[4].w, bordergeom[4].h);
                    }
                }

                /* set the blitting flags */
                surface->setBlittingFlagsByBrightnessAlphaAndOpacity(brightness, alpha, opacity);

                /* blit the first time */
                surface->stretchBlit(borderimages[4], NULL, &bordergeom[4]);
            }
        }
        /* (bottom-left) */
        if (!borderimages[6]) {
            /* create newImage surface */
            if ((borderimages[6] = im->newImage("", bordergeom[6].w, bordergeom[6].h))) {
                borderimages[6]->clear();
                if (color.a) {
                    borderimages[6]->setColor(color.r, color.g, color.b, 255);
                    if (borderRCorners) {
                        /* draw a round corner with circles */
                        if (bordergeom[6].w > 1) {
                            int j1 = bordergeom[6].w - 1;
                            int j2 = j1-1;
                            borderimages[6]->drawCircle(j1, 0, j1, 4, 5);
                            for (unsigned int i = 1; i < borderThickness; i++) {
                                int j3 = j1-i;
                                borderimages[6]->drawCircle(j2, 0, j3, 4, 5);
                                borderimages[6]->drawCircle(j1, 1, j3, 4, 5);
                                borderimages[6]->drawCircle(j1, 0, j3, 4, 5);
                            }
                        }
                    }
                    else {
                        // the corner is like an rectangle
                        borderimages[6]->fillRectangle(0, 0, bordergeom[6].w, bordergeom[6].h);
                    }
                }

                /* set the blitting flags */
                surface->setBlittingFlagsByBrightnessAlphaAndOpacity(brightness, alpha, opacity);

                /* blit the first time */
                surface->stretchBlit(borderimages[6], NULL, &bordergeom[6]);
            }
        }

        /* SECOND: horizontal/vertical lines */
        /* (top) */
        if (!borderimages[1]) {
            /* create newImage surface */
            if ((borderimages[1] = im->newImage("", bordergeom[1].w, bordergeom[1].h))) {
                borderimages[1]->clear();
                if (color.a) {
                    borderimages[1]->setColor(color.r, color.g, color.b, 255);
                    borderimages[1]->fillRectangle(0, 0, bordergeom[1].w, bordergeom[1].h);
                }

                /* set the blitting flags */
                surface->setBlittingFlagsByBrightnessAlphaAndOpacity(brightness, alpha, opacity);

                /* blit the first time */
                surface->stretchBlit(borderimages[1], NULL, &bordergeom[1]);
            }
        }
        /* (right) */
        if (!borderimages[3]) {
            /* create newImage surface */
            if ((borderimages[3] = im->newImage("", bordergeom[3].w, bordergeom[3].h))) {
                borderimages[3]->clear();
                if (color.a) {
                    borderimages[3]->setColor(color.r, color.g, color.b, 255);
                    borderimages[3]->fillRectangle(0, 0, bordergeom[3].w, bordergeom[3].h);
                }

                /* set the blitting flags */
                surface->setBlittingFlagsByBrightnessAlphaAndOpacity(brightness, alpha, opacity);

                /* blit the first time */
                surface->stretchBlit(borderimages[3], NULL, &bordergeom[3]);
            }
        }
        /* (bottom) */
        if (!borderimages[5]) {
            /* create newImage surface */
            if ((borderimages[5] = im->newImage("", bordergeom[5].w, bordergeom[5].h))) {
                borderimages[5]->clear();
                if (color.a) {
                    borderimages[5]->setColor(color.r, color.g, color.b, 255);
                    borderimages[5]->fillRectangle(0, 0, bordergeom[5].w, bordergeom[5].h);
                }

                /* set the blitting flags */
                surface->setBlittingFlagsByBrightnessAlphaAndOpacity(brightness, alpha, opacity);

                /* blit the first time */
                surface->stretchBlit(borderimages[5], NULL, &bordergeom[5]);
            }
        }
        /* (left) */
        if (!borderimages[7]) {
            /* create newImage surface */
            if ((borderimages[7] = im->newImage("", bordergeom[7].w, bordergeom[7].h))) {
                borderimages[7]->clear();
                if (color.a) {
                    borderimages[7]->setColor(color.r, color.g, color.b, 255);
                    borderimages[7]->fillRectangle(0, 0, bordergeom[7].w, bordergeom[7].h);
                }

                /* set the blitting flags */
                surface->setBlittingFlagsByBrightnessAlphaAndOpacity(brightness, alpha, opacity);

                /* blit the first time */
                surface->stretchBlit(borderimages[7], NULL, &bordergeom[7]);
            }
        }
    }

    /* unlock */
    surface->unlock();
}

