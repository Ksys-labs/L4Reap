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

#ifndef MMSDVD_H_
#define MMSDVD_H_

#include "mmsmedia/mmsav.h"

/**
 * @brief   Handles DVD playback.
 *
 * @ingroup     mmsmedia
 *
 * @author      Stefan Schwarzer (stefan.schwarzer@diskohq.org)
 * @author      Matthias Hardt (matthias.hardt@diskohq.org)
 * @author      Jens Schneider (pupeider@gmx.de)
 * @author      Guido Madaus (guido.madaus@diskohq.org)
 * @author      Patrick Helterhoff (patrick.helterhoff@diskohq.org)
 * @author		René Bählkow (rene.baehlkow@diskohq.org)
 *
 * This class is derived from MMSAV and specialized in
 * handling DVD playback.
 */
class MMSDVD : public MMSAV {
    private:
        string  		device;             /**< dvd device to use                      */
        int     		audioChannel,       /**< current audio channel                  */
						spuChannel;         /**< current subtitle channel               */
        int     		maxAudioChannels,   /**< number of available audio channels     */
						maxSpuChannels;     /**< number of available subtitle channels  */
        unsigned int 	windowWidth,		/**< width of given video window			*/
						windowHeight;		/**< height of given video window			*/

        void checkDevice(const string device);
        void initialize(MMSWindow *window, const string device, const bool verbose);

        void mouseEvent(const unsigned int event, const unsigned int x, const unsigned int y) const;

#ifdef __HAVE_GSTREAMER__
#endif
#ifdef __HAVE_XINE__
        void xineOpen();
#endif

    protected:

    public:
        /* status constants */
        static const unsigned short STATUS_PREVIOUS                 =  100;     /**< playing previous chapter           */
        static const unsigned short STATUS_NEXT                     =  101;     /**< playing next chapter               */
        static const unsigned short STATUS_EJECT                    =  102;     /**< ejecting DVD                       */
        static const unsigned short STATUS_ANGLE_PREVIOUS           =  103;     /**< setting previous angle             */
        static const unsigned short STATUS_ANGLE_NEXT               =  104;     /**< setting next angle                 */
        static const unsigned short STATUS_AUDIO_PREVIOUS           =  105;     /**< using previous audio channel       */
        static const unsigned short STATUS_AUDIO_NEXT               =  106;     /**< using next audio channel           */
        static const unsigned short STATUS_SPU_PREVIOUS             =  107;     /**< using previous subtitle channel    */
        static const unsigned short STATUS_SPU_NEXT                 =  108;     /**< using next subtitle channel        */


        MMSDVD(MMSWindow *window, const string device = "/dev/dvd", const bool verbose = false);
        ~MMSDVD();

        void startPlaying(const bool cont = true);
        void rewind();
        void previous();
        void next();
        void anglePrevious();
        void angleNext();
        void audioChannelPrevious();
        void audioChannelNext();
        void spuChannelPrevious();
        void spuChannelNext();

        void eject();

		void menuUp();
		void menuDown();
		void menuLeft();
		void menuRight();
		void menuSelect();
		void showMainMenu();
		bool inMenu();

		void mouseButton(const unsigned int x, const unsigned int y) const;
		void mouseMove(const unsigned int x, const unsigned int y) const;

		/* get DVD information */
        string getTitle();
        int    getChapterNumber();
        int    getChapterCount();
        int    getTitleNumber();
        int    getTitleCount();

        void   updateChannelInfo();
};

#endif /*MMSDVD_H_*/
