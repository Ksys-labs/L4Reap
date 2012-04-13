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

#ifndef MMSMENUWIDGETCLASS_H_
#define MMSMENUWIDGETCLASS_H_

#include "mmsgui/theme/mmswidgetclass.h"

//! describe attributes for MMSMenuWidget which are additional to the MMSWidgetClass
namespace MMSGUI_MENUWIDGET_ATTR {

	#define MMSGUI_MENUWIDGET_ATTR_ATTRDESC \
		{ "item_width", TAFF_ATTRTYPE_STRING }, \
		{ "item_height", TAFF_ATTRTYPE_STRING }, \
		{ "item_hmargin", TAFF_ATTRTYPE_UCHAR }, \
		{ "item_vmargin", TAFF_ATTRTYPE_UCHAR }, \
		{ "cols", TAFF_ATTRTYPE_INT }, \
		{ "dim_items", TAFF_ATTRTYPE_UCHAR }, \
		{ "fixed_pos", TAFF_ATTRTYPE_INT }, \
		{ "hloop", TAFF_ATTRTYPE_BOOL }, \
		{ "vloop", TAFF_ATTRTYPE_BOOL }, \
		{ "trans_items", TAFF_ATTRTYPE_UCHAR }, \
		{ "dim_top", TAFF_ATTRTYPE_UCHAR }, \
		{ "dim_bottom", TAFF_ATTRTYPE_UCHAR }, \
		{ "dim_left", TAFF_ATTRTYPE_UCHAR }, \
		{ "dim_right", TAFF_ATTRTYPE_UCHAR }, \
		{ "trans_top", TAFF_ATTRTYPE_UCHAR }, \
		{ "trans_bottom", TAFF_ATTRTYPE_UCHAR }, \
		{ "trans_left", TAFF_ATTRTYPE_UCHAR }, \
		{ "trans_right", TAFF_ATTRTYPE_UCHAR }, \
		{ "zoomsel_width", TAFF_ATTRTYPE_STRING }, \
		{ "zoomsel_height", TAFF_ATTRTYPE_STRING }, \
		{ "zoomsel_shiftx", TAFF_ATTRTYPE_STRING }, \
		{ "zoomsel_shifty", TAFF_ATTRTYPE_STRING }, \
		{ "smooth_scrolling", TAFF_ATTRTYPE_SEQUENCE_MODE }, \
		{ "parent_window", TAFF_ATTRTYPE_STRING }, \
		{ "selimage", TAFF_ATTRTYPE_STRING }, \
		{ "selimage.path", TAFF_ATTRTYPE_STRING }, \
		{ "selimage.name", TAFF_ATTRTYPE_STRING }, \
		{ "smooth_selection", TAFF_ATTRTYPE_SEQUENCE_MODE }, \
		{ "smooth_delay", TAFF_ATTRTYPE_INT }

	#define MMSGUI_MENUWIDGET_ATTR_IDS \
		MMSGUI_MENUWIDGET_ATTR_IDS_item_width, \
		MMSGUI_MENUWIDGET_ATTR_IDS_item_height, \
		MMSGUI_MENUWIDGET_ATTR_IDS_item_hmargin, \
		MMSGUI_MENUWIDGET_ATTR_IDS_item_vmargin, \
		MMSGUI_MENUWIDGET_ATTR_IDS_cols, \
		MMSGUI_MENUWIDGET_ATTR_IDS_dim_items, \
		MMSGUI_MENUWIDGET_ATTR_IDS_fixed_pos, \
		MMSGUI_MENUWIDGET_ATTR_IDS_hloop, \
		MMSGUI_MENUWIDGET_ATTR_IDS_vloop, \
		MMSGUI_MENUWIDGET_ATTR_IDS_trans_items, \
		MMSGUI_MENUWIDGET_ATTR_IDS_dim_top, \
		MMSGUI_MENUWIDGET_ATTR_IDS_dim_bottom, \
		MMSGUI_MENUWIDGET_ATTR_IDS_dim_left, \
		MMSGUI_MENUWIDGET_ATTR_IDS_dim_right, \
		MMSGUI_MENUWIDGET_ATTR_IDS_trans_top, \
		MMSGUI_MENUWIDGET_ATTR_IDS_trans_bottom, \
		MMSGUI_MENUWIDGET_ATTR_IDS_trans_left, \
		MMSGUI_MENUWIDGET_ATTR_IDS_trans_right, \
		MMSGUI_MENUWIDGET_ATTR_IDS_zoomsel_width, \
		MMSGUI_MENUWIDGET_ATTR_IDS_zoomsel_height, \
		MMSGUI_MENUWIDGET_ATTR_IDS_zoomsel_shiftx, \
		MMSGUI_MENUWIDGET_ATTR_IDS_zoomsel_shifty, \
		MMSGUI_MENUWIDGET_ATTR_IDS_smooth_scrolling, \
		MMSGUI_MENUWIDGET_ATTR_IDS_parent_window, \
		MMSGUI_MENUWIDGET_ATTR_IDS_selimage, \
		MMSGUI_MENUWIDGET_ATTR_IDS_selimage_path, \
		MMSGUI_MENUWIDGET_ATTR_IDS_selimage_name, \
		MMSGUI_MENUWIDGET_ATTR_IDS_smooth_selection, \
		MMSGUI_MENUWIDGET_ATTR_IDS_smooth_delay

	#define MMSGUI_MENUWIDGET_ATTR_INIT { \
		MMSGUI_BASE_ATTR_ATTRDESC, \
		MMSGUI_BORDER_ATTR_ATTRDESC, \
		MMSGUI_WIDGET_ATTR_ATTRDESC, \
		MMSGUI_MENUWIDGET_ATTR_ATTRDESC, \
		{ NULL, TAFF_ATTRTYPE_NONE } \
	}

	typedef enum {
		MMSGUI_BASE_ATTR_IDS,
		MMSGUI_BORDER_ATTR_IDS,
		MMSGUI_WIDGET_ATTR_IDS,
		MMSGUI_MENUWIDGET_ATTR_IDS
	} ids;
}

extern TAFF_ATTRDESC MMSGUI_MENUWIDGET_ATTR_I[];


//! A data access class for the menu widget.
/*!
This class is the base for the MMSMenuWidget class.
With this data store you have access to all changeable widget attributes.
It is also one of the base classes for MMSThemeManager and MMSDialogManager
which are main features of the MMSGUI.
\note This class will be internally used by class MMSMenuWidget.
\author Jens Schneider
*/
class MMSMenuWidgetClass {
    private:
    	//! name of the theme class
        string          className;

        //! the copied TAFF buffer
        MMSTaffFile		*tafff;

        //! is itemwidth set?
        bool            isitemwidth;

        //! width of an item (percent or pixel)
        string          itemwidth;

        //! is itemheight set?
        bool            isitemheight;

        //! height of an item (percent or pixel)
        string          itemheight;

        //! is horizontal item margin set?
        bool            isitemhmargin;

        //! horizontal item margin
        unsigned int    itemhmargin;

        //! is vertical item margin set?
        bool            isitemvmargin;

        //! vertical item margin
        unsigned int    itemvmargin;

        //! is the number of columns set?
        bool            iscols;

        //! number of columns
        unsigned int    cols;

        //! is the dimitems value set?
        bool            isdimitems;

        //! dimitems if the menu has not the focus
        unsigned int    dimitems;

        //! is the fixedpos value set?
        bool            isfixedpos;

        //! fixed position of the selected item
        unsigned int    fixedpos;

        //! is the hloop flag set?
        bool            ishloop;

        //! loop horizontal (left/right) without jumping out of the menu
        unsigned int    hloop;

        //! is the vloop flag set?smoothscrolling
        bool            isvloop;

        //! loop vertical (up/down) without jumping out of the menu
        unsigned int    vloop;

        //! is the transitems value set?
        bool            istransitems;

        //! transitems if the menu has not the focus
        unsigned int    transitems;

        //! is the dimtop value set?
        bool            isdimtop;

        //! dim the items at the top of the menu
        unsigned int    dimtop;

        //! is the dimbottom value set?
        bool            isdimbottom;

        //! dim the items at the bottom of the menu
        unsigned int    dimbottom;

        //! is the dimleft value set?
        bool            isdimleft;

        //! dim the items at the left of the menu
        unsigned int    dimleft;

        //! is the dimright value set?
        bool            isdimright;

        //! dim the items at the right of the menu
        unsigned int    dimright;

        //! is the transtop value set?
        bool            istranstop;

        //! transparent the items at the top of the menu
        unsigned int    transtop;

        //! is the transbottom value set?
        bool            istransbottom;

        //! transparent the items at the bottom of the menu
        unsigned int    transbottom;

        //! is the transleft value set?
        bool            istransleft;

        //! transparent the items at the left of the menu
        unsigned int    transleft;

        //! is the transright value set?
        bool            istransright;

        //! transparent the items at the right of the menu
        unsigned int    transright;

        //! is zoom in (width) value set?
        bool			iszoomselwidth;

        //! zoom in (width) the selected item (percent or pixel)
        string			zoomselwidth;

        //! is zoom in (height) value set?
        bool			iszoomselheight;

        //! zoom in (height) the selected item (percent or pixel)
        string          zoomselheight;

        //! is horizontal shift value set?
        bool			iszoomselshiftx;

        //! horizontal shift of the selected item (percent or pixel)
        string			zoomselshiftx;

        //! is vertical shift value set?
        bool			iszoomselshifty;

        //! vertical shift of the selected item (percent or pixel)
        string 			zoomselshifty;

        //! is smooth scrolling mode set?
        bool			issmoothscrolling;

        //! smooth scrolling mode
        MMSSEQUENCEMODE	smoothscrolling;

        //! is parent window set?
        bool			isparentwindow;

        //! name of the parent window
        string 			parentwindow;

        //! is selimage path set?
        bool			isselimagepath;

        //! selimage path
        string 			selimagepath;

        //! is selimage name set?
        bool			isselimagename;

        //! selimage filename
        string 			selimagename;

        //! is smooth selection mode set?
        bool			issmoothselection;

        //! smooth selection mode
        MMSSEQUENCEMODE	smoothselection;

        //! is smooth delay set?
        bool			issmoothdelay;

        //! smooth delay
        unsigned int	smoothdelay;

        //! Read and set all attributes from the given TAFF buffer.
        /*!
        \param tafff   		pointer to the TAFF buffer
        \param prefix  		optional, prefix to all attribute names (<prefix><attrname>=<attrvalue>)
        \param path    		optional, path needed for empty path values from the TAFF buffer
        \param reset_paths  optional, should reset all path attributes?
        */
        void setAttributesFromTAFF(MMSTaffFile *tafff, string *prefix = NULL, string *path = NULL,
								   bool reset_paths = false);

        //! Saves a copy of an TAFF buffer including all child nodes.
        /*!
        \param tafff   pointer to the TAFF buffer
        */
        void duplicateTAFF(MMSTaffFile *tafff);

    public:
    	//! stores base widget attributes
        MMSWidgetClass widgetClass;

        //! Constructor of class MMSMenuWidgetClass.
        MMSMenuWidgetClass();

        //! Destructor of class MMSMenuWidgetClass.
        ~MMSMenuWidgetClass();

        //! Mark all attributes as not set.
        void unsetAll();

        //! Get the copied TAFF buffer, see duplicateTAFF().
        /*!
        \return pointer to the TAFF buffer
        */
        MMSTaffFile *getTAFF();

        //! Set the name of the theme class.
        /*!
        \param classname  name of the class
        */
        void setClassName(string className);

        //! Get the name of the theme class.
        /*!
        \return name of the class
        */
        string getClassName();

        //! Check if the itemwidth is set.
        bool isItemWidth();

        //! Set the width of each item.
        /*!
        \param itemwidth  width of an item (percent or pixel)
        */
        void setItemWidth(string itemwidth);

        //! Mark the itemwidth as not set.
        void unsetItemWidth();

        //! Get the width of an item.
        /*!
        \return item width
        */
        string getItemWidth();

        //! Check if the itemheight is set.
        bool isItemHeight();

        //! Set the height of each item.
        /*!
        \param itemheight  height of an item (percent or pixel)
        */
        void setItemHeight(string itemheight);

        //! Mark the itemheight as not set.
        void unsetItemHeight();

        //! Get the height of an item.
        /*!
        \return item height
        */
        string getItemHeight();

        //! Check if the horizontal item margin is set.
        bool isItemHMargin();

        //! Set the horizontal item margin.
        /*!
        \param itemhmargin  horizontal margin
        */
        void setItemHMargin(unsigned int itemhmargin);

        //! Mark the itemhmargin as not set.
        void unsetItemHMargin();

        //! Get the horizontal item margin.
        /*!
        \return horizontal margin
        */
        unsigned int getItemHMargin();

        //! Check if the vertical item margin is set.
        bool isItemVMargin();

        //! Set the vertical item margin.
        /*!
        \param itemvmargin  vertical margin
        */
        void setItemVMargin(unsigned int itemvmargin);

        //! Mark the itemvmargin as not set.
        void unsetItemVMargin();

        //! Get the vertical item margin.
        /*!
        \return vertical margin
        */
        unsigned int getItemVMargin();

        //! Check if the number of columns is set.
        bool isCols();

        //! Set the number of columns.
        /*!
        \param cols  columns (equal or greater than 1)
        */
        void setCols(unsigned int cols);

        //! Mark the cols as not set.
        void unsetCols();

        //! Get the number of columns.
        /*!
        \return columns
        */
        unsigned int getCols();

        //! Check if the dimitems value is set.
        bool isDimItems();

        //! Set the dimitems value. The menu items will dim with this value if the menu has not the focus.
        /*!
        \param dimitems  dim the menu items with this value (0..255)
        */
        void setDimItems(unsigned int dimitems);

        //! Mark the dimitems as not set.
        void unsetDimItems();

        //! Get the dimitems value.
        /*!
        \return dimitems value
        */
        unsigned int getDimItems();

        //! Check if the fixedpos value is set.
        bool isFixedPos();

        //! Set the fixedpos value. The position of the selected menu item will not changed.
        /*!
        \param fixedpos  fixed position
        */
        void setFixedPos(int fixedpos);

        //! Mark the fixedpos as not set.
        void unsetFixedPos();

        //! Get the fixedpos value.
        /*!
        \return fixed position
        */
        int getFixedPos();

        //! Check if the hloop flag is set.
        bool isHLoop();

        //! Set the hloop flag. If true then it loops horizontal (left/right) without jumping out of the menu.
        /*!
        \param hloop  loop horizontal
        */
        void setHLoop(bool hloop);

        //! Mark the hloop as not set.
        void unsetHLoop();

        //! Get the hloop flag.
        /*!
        \return hloop flag
        */
        bool getHLoop();

        //! Check if the vloop flag is set.
        bool isVLoop();

        //! Set the vloop flag. If true then it loops vertical (up/down) without jumping out of the menu.
        /*!
        \param vloop  loop vertical
        */
        void setVLoop(bool vloop);

        //! Mark the vloop as not set.
        void unsetVLoop();

        //! Get the vloop flag.
        /*!
        \return vloop flag
        */
        bool getVLoop();

        //! Check if the transitems value is set.
        bool isTransItems();

        //! Set the transitems value. The menu items will get transparent with this value if the menu has not the focus.
        /*!
        \param transitems  transparent the menu items with this value (0..255)
        */
        void setTransItems(unsigned int transitems);

        //! Mark the transitems as not set.
        void unsetTransItems();

        //! Get the transitems value.
        /*!
        \return transitems value
        */
        unsigned int getTransItems();

        //! Check if the dimtop value is set.
        bool isDimTop();

        //! Set the dimtop value. The menu items at the top will dim with this value.
        /*!
        \param dimtop  dim the menu items with this value (0..255)
        */
        void setDimTop(unsigned int dimtop);

        //! Mark the dimtop as not set.
        void unsetDimTop();

        //! Get the dimtop value.
        /*!
        \return dimtop value
        */
        unsigned int getDimTop();

        //! Check if the dimbottom value is set.
        bool isDimBottom();

        //! Set the dimbottom value. The menu items at the bottom will dim with this value.
        /*!
        \param dimbottom  dim the menu items with this value (0..255)
        */
        void setDimBottom(unsigned int dimbottom);

        //! Mark the dimbottom as not set.
        void unsetDimBottom();

        //! Get the dimbottom value.
        /*!
        \return dimbottom value
        */
        unsigned int getDimBottom();

        //! Check if the dimleft value is set.
        bool isDimLeft();

        //! Set the dimleft value. The menu items at the left will dim with this value.
        /*!
        \param dimleft  dim the menu items with this value (0..255)
        */
        void setDimLeft(unsigned int dimleft);

        //! Mark the dimleft as not set.
        void unsetDimLeft();

        //! Get the dimleft value.
        /*!
        \return dimleft value
        */
        unsigned int getDimLeft();

        //! Check if the dimright value is set.
        bool isDimRight();

        //! Set the dimright value. The menu items at the right will dim with this value.
        /*!
        \param dimright  dim the menu items with this value (0..255)
        */
        void setDimRight(unsigned int dimright);

        //! Mark the dimright as not set.
        void unsetDimRight();

        //! Get the dimright value.
        /*!
        \return dimright value
        */
        unsigned int getDimRight();

        //! Check if the transtop value is set.
        bool isTransTop();

        //! Set the transtop value. The menu items at the top will get transparent with this value.
        /*!
        \param transtop  transparent the menu items with this value (0..255)
        */
        void setTransTop(unsigned int transtop);

        //! Mark the transtop as not set.
        void unsetTransTop();

        //! Get the transtop value.
        /*!
        \return transtop value
        */
        unsigned int getTransTop();

        //! Check if the transbottom value is set.
        bool isTransBottom();

        //! Set the transbottom value. The menu items at the bottom will get transparent with this value.
        /*!
        \param transbottom  transparent the menu items with this value (0..255)
        */
        void setTransBottom(unsigned int transbottom);

        //! Mark the transbottom as not set.
        void unsetTransBottom();

        //! Get the transbottom value.
        /*!
        \return transbottom value
        */
        unsigned int getTransBottom();

        //! Check if the transleft value is set.
        bool isTransLeft();

        //! Set the transleft value. The menu items at the left will get transparent with this value.
        /*!
        \param transleft  transparent the menu items with this value (0..255)
        */
        void setTransLeft(unsigned int transleft);

        //! Mark the transleft as not set.
        void unsetTransLeft();

        //! Get the transleft value.
        /*!
        \return transleft value
        */
        unsigned int getTransLeft();

        //! Check if the transright value is set.
        bool isTransRight();

        //! Set the transright value. The menu items at the right will get transparent with this value.
        /*!
        \param transright  transparent the menu items with this value (0..255)
        */
        void setTransRight(unsigned int transright);

        //! Mark the transright as not set.
        void unsetTransRight();

        //! Get the transright value.
        /*!
        \return transright value
        */
        unsigned int getTransRight();


        bool isZoomSelWidth();
        void setZoomSelWidth(string zoomselwidth);
        void unsetZoomSelWidth();
        string getZoomSelWidth();
        bool isZoomSelHeight();
        void setZoomSelHeight(string zoomselheight);
        void unsetZoomSelHeight();
        string getZoomSelHeight();
        bool isZoomSelShiftX();
        void setZoomSelShiftX(string zoomselshiftx);
        void unsetZoomSelShiftX();
        string getZoomSelShiftX();
        bool isZoomSelShiftY();
        void setZoomSelShiftY(string zoomselshifty);
        void unsetZoomSelShiftY();
        string getZoomSelShiftY();

        bool isSmoothScrolling();
        void setSmoothScrolling(MMSSEQUENCEMODE smoothscrolling);
        void unsetSmoothScrolling();
        MMSSEQUENCEMODE getSmoothScrolling();

        bool isParentWindow();
        void setParentWindow(string parentwindow);
        void unsetParentWindow();
        string getParentWindow();


        bool isSelImagePath();
        void setSelImagePath(string selimagepath);
        void unsetSelImagePath();
        string getSelImagePath();

        bool isSelImageName();
        void setSelImageName(string selimagename);
        void unsetSelImageName();
        string getSelImageName();

        bool isSmoothSelection();
        void setSmoothSelection(MMSSEQUENCEMODE smoothselection);
        void unsetSmoothSelection();
        MMSSEQUENCEMODE getSmoothSelection();

        bool isSmoothDelay();
        void setSmoothDelay(unsigned int smoothdelay);
        void unsetSmoothDelay();
        unsigned int getSmoothDelay();

    // friends
    friend class MMSThemeManager;
    friend class MMSDialogManager;
};

#endif /*MMSMENUWIDGETCLASS_H_*/
