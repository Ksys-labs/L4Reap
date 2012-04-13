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

#ifndef MMSWINDOWCLASS_H_
#define MMSWINDOWCLASS_H_

#include "mmsgui/theme/mmsborderclass.h"

//! describe attributes for MMSWindow which are additional to the MMSBorderClass
namespace MMSGUI_WINDOW_ATTR {

	#define MMSGUI_WINDOW_ATTR_ATTRDESC \
		{ "alignment", TAFF_ATTRTYPE_STRING }, \
		{ "dx", TAFF_ATTRTYPE_STRING }, \
		{ "dy", TAFF_ATTRTYPE_STRING }, \
		{ "w", TAFF_ATTRTYPE_STRING }, \
		{ "h", TAFF_ATTRTYPE_STRING }, \
		{ "bgcolor", TAFF_ATTRTYPE_COLOR }, \
		{ "bgcolor.a", TAFF_ATTRTYPE_UCHAR }, \
		{ "bgcolor.r", TAFF_ATTRTYPE_UCHAR }, \
		{ "bgcolor.g", TAFF_ATTRTYPE_UCHAR }, \
		{ "bgcolor.b", TAFF_ATTRTYPE_UCHAR }, \
		{ "bgimage", TAFF_ATTRTYPE_STRING }, \
		{ "bgimage.path", TAFF_ATTRTYPE_STRING }, \
		{ "bgimage.name", TAFF_ATTRTYPE_STRING }, \
		{ "opacity", TAFF_ATTRTYPE_UCHAR }, \
		{ "fadein", TAFF_ATTRTYPE_BOOL }, \
		{ "fadeout", TAFF_ATTRTYPE_BOOL }, \
		{ "debug", TAFF_ATTRTYPE_BOOL }, \
		{ "margin", TAFF_ATTRTYPE_UCHAR }, \
		{ "up_arrow", TAFF_ATTRTYPE_STRING }, \
		{ "down_arrow", TAFF_ATTRTYPE_STRING }, \
		{ "left_arrow", TAFF_ATTRTYPE_STRING }, \
		{ "right_arrow", TAFF_ATTRTYPE_STRING }, \
		{ "navigate_up", TAFF_ATTRTYPE_STRING }, \
		{ "navigate_down", TAFF_ATTRTYPE_STRING }, \
		{ "navigate_left", TAFF_ATTRTYPE_STRING }, \
		{ "navigate_right", TAFF_ATTRTYPE_STRING }, \
		{ "own_surface", TAFF_ATTRTYPE_BOOL }, \
		{ "movein", TAFF_ATTRTYPE_STRING }, \
		{ "moveout", TAFF_ATTRTYPE_STRING }, \
		{ "modal", TAFF_ATTRTYPE_BOOL }, \
		{ "static_zorder", TAFF_ATTRTYPE_BOOL }, \
		{ "always_on_top", TAFF_ATTRTYPE_BOOL }, \
		{ "focusable", TAFF_ATTRTYPE_BOOL }, \
		{ "backbuffer", TAFF_ATTRTYPE_BOOL }, \
		{ "initial_load", TAFF_ATTRTYPE_BOOL }

	#define MMSGUI_WINDOW_ATTR_IDS \
		MMSGUI_WINDOW_ATTR_IDS_alignment, \
		MMSGUI_WINDOW_ATTR_IDS_dx, \
		MMSGUI_WINDOW_ATTR_IDS_dy, \
		MMSGUI_WINDOW_ATTR_IDS_w, \
		MMSGUI_WINDOW_ATTR_IDS_h, \
		MMSGUI_WINDOW_ATTR_IDS_bgcolor, \
		MMSGUI_WINDOW_ATTR_IDS_bgcolor_a, \
		MMSGUI_WINDOW_ATTR_IDS_bgcolor_r, \
		MMSGUI_WINDOW_ATTR_IDS_bgcolor_g, \
		MMSGUI_WINDOW_ATTR_IDS_bgcolor_b, \
		MMSGUI_WINDOW_ATTR_IDS_bgimage, \
		MMSGUI_WINDOW_ATTR_IDS_bgimage_path, \
		MMSGUI_WINDOW_ATTR_IDS_bgimage_name, \
		MMSGUI_WINDOW_ATTR_IDS_opacity, \
		MMSGUI_WINDOW_ATTR_IDS_fadein, \
		MMSGUI_WINDOW_ATTR_IDS_fadeout, \
		MMSGUI_WINDOW_ATTR_IDS_debug, \
		MMSGUI_WINDOW_ATTR_IDS_margin, \
		MMSGUI_WINDOW_ATTR_IDS_up_arrow, \
		MMSGUI_WINDOW_ATTR_IDS_down_arrow, \
		MMSGUI_WINDOW_ATTR_IDS_left_arrow, \
		MMSGUI_WINDOW_ATTR_IDS_right_arrow, \
		MMSGUI_WINDOW_ATTR_IDS_navigate_up, \
		MMSGUI_WINDOW_ATTR_IDS_navigate_down, \
		MMSGUI_WINDOW_ATTR_IDS_navigate_left, \
		MMSGUI_WINDOW_ATTR_IDS_navigate_right, \
		MMSGUI_WINDOW_ATTR_IDS_own_surface, \
		MMSGUI_WINDOW_ATTR_IDS_movein, \
		MMSGUI_WINDOW_ATTR_IDS_moveout, \
		MMSGUI_WINDOW_ATTR_IDS_modal, \
		MMSGUI_WINDOW_ATTR_IDS_static_zorder, \
		MMSGUI_WINDOW_ATTR_IDS_always_on_top, \
		MMSGUI_WINDOW_ATTR_IDS_focusable, \
		MMSGUI_WINDOW_ATTR_IDS_backbuffer, \
		MMSGUI_WINDOW_ATTR_IDS_initial_load

	typedef enum {
		MMSGUI_BASE_ATTR_IDS,
		MMSGUI_BORDER_ATTR_IDS,
		MMSGUI_WINDOW_ATTR_IDS
	} ids;
}


class MMSWindowClass {
    private:
    	struct {
	        bool         isalignment;
	        MMSALIGNMENT alignment;
	        bool         isdx;
	        bool         isdy;
	        bool         iswidth;
	        bool         isheight;
	        bool         isbgcolor;
	        MMSFBColor   bgcolor;
	        bool         isbgimagepath;
	        bool         isbgimagename;
	        bool         isopacity;
	        unsigned int opacity;
	        bool         isfadein;
	        bool         fadein;
	        bool         isfadeout;
	        bool         fadeout;
	        bool         isdebug;
	        bool         debug;
	        bool         ismargin;
	        unsigned int margin;
	        bool         isuparrow;         //! is the uparrow set?
	        bool         isdownarrow;       //! is the downarrow set?
	        bool         isleftarrow;       //! is the leftarrow set?
	        bool         isrightarrow;      //! is the rightarrow set?
	        bool         isnavigateup;      //! is the navigateup set?
	        bool         isnavigatedown;    //! is the navigatedown set?
	        bool         isnavigateleft;    //! is the navigateleft set?
	        bool         isnavigateright;   //! is the navigateright set?
	        bool         isownsurface;
	        bool		 ownsurface;
	        bool         ismovein;
	        MMSDIRECTION movein;
	        bool         ismoveout;
	        MMSDIRECTION moveout;

	        //! is modal flag set?
	        bool		ismodal;

	        //! if true, the focus cannot be changed to another window
	        bool		modal;

	        //! is static zorder flag set?
	        bool		isstaticzorder;

	        //! if true, the zorder of child windows will not automatically changed during show() or setFocus()
	        bool		staticzorder;

	        //! is always on top flag set?
	        bool		isalwaysontop;

	        //! if true, the window will be permanently displayed at the top of the window stack
	        bool		alwaysontop;

	        //! is focusable flag set?
	        bool		isfocusable;

	        //! window can get the focus true/false
	        bool		focusable;

	        //! is backbuffer flag set?
	        bool		isbackbuffer;

	        //! window surface has an backbuffer true/false
	        bool		backbuffer;

	        //! is initial load flag set?
	        bool		isinitialload;

	        //! window should load images, fonts etc. during initialization true/false
	        bool		initialload;
    	} id;

        struct {
	        string       *dx;
	        string       *dy;
	        string       *width;
	        string       *height;
	        string       *bgimagepath;
	        string       *bgimagename;
	        string       *uparrow;          //! the name of the widget which represents the scroll up arrow
	        string       *downarrow;        //! the name of the widget which represents the scroll down arrow
	        string       *leftarrow;        //! the name of the widget which represents the scroll left arrow
	        string       *rightarrow;       //! the name of the widget which represents the scroll right arrow
	        string       *navigateup;       //! the name of the window to which should navigate up
	        string       *navigatedown;     //! the name of the window to which should navigate down
	        string       *navigateleft;     //! the name of the window to which should navigate left
	        string       *navigateright;   	//! the name of the window to which should navigate right
		} ed;

    	/* init routines */
        void initAlignment();
        void initDx();
        void initDy();
        void initWidth();
        void initHeight();
        void initBgColor();
        void initBgImagePath();
        void initBgImageName();
        void initOpacity();
        void initFadeIn();
        void initFadeOut();
        void initDebug();
        void initMargin();
        void initUpArrow();
        void initDownArrow();
        void initLeftArrow();
        void initRightArrow();
        void initNavigateUp();
        void initNavigateDown();
        void initNavigateLeft();
        void initNavigateRight();
        void initOwnSurface();
        void initMoveIn();
        void initMoveOut();
        void initModal();
        void initStaticZOrder();
        void initAlwaysOnTop();
        void initFocusable();
        void initBackBuffer();
        void initInitialLoad();

    	/* free routines */
        void freeAlignment();
        void freeDx();
        void freeDy();
        void freeWidth();
        void freeHeight();
        void freeBgColor();
        void freeBgImagePath();
        void freeBgImageName();
        void freeOpacity();
        void freeFadeIn();
        void freeFadeOut();
        void freeDebug();
        void freeMargin();
        void freeUpArrow();
        void freeDownArrow();
        void freeLeftArrow();
        void freeRightArrow();
        void freeNavigateUp();
        void freeNavigateDown();
        void freeNavigateLeft();
        void freeNavigateRight();
        void freeOwnSurface();
        void freeMoveIn();
        void freeMoveOut();
        void freeModal();
        void freeStaticZOrder();
        void freeAlwaysOnTop();
        void freeFocusable();
        void freeBackBuffer();
        void freeInitialLoad();

        //! Read and set all attributes from the given TAFF buffer.
        /*!
        \param tafff   		pointer to the TAFF buffer
        \param path    		optional, path needed for empty path values from the TAFF buffer
        \param reset_paths  optional, should reset all path attributes?
        */
        void setAttributesFromTAFF(MMSTaffFile *tafff, string *path = NULL, bool reset_paths = false);

    public:
        MMSBorderClass border;

        MMSWindowClass();
        ~MMSWindowClass();
        MMSWindowClass &operator=(const MMSWindowClass &c);
        //
        void unsetAll();

        bool isAlignment();
        void unsetAlignment();
        void setAlignment(MMSALIGNMENT alignment);
        bool getAlignment(MMSALIGNMENT &alignment);
        //
        bool isDx();
        void unsetDx();
        void setDx(const string &dx);
        bool getDx(string &dx);
        //
        bool isDy();
        void unsetDy();
        void setDy(const string &dy);
        bool getDy(string &dy);
        //
        bool isWidth();
        void unsetWidth();
        void setWidth(const string &width);
        bool getWidth(string &width);
        //
        bool isHeight();
        void unsetHeight();
        void setHeight(const string &height);
        bool getHeight(string &height);
        //
        bool isBgColor();
        void unsetBgColor();
        void setBgColor(const MMSFBColor &bgcolor);
        bool getBgColor(MMSFBColor &bgcolor);
        //
        bool isBgImagePath();
        void unsetBgImagePath();
        void setBgImagePath(const string &bgimagepath);
        bool getBgImagePath(string &bgimagepath);
        //
        bool isBgImageName();
        void unsetBgImageName();
        void setBgImageName(const string &bgimagename);
        bool getBgImageName(string &bgimagename);
        //
        bool isOpacity();
        void unsetOpacity();
        void setOpacity(unsigned int opacity);
        bool getOpacity(unsigned int &opacity);
        //
        bool isFadeIn();
        void unsetFadeIn();
        void setFadeIn(bool fadein);
        bool getFadeIn(bool &fadein);
        //
        bool isFadeOut();
        void unsetFadeOut();
        void setFadeOut(bool fadeout);
        bool getFadeOut(bool &fadeout);
        //
        bool isDebug();
        void unsetDebug();
        void setDebug(bool debug);
        bool getDebug(bool &debug);
        //
        bool isMargin();
        void unsetMargin();
        void setMargin(unsigned int margin);
        bool getMargin(unsigned int &margin);

        ////////////////////////////////////////////////////////////////////////

        //! Check if the uparrow is set.
        bool isUpArrow();

        //! Mark the uparrow as not set.
        void unsetUpArrow();

        //! Set the uparrow.
        /*!
        \param uparrow  the name of the widget which represents the navigate up arrow
        */
        void setUpArrow(const string &uparrow);


        //! Get the uparrow.
        /*!
        \param uparrow  the name of the widget which represents the navigate up arrow
        \return true if set
        */
        bool getUpArrow(string &uparrow);

        ////////////////////////////////////////////////////////////////////////

        //! Check if the downarrow is set.
        bool isDownArrow();

        //! Mark the downarrow as not set.
        void unsetDownArrow();

        //! Set the downarrow.
        /*!
        \param downarrow  the name of the widget which represents the navigate down arrow
        */
        void setDownArrow(const string &downarrow);

        //! Get the downarrow.
        /*!
        \param downarrow  the name of the widget which represents the navigate down arrow
        \return true if set
        */
        bool getDownArrow(string &downarrow);

        ////////////////////////////////////////////////////////////////////////

        //! Check if the leftarrow is set.
        bool isLeftArrow();

        //! Mark the leftarrow as not set.
        void unsetLeftArrow();

        //! Set the leftarrow.
        /*!
        \param leftarrow  the name of the widget which represents the navigate left arrow
        */
        void setLeftArrow(const string &leftarrow);

        //! Get the leftarrow.
        /*!
        \param leftarrow  the name of the widget which represents the navigate left arrow
        \return true if set
        */
        bool getLeftArrow(string &leftarrow);

        ////////////////////////////////////////////////////////////////////////

        //! Check if the rightarrow is set.
        bool isRightArrow();

        //! Mark the rightarrow as not set.
        void unsetRightArrow();

        //! Set the rightarrow.
        /*!
        \param rightarrow  the name of the widget which represents the navigate right arrow
        */
        void setRightArrow(const string &rightarrow);

        //! Get the rightarrow.
        /*!
        \param rightarrow  the name of the widget which represents the navigate right arrow
        \return true if set
        */
        bool getRightArrow(string &rightarrow);

        ////////////////////////////////////////////////////////////////////////

        //! Check if the navigateup is set.
        bool isNavigateUp();

        //! Mark the navigateup as not set.
        void unsetNavigateUp();

        //! Set the navigateup window.
        /*!
        \param navigateup  the name of the window to which should navigate up
        */
        void setNavigateUp(const string &navigateup);

        //! Get the navigateup window.
        /*!
        \param navigateup  the name of the window to which should navigate up
        \return true if set
        */
        bool getNavigateUp(string &navigateup);

        ////////////////////////////////////////////////////////////////////////

        //! Check if the navigatedown is set.
        bool isNavigateDown();

        //! Mark the navigatedown as not set.
        void unsetNavigateDown();

        //! Set the navigatedown window.
        /*!
        \param navigatedown  the name of the window to which should navigate down
        */
        void setNavigateDown(const string &navigatedown);

        //! Get the navigatedown window.
        /*!
        \param navigatedown  the name of the window to which should navigate down
        \return true if set
        */
        bool getNavigateDown(string &navigatedown);

        ////////////////////////////////////////////////////////////////////////

        //! Check if the navigateleft is set.
        bool isNavigateLeft();

        //! Mark the navigateleft as not set.
        void unsetNavigateLeft();

        //! Set the navigateleft window.
        /*!
        \param navigateleft  the name of the window to which should navigate left
        */
        void setNavigateLeft(const string &navigateleft);

        //! Get the navigateleft window.
        /*!
        \param navigateleft  the name of the window to which should navigate left
        \return true if set
        */
        bool getNavigateLeft(string &navigateleft);

        ////////////////////////////////////////////////////////////////////////

        //! Check if the navigateright is set.
        bool isNavigateRight();

        //! Mark the navigateright as not set.
        void unsetNavigateRight();

        //! Set the navigateright.
        /*!
        \param navigateright  the name of the window to which should navigate right
        */
        void setNavigateRight(const string &navigateright);

        //! Get the navigateright window.
        /*!
        \param navigateright  the name of the window to which should navigate right
        \return true if set
        */
        bool getNavigateRight(string &navigateright);

        //
        bool isOwnSurface();
        void unsetOwnSurface();
        void setOwnSurface(bool ownsurface);
        bool getOwnSurface(bool &ownsurface);


        bool isMoveIn();
        void unsetMoveIn();
        void setMoveIn(MMSDIRECTION movein);
        bool getMoveIn(MMSDIRECTION &movein);

        bool isMoveOut();
        void unsetMoveOut();
        void setMoveOut(MMSDIRECTION moveout);
        bool getMoveOut(MMSDIRECTION &moveout);

        bool isModal();
        void unsetModal();
        void setModal(bool modal);
        bool getModal(bool &modal);

        bool isStaticZOrder();
        void unsetStaticZOrder();
        void setStaticZOrder(bool staticzorder);
        bool getStaticZOrder(bool &staticzorder);

        bool isAlwaysOnTop();
        void unsetAlwaysOnTop();
        void setAlwaysOnTop(bool alwaysontop);
        bool getAlwaysOnTop(bool &alwaysontop);

        bool isFocusable();
        void unsetFocusable();
        void setFocusable(bool focusable);
        bool getFocusable(bool &focusable);

        bool isBackBuffer();
        void unsetBackBuffer();
        void setBackBuffer(bool backbuffer);
        bool getBackBuffer(bool &backbuffer);

        bool isInitialLoad();
        void unsetInitialLoad();
        void setInitialLoad(bool initialload);
        bool getInitialLoad(bool &initialload);

    // friends
    friend class MMSThemeManager;
    friend class MMSDialogManager;
};

#endif /*MMSWINDOWCLASS_H_*/
