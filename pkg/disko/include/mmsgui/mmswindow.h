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

#ifndef MMSWINDOW_H_
#define MMSWINDOW_H_

#include "mmsgui/mmswidget.h"
#include "mmsgui/mmswindowaction.h"
#include "mmsgui/mmsimagemanager.h"
#include "mmsgui/mmsfontmanager.h"
#include "mmsgui/mmsfbmanager.h"
#include "mmsgui/interfaces/immswindowmanager.h"
#include "mmstools/mmsmutex.h"
#include "mmstools/mmspulser.h"

// support old renamed methods
#define searchForWindow		findWindow
#define searchForWidget		findWidget
#define searchForWidgetType	findWidgetType

//! The available types of windows.
typedef enum {
	/*!
	Main windows will be displayed over the root window.
	Only one main window can be shown at the same time.
	If a main window appears the currently shown main window will be disappear.
	*/
	MMSWINDOWTYPE_MAINWINDOW = 0,
	/*!
	Popup windows will be displayed over root and main windows.
	The popup window which appears finally is on the top of the screen.
	*/
    MMSWINDOWTYPE_POPUPWINDOW,
    /*!
    Root windows will be displayed in the background.
	Only one root window can be shown at the same time.
	If a root window appears the currently shown root window will be disappear.
	*/
    MMSWINDOWTYPE_ROOTWINDOW,
    /*!
    Child windows are parts of main, popup and root windows.
    The full window functionality is given.
    */
    MMSWINDOWTYPE_CHILDWINDOW
} MMSWINDOWTYPE;

//! The available window flags.
typedef enum {
	//! none
    MMSW_NONE               = 0x00000000,
    //! The window displays a video stream and should be on the video layer if it exists.
    MMSW_VIDEO              = 0x00000001,
    //! The window should use the graphics layer.
    MMSW_USEGRAPHICSLAYER   = 0x00000002
} MMSWINDOW_FLAGS;

class MMSChildWindow;

//! current mode of the pulser
typedef enum {
	//! show action
	MMSWINDOW_PULSER_MODE_SHOW = 0,
	//! hide action
	MMSWINDOW_PULSER_MODE_HIDE
} MMSWINDOW_PULSER_MODE;

//! This class is the base class for all windows.
/*!
This class includes the base functionality available for all windows within MMSGUI.
This class cannot be constructed. Only windows which are derived from this class can be constructed.
\author Jens Schneider
*/
class MMSWindow {
    private:

    	//! describes a child window
        typedef struct {
        	//! points to the child window
            MMSWindow       *window;
            //! region of the window within parent window
            MMSFBRegion     region;
            //! opacity of the window
            unsigned char   opacity;
         	//! old opacity of the window
            unsigned char   oldopacity;
            //! save the last focused widget here
            unsigned int    focusedWidget;
            //! special blit done
            bool			special_blit;
        } CHILDWINS;

        //! status area for the arrow widgets
        typedef struct {
        	//! currently navigate up is possible?
        	bool up;
        	//! currently navigate down is possible?
        	bool down;
        	//! currently navigate left is possible?
        	bool left;
        	//! currently navigate right is possible?
        	bool right;
        } ARROW_WIDGET_STATUS;

        //! type of the window
        MMSWINDOWTYPE type;

        //! access to the theme which is used
        MMSTheme            *theme;

        //! base attributes of the window
        /*!
        This can be initialization values from theme.cpp
        or from theme.xml (NOT a \<class/\> definition, but e.g. tag \<mainwindow/\>).
        */
        MMSWindowClass      *baseWindowClass;

        //! attributes set by \<class/\> tag in theme.xml
        /*!
        Is NULL, if window has no theme class definition.
        Attributes set here, prevails over attributes from baseWindowClass.
        */
        MMSWindowClass      *windowClass;

        //! attributes of the window which will be set during the runtime
        /*!
        The runtime attributes set here (e.g. window->setAlignment(...) ), prevails over attributes
        from windowClass and baseWindowClass.
        */
        MMSWindowClass      myWindowClass;

        //! window creation flags
        MMSWINDOW_FLAGS		flags;

        //! to make it thread-safe :)
		MMSMutex      		Lock;

		//! save the id of the thread which has locked the window
        unsigned long       TID;

        //! count the number of times the thread has call lock() because it is not a problem, that the same thread calls lock() several times
        unsigned long       Lock_cnt;

        //! special draw lock
        MMSMutex      		drawLock;

        //! special flip lock
        MMSMutex      		flipLock;

        //! lock the pre-calculation of the navigation
        MMSMutex  			preCalcNaviLock;

        //! is window initialized?
        bool                initialized;

        //! pre-calc navigation done?
        bool                precalcnav;

        //! name of the window
        string 				name;

        //! parent window (if window is a child window) or NULL
        MMSWindow           *parent;

        //! toplevel parent window (if window is a child window) or NULL
        MMSWindow			*toplevel_parent;

        //! image manager for the window (layer 1)
        static MMSImageManager 	*im1;

        //! layer for im1
        static MMSFBLayer		*im1_layer;

        //! image manager for the window (layer 2)
        static MMSImageManager	*im2;

        //! layer for im2
        static MMSFBLayer		*im2_layer;

        //! image manager for the window
        MMSImageManager     *im;

        //! font manager for the window
        static MMSFontManager	*fm;

        //! window action thread (used for animations)
        MMSWindowAction   	*action;

        //!	background image
        MMSFBSurface        *bgimage;

        //! background image set by application
        bool				bgimage_from_external;

        //! border images
        MMSFBSurface        *borderimages[MMSBORDER_IMAGE_NUM_SIZE];

        //! border geometry
        MMSFBRectangle    	bordergeom[MMSBORDER_IMAGE_NUM_SIZE];

        //! border geometry set?
        bool                bordergeomset;

        //! access to the MMSFBLayer on which the window has to be displayed
        MMSFBLayer          *layer;

        //! a full screen MMSFBWindow as buffer for all MMSRootWindow objects with own_surface=false
        static MMSFBWindow 	*fullscreen_root_window;

        //! use count for fullscreen_root_window
        static int			fullscreen_root_window_use_count;

        //! a full screen MMSFBWindow as buffer for all MMSMainWindow objects with own_surface=false
        static MMSFBWindow 	*fullscreen_main_window;

        //! use count for fullscreen_main_window
        static int			fullscreen_main_window_use_count;

        //! access to the MMSFBWindow which is behind of this class
        MMSFBWindow         *window;

        //! access to the MMSFBSurface of the window
        MMSFBSurface        *surface;

        //! visible screen area (that means the visible area e.g. on the TV set), see the initialization of the MMSWindowManager
        MMSFBRectangle 		vrect;

        //! x-movement of the window based on the alignment attribute
        int                 dxpix;

        //! y-movement of the window based on the alignment attribute
        int                 dypix;

        //! geometry of the window based on the margin attribute
        MMSFBRectangle        geom;

        //! inner geometry of the window based on the border margin attribute
        MMSFBRectangle        innerGeom;

        //! check and recalc the geometry of the widgets during the next draw()?
        bool				draw_setgeom;

        //! widgets of the window
        vector<MMSWidget *>	children;

        //! focused widget or NULL
        MMSWidget 			*focusedwidget;

        //! is window shown?
        bool                shown;

        //! is show animation running?
        bool				willshow;

        //! is hide animation running?
        bool				willhide;

        //! focus set the first time?
        bool                firstfocusset;

        //! child windows of the window
        vector<CHILDWINS>   childwins;

        //! focused child window
        unsigned int        focusedChildWin;

        //! widget which has to be selected if it is possible to navigate up
        MMSWidget       	*upArrowWidget;

        //! widget which has to be selected if it is possible to navigate down
        MMSWidget       	*downArrowWidget;

        //! widget which has to be selected if it is possible to navigate left
        MMSWidget       	*leftArrowWidget;

        //! widget which has to be selected if it is possible to navigate right
        MMSWidget       	*rightArrowWidget;

        //! up/down/left/right arrow widgets updated the first time?
        bool            	initialArrowsDrawn;

        //! child window which is to be focused if user navigates up
        MMSWindow       	*navigateUpWindow;

        //! child window which is to be focused if user navigates down
        MMSWindow       	*navigateDownWindow;

        //! child window which is to be focused if user navigates left
        MMSWindow       	*navigateLeftWindow;

    	//! child window which is to be focused if user navigates right
        MMSWindow       	*navigateRightWindow;

        //! widget on which the user has pressed the (mouse) button
        MMSWidget			*buttonpress_widget;

        //! child window on which the user has pressed the (mouse) button
        MMSWindow			*buttonpress_childwin;

        //! window will be stretched to the layer or the parent window using stretchBlit(), default is false
        bool 				stretchmode;

        //! stretch the window to X percent of the window WIDTH to the left side
        //! the value is valid, if window is in stretch mode
        //! a value of 25600 means 100% (normal blit() will be used)
        int					stretchLeft;

        //! stretch the window to X percent of the window HEIGHT to the up side
        //! the value is valid, if window is in stretch mode
        //! a value of 25600 means 100% (normal blit() will be used)
        int					stretchUp;

        //! stretch the window to X percent of the window WIDTH to the right side
        //! the value is valid, if window is in stretch mode
        //! a value of 25600 means 100% (normal blit() will be used)
        int					stretchRight;

        //! stretch the window to X percent of the window HEIGHT to the down side
        //! the value is valid, if window is in stretch mode
        //! a value of 25600 means 100% (normal blit() will be used)
        int					stretchDown;


        //! index in childwins vector for the first window with the always on top flag
        unsigned int        always_on_top_index;


        //! Pulser for e.g. fade/move animations during show/hide
        MMSPulser				pulser;

        //! connection object for MMSPulser::onBeforeAnimation callback
        sigc::connection 		onBeforeAnimation_connection;

        //! connection object for MMSPulser::onAnimation callback
        sigc::connection 		onAnimation_connection;

        //! connection object for MMSPulser::onAfterAnimation callback
        sigc::connection 		onAfterAnimation_connection;

        //! current pulser mode
        MMSWINDOW_PULSER_MODE	pulser_mode;


        unsigned int	anim_opacity;
        MMSFBRectangle	anim_rect;
        bool			anim_fade;
        MMSDIRECTION	anim_move;
    	unsigned int 	anim_opacity_step;
		int 			anim_move_step;


		bool			need_redraw;


        //! Internal method: Creates the window.
        bool create(string dx, string dy, string w, string h, MMSALIGNMENT alignment, MMSWINDOW_FLAGS flags,
        		    bool *own_surface, bool *backbuffer);

        //! Internal method: Creates the window.
        bool create(string w, string h, MMSALIGNMENT alignment, MMSWINDOW_FLAGS flags,
					bool *own_surface, bool *backbuffer);

        //! Internal method: Resize the window.
        bool resize(bool refresh = true);

        //! Internal method: Add a child window.
        bool addChildWindow(MMSWindow *childwin);

        //! Internal method: Remove a child window.
        bool removeChildWindow(MMSWindow *childwin);

        //! Internal method: Set the opacity of a child window.
        bool setChildWindowOpacity(MMSWindow *childwin, unsigned char opacity, bool refresh = true);

        //! Internal method: Set the region of a child window.
        bool setChildWindowRegion(MMSWindow *childwin, bool refresh = true);

        //! Internal method: Move a child window.
        bool moveChildWindow(MMSWindow *childwin, int x, int y, bool refresh = true);

        //! Internal method: Draw a child window.
        void drawChildWindows(MMSFBSurface *dst_surface, MMSFBRegion *region = NULL, int offsX = 0, int offsY = 0);

        //! Internal method: Flip a window.
        bool flipWindow(MMSWindow *win = NULL, MMSFBRegion *region = NULL,
                        MMSFBFlipFlags flags = MMSFB_FLIP_NONE,
                        bool flipChildSurface = true, bool locked = false);

        //! Internal method: Remove the focus from a child window.
        void removeFocusFromChildWindow();

        //! Internal method: Load widgets for up/down/left/right arrows.
        void loadArrowWidgets();

        //! Internal method: Get the navigation status. With this infos we can select/unselect the arrow widgets.
        void getArrowWidgetStatus(ARROW_WIDGET_STATUS *setarrows);

        //! Internal method: Update the status of the arrow widgets.
        void switchArrowWidgets();

        //! Internal method: Init navigation.
        bool initnav();

        //! Internal method: Load images and setup other things.
        virtual bool init();

        //! Internal method: Release images and other things.
        virtual bool release();

        //! Internal method: Draw me.
        virtual void draw(bool toRedrawOnly = false, MMSFBRectangle *rect2update = NULL,
        				  bool clear = true, unsigned char opacity = 255);


		//! tbd
        bool buffered_shown;

        //! tbd
        void showBufferedShown();


        //! Internal method: Draw my border.
        void drawMyBorder(unsigned char opacity = 255);

        //! Internal method: Focus one widget/child window for the first time.
        bool setFirstFocus(bool cw = false);

        //! Internal method: Used to find best candidate to navigate up from currPos.
        double calculateDistGradCode_Up(MMSFBRectangle currPos, MMSFBRectangle candPos);

        //! Internal method: Used to find best candidate to navigate down from currPos.
        double calculateDistGradCode_Down(MMSFBRectangle currPos, MMSFBRectangle candPos);

        //! Internal method: Used to find best candidate to navigate left from currPos.
        double calculateDistGradCode_Left(MMSFBRectangle currPos, MMSFBRectangle candPos);

        //! Internal method: Used to find best candidate to navigate right from currPos.
        double calculateDistGradCode_Right(MMSFBRectangle currPos, MMSFBRectangle candPos);

        //! Internal method: Handle widget navigation (up/down/left/right).
        bool handleNavigationForWidgets(MMSInputEvent *inputevent);

        //! Internal method: Remove the focus from the currently focused child window. Goal: Change status of widgets.
        void removeChildWinFocus();

        //! Internal method: Restore the focus to the currently focused child window. Goal: Change status of widgets.
        bool restoreChildWinFocus(MMSInputEvent *inputevent = NULL);

        //! Internal method: Handle child window navigation (up/down/left/right).
        bool handleNavigationForChildWins(MMSInputEvent *inputevent);

        //! Internal method: Do the pre-calculation of the navigation routes.
        void preCalcNavigation();

        //! Internal method: Lock the window. Will be used by the widgets.
        void lock();

        //! Internal method: Unlock the window. Will be used by the widgets.
        void unlock();

        bool onBeforeAnimation(MMSPulser *pulser);
        bool onAnimation(MMSPulser *pulser);
        void onAfterAnimation(MMSPulser *pulser);

        virtual bool beforeShowAction(MMSPulser *pulser);
        virtual bool showAction(MMSPulser *pulser);
        virtual void afterShowAction(MMSPulser *pulser);

        virtual bool beforeHideAction(MMSPulser *pulser);
        virtual bool hideAction(MMSPulser *pulser);
        virtual void afterHideAction(MMSPulser *pulser);


        //! Internal method: Will be called from the MMSWindowAction thread if the window should disappear.
//        virtual bool hideAction(bool *stopaction);

        //! Internal method: Refresh a part of a window. Will be used by the widgets.
        void refreshFromChild(MMSWidget *child, MMSFBRectangle *rect2update = NULL, bool check_shown = true);

        //! Internal method: Set the focused widget.
        void setFocusedWidget(MMSWidget *child, bool set, bool switchfocus = false, bool refresh = true);

        //! Internal method: Will be called by MMSInputManager if the window has the input focus.
        bool handleInput(MMSInputEvent *inputevent);

        //! Internal method: (Re-)calculate the position and size of all widgets.
        void recalculateChildren();

        //! Internal method: Show the window without animation.
		void instantShow();

        //! Internal method: Hide the window without animation.
		void instantHide();

        //! Internal method: Give window a recalculation hint used for next draw().
		void setWidgetGeometryOnNextDraw();

        //! Internal method: Inform the window, that the language has changed.
        void targetLangChanged(MMSLanguage lang, bool refresh = true);

        //! Internal method: Inform the window, that the theme has changed.
        void themeChanged(string &themeName, bool refresh = true);

    protected:

    	//! interface to the window manager
        static class IMMSWindowManager 	*windowmanager;

    public:

    	//! The base constructor for all window types.
    	/*! This will internally used by the supported window types/classes (see MMSWINDOWTYPE).
    	*/
        MMSWindow();

        //! The base destructor for this class.
        virtual ~MMSWindow();

        //! Get the type of the window.
        /*!
        \return type of the window
        */
        MMSWINDOWTYPE getType();

        //! Get the name of the window.
        /*!
        \return name of the window
        */
        string getName();

        //! Set the name of the window.
        /*!
        \param name		name of the window
        */
        void   setName(string name);

        //! Find a window over its name.
        /*!
        \param name		name of the window
        \return pointer to the MMSWindow object or NULL
        */
        MMSWindow* findWindow(string name);

        //! Return last window in the stack.
        /*!
        \return pointer to the MMSWindow object or NULL
        */
        MMSWindow* getLastWindow();

        //! Makes a window visible.
        /*!
        \return true if the show action successfully started
        \note The MMSWindowAction thread will be started here and show() returnes immediately!
        */
        virtual bool show();

        //! Hide a visible window.
        /*!
        \param goback	this parameter will be routed to the onHide callback, so the application can
        				decide which if and which window should appear afterwards
        \param wait		waiting until hideAction is finished?
        \return true if the hide action successfully started
        \note The MMSWindowAction thread will be started here and hide() returnes immediately if wait=false!
        */
        virtual bool hide(bool goback = false, bool wait = false);

        //! Can be called after show() waiting for end of showAction thread.
        void waitUntilShown();

        //! Can be called after hide() waiting for end of hideAction thread.
        void waitUntilHidden();

        //! Get the geometry of the window.
        /*!
        \return the rectangle of the window on the layer or parent window
        */
        MMSFBRectangle getGeometry();

        //! Get the geometry of the window based on the layer.
        /*!
        \return the rectangle of the window on the layer
        */
        MMSFBRectangle getRealGeometry();

        //! Add a widget to the window.
        /*!
        \param child	pointer to a widget
        */
        void add(MMSWidget *child);

        //! Remove a widget from the window.
        /*!
        \param child	pointer to a widget
        */
        void remove(MMSWidget *child);

        //! Get the currently focused widget.
        /*!
        \return pointer to the focused widget
        */
        MMSWidget *getFocusedWidget();

        //! Get the number of focusable widgets.
        /*!
        \param cw	go recursive through child windows if set to true
        \return number of focusable widgets
        */
        int getNumberOfFocusableWidgets(bool cw = false);

        //! Get the number of focusable child windows.
        /*!
        \return number of focusable child windows
        */
        int getNumberOfFocusableChildWins();

        //! Refresh (redraw) the whole window or a part of it.
        /*!
        It is possible to update window attributes without refresh.
        In this case you have to refresh() the window 'manually' to make the changes visible.
        \param region	region of the window which is to refresh, default NULL means the whole window

        For example you can call:

          a) setOpacity(100) and setBgColor(bgcolor)

         or

          b) setOpacity(100, false), setBgColor(bgcolor, false) and then refresh()

        With variant b) you have a better performance because only one refresh will be done.

        This works also for widgets. You can update a few widgets without direct refresh
        and call window->refresh() afterwards.
        */
        void refresh(MMSFBRegion *region = NULL);

        //! Flip the surface of the window to make changes visible.
        /*!
        \return true if successfully flipped
        \note You have to call this method ONLY, if you are drawing own things direct on the window surface!
        \note Please do NOT use this method within the 'normal' GUI context! There is NO need!
        \note If you want to flip, you can also use the flip() method of the MMSFBSurface. But this window flip method handles also childwindows.
        */
        bool flip(void);

        //! Get access to the layer of the window.
        /*!
        \return pointer to the MMSFBLayer object
        */
        MMSFBLayer *getLayer();

        //! Get access to the surface of the window needed to display video streams.
        /*!
        \return pointer to the surface class
        */
        MMSFBSurface *getSurface();

        //! Get the parent window.
        /*!
        \param toplevel		if true the toplevel parent will be returned
        \return pointer to the parent window or NULL if the window has no parent
        */
        MMSWindow *getParent(bool toplevel = false);

        //! Set the window manager.
        /*!
        \param wm	interface to the window manager
        */
        void setWindowManager(IMMSWindowManager *wm);

        //! Is the window shown?
        /*!
        \param checkparents		if true the parent(s) will be check too
        \param checkopacity		if true the opacity of window(s) will be check too
        \return true, if the window is shown
        */
        bool isShown(bool checkparents = false, bool checkopacity = false);

        //! Is the hide action running?
        /*!
        \return true, if the window action thread is going to hide the window
        */
        bool willHide();

        //! Set the focus to this window.
        void setFocus();

        //! Is the window focused?
        /*!
        \param checkparents		if true the parent(s) will be check too
        \return true, if the window is focused
        */
        bool getFocus(bool checkparents = false);

        //! Find a widget with a given name.
        /*!
        \param name		name of the widget
        \return pointer to the widget which was found or NULL
        */
        MMSWidget* findWidget(string name);

        //! Find a widget with a given type.
        /*!
        \param type		type of the widget
        \return pointer to the widget which was found or NULL
        */
        MMSWidget* findWidgetType(MMSWIDGETTYPE type);

        //! Find a widget with a given name and type.
        /*!
        Find a root widget with the given name. If the found widget has not the
        given type, then it searches for the given type under the root widget node.
        \param name		name of the widget
        \param type		type of the widget
        \return pointer to the widget which was found or NULL
        */
        MMSWidget* findWidgetAndType(string name, MMSWIDGETTYPE type);

        //! Operator [] which you can use to find a widget.
        /*!
        \param name		name of the widget
        \return pointer to the widget which was found or NULL
        \see findWidget()
        */
        MMSWidget* operator[](string name);

        //! Get the window to which the GUI navigates if the user press up.
        /*!
        \return pointer to the window or NULL
        */
        MMSWindow *getNavigateUpWindow();

        //! Get the window to which the GUI navigates if the user press down.
        /*!
        \return pointer to the window or NULL
        */
        MMSWindow *getNavigateDownWindow();

        //! Get the window to which the GUI navigates if the user press left.
        /*!
        \return pointer to the window or NULL
        */
        MMSWindow *getNavigateLeftWindow();

        //! Get the window to which the GUI navigates if the user press right.
        /*!
        \return pointer to the window or NULL
        */
        MMSWindow *getNavigateRightWindow();

        //! Set the window to which the GUI navigates if the user press up.
        /*!
        \param upWindow		pointer to the window or NULL
        */
        void setNavigateUpWindow(MMSWindow *upWindow);

        //! Set the window to which the GUI navigates if the user press down.
        /*!
        \param downWindow	pointer to the window or NULL
        */
        void setNavigateDownWindow(MMSWindow *downWindow);

        //! Set the window to which the GUI navigates if the user press left.
        /*!
        \param leftWindow	pointer to the window or NULL
        */
        void setNavigateLeftWindow(MMSWindow *leftWindow);

        //! Set the window to which the GUI navigates if the user press right.
        /*!
        \param rightWindow	pointer to the window or NULL
        */
        void setNavigateRightWindow(MMSWindow *rightWindow);



        bool raiseToTop(int zlevel = 0);
        bool lowerToBottom();

        bool moveTo(int x, int y, bool refresh = true);




        //! Stretch the window to the layer or the parent window using stretchBlit().
        /*!
        \param left		stretch the window to X percent of the window WIDTH to the left side,
						a value of 100 means 100%
        \param up		stretch the window to X percent of the window HEIGHT to the up side,
						a value of 100 means 100%
        \param right	stretch the window to X percent of the window WIDTH to the right side,
						a value of 100 means 100%
        \param down		stretch the window to X percent of the window HEIGHT to the down side,
						a value of 100 means 100%
        \return true, if the values are accepted
        \note Call stretch() or stretch(100, 100, 100, 100) to switch-of the stretch mode.
        \note The formulas (((left-100)+(right-100)+100) > 0) and (((up-100)+(down-100)+100) > 0)
              need to be met! Else nothing is to be displayed!
        \note This function was implemented to get cool effects. If you display windows permanently in
              stretch mode the performance will be drastically decreased.
        */
        bool stretch(double left = 100, double up = 100, double right = 100, double down = 100);



        //! Set one or more callbacks for the onBeforeShow event.
        /*!
        The connected callbacks will be called during show().
        If at least one of the callbacks returns false, the show process of the window
        will be stopped and the window will not appear.

        A callback method must be defined like this:

        	bool myclass::mycallbackmethod(MMSWindow *win);

        	\param win	is the pointer to the window which is to be shown

        	\return true if the show process should continue, else false if the window should not appear

        To connect your callback to onBeforeShow do this:

            sigc::connection connection;
            connection = mywindow->onBeforeShow->connect(sigc::mem_fun(myobject,&myclass::mycallbackmethod));

        To disconnect your callback do this:

            connection.disconnect();

        Please note:

            You HAVE TO disconnect myobject from onBeforeShow BEFORE myobject will be deleted!!!
            Else an abnormal program termination can occur.
            You HAVE TO call the disconnect() method of sigc::connection explicitly. The destructor will NOT do this!!!
        */
        sigc::signal<bool, MMSWindow*>::accumulated<bool_accumulator> *onBeforeShow;

        //! Set one or more callbacks for the onAfterShow event.
        /*!
        The connected callbacks will be called during show().

        A callback method must be defined like this:

        	void myclass::mycallbackmethod(MMSWindow *win, bool already_shown);

        	\param win				is the pointer to the window which is shown now
       		\param already_shown	the window was already shown?

        To connect your callback to onAfterShow do this:

            sigc::connection connection;
            connection = mywindow->onAfterShow->connect(sigc::mem_fun(myobject,&myclass::mycallbackmethod));

        To disconnect your callback do this:

            connection.disconnect();

        Please note:

            You HAVE TO disconnect myobject from onAfterShow BEFORE myobject will be deleted!!!
            Else an abnormal program termination can occur.
            You HAVE TO call the disconnect() method of sigc::connection explicitly. The destructor will NOT do this!!!
        */
        sigc::signal<void, MMSWindow*, bool> *onAfterShow;

        //! Set one or more callbacks for the onBeforeHide event.
        /*!
        The connected callbacks will be called during hide().
        If at least one of the callbacks returns false, the hide process of the window
        will be stopped and the window will not disappear.

        A callback method must be defined like this:

        	bool myclass::mycallbackmethod(MMSWindow *win, bool goback);

        	\param win		is the pointer to the window which is to be hidden
       		\param goback	the application can decide, what to do if true/false, see hide()

        	\return true if the hide process should continue, else false if the window should not disappear

        To connect your callback to onBeforeHide do this:

            sigc::connection connection;
            connection = mywindow->onBeforeHide->connect(sigc::mem_fun(myobject,&myclass::mycallbackmethod));

        To disconnect your callback do this:

            connection.disconnect();

        Please note:

            You HAVE TO disconnect myobject from onBeforeHide BEFORE myobject will be deleted!!!
            Else an abnormal program termination can occur.
            You HAVE TO call the disconnect() method of sigc::connection explicitly. The destructor will NOT do this!!!
        */
        sigc::signal<bool, MMSWindow*, bool>::accumulated<bool_accumulator> *onBeforeHide;

        //! Set one or more callbacks for the onHide event.
        /*!
        The connected callbacks will be called during hide().

        A callback method must be defined like this:

        	void myclass::mycallbackmethod(MMSWindow *win, bool goback);

        	\param win		is the pointer to the window which is hidden now
       		\param goback	the application can decide, what to do if true/false, see hide()

        To connect your callback to onHide do this:

            sigc::connection connection;
            connection = mywindow->onHide->connect(sigc::mem_fun(myobject,&myclass::mycallbackmethod));

        To disconnect your callback do this:

            connection.disconnect();

        Please note:

            You HAVE TO disconnect myobject from onHide BEFORE myobject will be deleted!!!
            Else an abnormal program termination can occur.
            You HAVE TO call the disconnect() method of sigc::connection explicitly. The destructor will NOT do this!!!
        */
        sigc::signal<void, MMSWindow*, bool> *onHide;

        //! Set one or more callbacks for the onHandleInput event.
        /*!
        The connected callbacks will be called if an input event was raised.

        A callback method must be defined like this:

        	bool myclass::mycallbackmethod(MMSWindow *win, MMSInputEvent *inputevent);

        	\param win			is the pointer to the window
       		\param inputevent	the input event

        	\return callback should return true if the input was handled, else false

        To connect your callback to onHandleInput do this:

            sigc::connection connection;
            connection = mywindow->onHandleInput->connect(sigc::mem_fun(myobject,&myclass::mycallbackmethod));

        To disconnect your callback do this:

            connection.disconnect();

        Please note:

            You HAVE TO disconnect myobject from onHandleInput BEFORE myobject will be deleted!!!
            Else an abnormal program termination can occur.
            You HAVE TO call the disconnect() method of sigc::connection explicitly. The destructor will NOT do this!!!
        */
        sigc::signal<bool, MMSWindow*, MMSInputEvent*>::accumulated<neg_bool_accumulator> *onHandleInput;

        //! Set one or more callbacks for the onBeforeHandleInput event.
        /*!
        The connected callbacks will be called if an input event was raised, before any standard operation took place

        A callback method must be defined like this:

        	bool myclass::mycallbackmethod(MMSWindow *win, MMSInputEvent *inputevent);

        	\param win			is the pointer to the window
       		\param inputevent	the input event

        	\return callback should return true if the input was handled, else false

        To connect your callback to onHandleInput do this:

            sigc::connection connection;
            connection = mywindow->onHandleInput->connect(sigc::mem_fun(myobject,&myclass::mycallbackmethod));

        To disconnect your callback do this:

            connection.disconnect();

        Please note:

            You HAVE TO disconnect myobject from onHandleInput BEFORE myobject will be deleted!!!
            Else an abnormal program termination can occur.
            You HAVE TO call the disconnect() method of sigc::connection explicitly. The destructor will NOT do this!!!
        */
        sigc::signal<bool, MMSWindow*, MMSInputEvent*>::accumulated<neg_bool_accumulator> *onBeforeHandleInput;


        //! Set one or more callbacks for the onDraw event.
        /*!
        The connected callbacks will be called if the window will be drawn.

        A callback method must be defined like this:

        	bool myclass::mycallbackmethod(MMSFBSurface *surface, bool clear);

        	\param surface		is the pointer to window's surface
        	\param clear		if true, the callback should clear the surface before drawing

        	\return callback should return true if it has drawn to the surface, else false

        To connect your callback to onDraw do this:

            sigc::connection connection;
            connection = mywindow->onDraw->connect(sigc::mem_fun(myobject,&myclass::mycallbackmethod));

        To disconnect your callback do this:

            connection.disconnect();

        Please note:

            You HAVE TO disconnect myobject from onDraw BEFORE myobject will be deleted!!!
            Else an abnormal program termination can occur.
            You HAVE TO call the disconnect() method of sigc::connection explicitly. The destructor will NOT do this!!!
        */
        sigc::signal<bool, MMSFBSurface*, bool>::accumulated<neg_bool_accumulator> *onDraw;








        unsigned int printStack(char *buffer, int space = 0);

    public:
        /* begin of theme access methods *************************************/

        //! Get the alignment of the window.
        /*!
        \param alignment	returns the alignment
        \return true, if alignment is successfully returned
        */
    	bool getAlignment(MMSALIGNMENT &alignment);

        //! Get the x-movement of the window.
        /*!
        \param dx	returns the x-movement
        \return true, if dx is successfully returned
        */
    	bool getDx(string &dx);

        //! Get the x-movement of the window as integer in pixel.
        /*!
        \return the x-movement
        */
    	int getDxPix();

        //! Get the y-movement of the window.
        /*!
        \param dy	returns the y-movement
        \return true, if dy is successfully returned
        */
        bool getDy(string &dy);

        //! Get the y-movement of the window as integer in pixel.
        /*!
        \return the y-movement
        */
        int getDyPix();

        //! Get the width of the window.
        /*!
        \param width	returns the width
        \return true, if width is successfully returned
        */
        bool getWidth(string &width);

        //! Get the height of the window.
        /*!
        \param height	returns the height
        \return true, if height is successfully returned
        */
        bool getHeight(string &height);

        //! Get the background color of the window.
        /*!
        \param bgcolor	returns the bgcolor
        \return true, if bgcolor is successfully returned
        */
        bool getBgColor(MMSFBColor &bgcolor);

        //! Get the path to the background image of the window.
        /*!
        \param bgimagepath	returns the path
        \return true, if bgimagepath is successfully returned
        */
        bool getBgImagePath(string &bgimagepath);

        //! Get the name of the background image.
        /*!
        \param bgimagename	returns the name
        \return true, if bgimagename is successfully returned
        */
        bool getBgImageName(string &bgimagename);

        //! Get the opacity of the window.
        /*!
        \param opacity	returns the opacity
        \return true, if opacity is successfully returned
        */
        bool getOpacity(unsigned int &opacity);

        //! Detect if the window has to fade-in during the show action.
        /*!
        \param fadein	returns the fadein status
        \return true, if fadein is successfully returned
        */
        bool getFadeIn(bool &fadein);

        //! Detect if the window has to fade-out during the hide action.
        /*!
        \param fadeout	returns the fadeout status
        \return true, if fadeout is successfully returned
        */
        bool getFadeOut(bool &fadeout);

        //! Detect if the window has to draw debug rectangles around its widgets.
        /*!
        \param debug	returns the debug status
        \return true, if debug is successfully returned
        */
        bool getDebug(bool &debug);

        //! Get the margin of the window.
        /*!
		The margin of a window is the space around the window border.
		If you have for example a 640x480 window with an margin 10, the window content will be drawed within 620x460.
        \param margin	returns the margin
        \return true, if margin is successfully returned
        */
        bool getMargin(unsigned int &margin);

        //! Get the name of the uparrow widget.
        /*!
        If the user can navigate up to a other widget or child window,
        the uparrow widget will be selected, else unselected.
        \param uparrow	returns the name
        \return true, if uparrow is successfully returned
        */
        bool getUpArrow(string &uparrow);

        //! Get the name of the downarrow widget.
        /*!
        If the user can navigate down to a other widget or child window,
        the downarrow widget will be selected, else unselected.
        \param downarrow	returns the name
        \return true, if downarrow is successfully returned
        */
        bool getDownArrow(string &downarrow);

        //! Get the name of the leftarrow widget.
        /*!
        If the user can navigate left to a other widget or child window,
        the leftarrow widget will be selected, else unselected.
        \param leftarrow	returns the name
        \return true, if leftarrow is successfully returned
        */
        bool getLeftArrow(string &leftarrow);

        //! Get the name of the rightarrow widget.
        /*!
        If the user can navigate right to a other widget or child window,
        the rightarrow widget will be selected, else unselected.
        \param rightarrow	returns the name
        \return true, if rightarrow is successfully returned
        */
        bool getRightArrow(string &rightarrow);

        //! Get the name of the navigateup window.
        /*!
        This is the child window which is to be focused if user navigates up.
        If it is not set, the GUI is searching automatically for the best route.
        \param navigateup	returns the name
        \return true, if navigateup is successfully returned
        */
        bool getNavigateUp(string &navigateup);

        //! Get the name of the navigatedown window.
        /*!
        This is the child window which is to be focused if user navigates down.
        If it is not set, the GUI is searching automatically for the best route.
        \param navigatedown	returns the name
        \return true, if navigatedown is successfully returned
        */
        bool getNavigateDown(string &navigatedown);

        //! Get the name of the navigateleft window.
        /*!
        This is the child window which is to be focused if user navigates left.
        If it is not set, the GUI is searching automatically for the best route.
        \param navigateleft	returns the name
        \return true, if navigateleft is successfully returned
        */
        bool getNavigateLeft(string &navigateleft);

        //! Get the name of the navigateright window.
        /*!
        This is the child window which is to be focused if user navigates right.
        If it is not set, the GUI is searching automatically for the best route.
        \param navigateright	returns the name
        \return true, if navigateright is successfully returned
        */
        bool getNavigateRight(string &navigateright);

        //! Detect if the window has an own surface.
        /*!
        If the window has no own surface, the drawing functions will be directly work on the parent surface.
        This can save a lot of memory.
        \param ownsurface	returns the ownsurface status
        \return true, if ownsurface is successfully returned
        */
        bool getOwnSurface(bool &ownsurface);

        //! Get the move-in direction of the window which is used during the show action.
        /*!
        \param movein	returns the movein direction
        \return true, if movein is successfully returned
        */
        bool getMoveIn(MMSDIRECTION &movein);

        //! Get the move-out direction of the window which is used during the hide action.
        /*!
        \param moveout	returns the moveout direction
        \return true, if moveout is successfully returned
        */
        bool getMoveOut(MMSDIRECTION &moveout);

    	//! Detect if the window has to be modal if it is focused.
        /*!
        \param modal	returns the modal status
        \return true, if modal is successfully returned
        */
        bool getModal(bool &modal);

    	//! Detect if the window automatically changes the zorder of child windows during show() or setFocus()
        /*!
        \param staticzorder	returns the static zorder flag
        \return true, if value is successfully returned
        */
        bool getStaticZOrder(bool &staticzorder);

    	//! Detect if the window will be permanently displayed at the top of the window stack.
        /*!
        \param alwaysontop	returns the always on top flag
        \return true, if value is successfully returned
        */
        bool getAlwaysOnTop(bool &alwaysontop);

    	//! Detect if the window can be focused.
        /*!
        \param focusable	returns the focusable flag
        \return true, if value is successfully returned
        */
        bool getFocusable(bool &focusable);

    	//! Detect if the window has an backbuffer.
        /*!
        \param backbuffer	returns the backbuffer flag
        \return true, if value is successfully returned
        */
        bool getBackBuffer(bool &backbuffer);

    	//! Detect if the window is loading images, fonts etc. during initialization.
        /*!
        \param initialload	returns the initial load flag
        \return true, if value is successfully returned
        \note Per default initial load is false. That means that the window and it's widgets
              will load images, fonts etc. during the first show().
        */
        bool getInitialLoad(bool &initialload);

        //! Get the color of the window border.
        /*!
        \param color	returns the border color
        \return true, if color is successfully returned
        */
        bool getBorderColor(MMSFBColor &color);

        //! Get the path to the window border images.
        /*!
        \param imagepath	returns the path
        \return true, if imagepath is successfully returned
        */
        bool getBorderImagePath(string &imagepath);

        //! Get the name of a border image.
        /*!
        \param num			number of image, see MMSBORDER_IMAGE_NUM
        \param imagename	returns the name
        \return true, if imagename is successfully returned
        */
        bool getBorderImageNames(MMSBORDER_IMAGE_NUM num, string &imagename);

        //! Get the border thickness.
        /*!
        \param thickness	returns the border thickness
        \return true, if thickness is successfully returned
        */
        bool getBorderThickness(unsigned int &thickness);

        //! Get the border margin.
        /*!
        The border margin is the space between the border an the inner rectangle of the window on which the
        widgets or child windows will be drawn.
        \param margin	returns the border margin
        \return true, if margin is successfully returned
        */
        bool getBorderMargin(unsigned int &margin);

        //! Detect if the border has round corners.
        /*!
        \param rcorners	returns the round corners status
        \return true, if rcorners is successfully returned
        */
        bool getBorderRCorners(bool &rcorners);


        //! Set the alignment of the window.
        /*!
        \param alignment	the alignment
        \param refresh		refresh the window after changing the alignment?
        \param resize		resize the window after changing the alignment?
        */
        void setAlignment(MMSALIGNMENT alignment, bool refresh = true, bool resize = true);

        //! Set the x-movement of the window.
        /*!
        \param dx		the x-movement
        \param refresh	refresh the window after changing the dx?
        \param resize	resize the window after changing the dx?
        */
        void setDx(string dx, bool refresh = false, bool resize = true);

        //! Set the x-movement of the window as integer in pixel.
        /*!
        \param dx		the x-movement in pixel
        \param refresh	refresh the window after changing the dx?
        \param resize	resize the window after changing the dx?
        */
        void setDxPix(int dx, bool refresh = false, bool resize = true);

        //! Set the y-movement of the window.
        /*!
        \param dy		the y-movement
        \param refresh	refresh the window after changing the dy?
        \param resize	resize the window after changing the dy?
        */
        void setDy(string dy, bool refresh = false, bool resize = true);

        //! Set the y-movement of the window as integer in pixel.
        /*!
        \param dy		the y-movement in pixel
        \param refresh	refresh the window after changing the dy?
        \param resize	resize the window after changing the dy?
        */
        void setDyPix(int dy, bool refresh = false, bool resize = true);

        //! Set the width of the window.
        /*!
        \param width	the width
        \param refresh	refresh the window after changing the width?
        \param resize	resize the window after changing the width?
        */
        void setWidth(string width, bool refresh = true, bool resize = true);

        //! Set the height of the window.
        /*!
        \param height	the height
        \param refresh	refresh the window after changing the height?
        \param resize	resize the window after changing the height?
        */
        void setHeight(string height, bool refresh = true, bool resize = true);

        //! Set the background color of the window.
        /*!
        \param bgcolor	the bgcolor
        \param refresh	refresh the window after changing the bgcolor?
        */
        void setBgColor(MMSFBColor bgcolor, bool refresh = true);

        //! Set the path to the background image of the window.
        /*!
        \param bgimagepath	the path
        \param load			reload the bgimage after changing the bgimagepath?
        \param refresh		refresh the window after changing the bgimagepath?
        */
        void setBgImagePath(string bgimagepath, bool load = true, bool refresh = true);

        //! Set the name of the background image.
        /*!
        \param bgimagename	the name
        \param load			reload the bgimage after changing the bgimagename?
        \param refresh		refresh the window after changing the bgimagename?
        */
        void setBgImageName(string bgimagename, bool load = true, bool refresh = true);

        //! Set background image already loaded by the application
        /*!
        \param bgimage		pointer to surface or NULL
        \param refresh		refresh the window after changing?
        */
        void setBgImage(MMSFBSurface *bgimage, bool refresh = true);

        //! Set the opacity of the window.
        /*!
        \param opacity	the opacity
        \param refresh	refresh the window after changing the opacity?
        */
        void setOpacity(unsigned int opacity, bool refresh = true);

        //! Set the fadein status needed during the show action.
        /*!
        \param fadein	if true, the window is fading in during the next show()
        */
        void setFadeIn(bool fadein);

        //! Set the fadeout status needed during the hide action.
        /*!
        \param fadeout	if true, the window is fading out during the next hide()
        */
        void setFadeOut(bool fadeout);

        //! Should the window draw debug rectangles around its widgets?
        /*!
        \param debug	the debug status
        \param refresh	refresh the window after changing the debug status?
        */
        void setDebug(bool debug, bool refresh = true);

        //! Set the margin of the window.
        /*!
		The margin of a window is the space around the window border.
		If you have for example a 640x480 window with an margin 10, the window content will be drawed within 620x460.
        \param margin	the margin
        \param refresh	refresh the window after changing the margin?
        \param resize	resize the window after changing the margin?
        */
        void setMargin(unsigned int margin, bool refresh = true, bool resize = true);

        //! Set the name of the uparrow widget.
        /*!
        If the user can navigate up to a other widget or child window,
        the uparrow widget will be selected, else unselected.
        \param uparrow	the name
        \param refresh	refresh the window after changing the uparrow?
        */
        void setUpArrow(string uparrow, bool refresh = true);

        //! Set the name of the downarrow widget.
        /*!
        If the user can navigate down to a other widget or child window,
        the downarrow widget will be selected, else unselected.
        \param downarrow	the name
        \param refresh		refresh the window after changing the downarrow?
        */
        void setDownArrow(string downarrow, bool refresh = true);

        //! Set the name of the leftarrow widget.
        /*!
        If the user can navigate left to a other widget or child window,
        the leftarrow widget will be selected, else unselected.
        \param leftarrow	the name
        \param refresh		refresh the window after changing the leftarrow?
        */
        void setLeftArrow(string leftarrow, bool refresh = true);

        //! Set the name of the rightarrow widget.
        /*!
        If the user can navigate right to a other widget or child window,
        the rightarrow widget will be selected, else unselected.
        \param rightarrow	the name
        \param refresh		refresh the window after changing the rightarrow?
        */
        void setRightArrow(string rightarrow, bool refresh = true);

        //! Set the name of the navigateup window.
        /*!
        This is the child window which is to be focused if user navigates up.
        If it is not set, the GUI is searching automatically for the best route.
        \param navigateup	the name
        */
        void setNavigateUp(string navigateup);

        //! Set the name of the navigatedown window.
        /*!
        This is the child window which is to be focused if user navigates down.
        If it is not set, the GUI is searching automatically for the best route.
        \param navigatedown	the name
        */
        void setNavigateDown(string navigatedown);

        //! Set the name of the navigateleft window.
        /*!
        This is the child window which is to be focused if user navigates left.
        If it is not set, the GUI is searching automatically for the best route.
        \param navigateleft	the name
        */
        void setNavigateLeft(string navigateleft);

        //! Set the name of the navigateright window.
        /*!
        This is the child window which is to be focused if user navigates right.
        If it is not set, the GUI is searching automatically for the best route.
        \param navigateright	the name
        */
        void setNavigateRight(string navigateright);

        //! Should the window working with an own surface?
        /*!
        If the window has no own surface, the drawing functions will be directly work on the parent surface.
        This can save a lot of memory.
        \param ownsurface	the ownsurface status
        */
        void setOwnSurface(bool ownsurface);

        //! Set the move-in direction of the window which is used during the show action.
        /*!
        \param movein	the movein direction
        */
        void setMoveIn(MMSDIRECTION movein);

        //! Set the move-out direction of the window which is used during the hide action.
        /*!
        \param moveout	the moveout direction
        */
        void setMoveOut(MMSDIRECTION moveout);

        //! Set the modal status of the window.
        /*!
        \param modal	if true, the window is marked as modal
        */
        void setModal(bool modal);

        //! Set the zorder status of the window.
        /*!
        \param staticzorder	if true, the window automatically changes the zorder of child window during show() / setFocus()
        */
        void setStaticZOrder(bool staticzorder);

        //! Set the always on top flag of the window.
        /*!
        \param alwaysontop	if true, the window will be permanently displayed at the top of the window stack
        */
        void setAlwaysOnTop(bool alwaysontop);

        //! Set the focusable flag of the window.
        /*!
        \param focusable	if true, the window will can get the focus
        */
        void setFocusable(bool focusable);

        //! Set the backbuffer flag of the window.
        /*!
        \param backbuffer	if true, the window surface has a front and a backbuffer
        */
        void setBackBuffer(bool backbuffer);

        //! Set the initial load flag of the window.
        /*!
        \param initialload	if true, the window is loading images, fonts etc. during initialization
        */
        void setInitialLoad(bool initialload);

        //! Set the color of the window border.
        /*!
        \param color	the border color
        \param refresh	refresh the window after changing the border color?
        */
        void setBorderColor(MMSFBColor color, bool refresh = true);

        //! Set the path to the window border images.
        /*!
        \param imagepath	returns the path
        \param load			reload the border images after changing the imagepath?
        \param refresh		refresh the window after changing the imagepath?
        */
        void setBorderImagePath(string imagepath, bool load = true, bool refresh = true);

        //! Set the names of the window border images.
        /*!
        \param imagename_1	the top-left image
        \param imagename_2	the top image
        \param imagename_3	the top-right image
        \param imagename_4	the right image
        \param imagename_5	the bottom-right image
        \param imagename_6	the bottom image
        \param imagename_7	the bottom-left imagethis->focusedChildWin
        \param imagename_8	the left image
        \param load			reload the border images after changing the image names?
        \param refresh		refresh the window after changing the image names?
        */
        void setBorderImageNames(string imagename_1, string imagename_2, string imagename_3, string imagename_4,
                                 string imagename_5, string imagename_6, string imagename_7, string imagename_8,
                                 bool load = true, bool refresh = true);

        //! Set the border thickness.
        /*!
        \param thickness	the border thickness
        \param refresh		refresh the window after changing the thickness?
        \param resize		resize the window after changing the thickness?
        */
        void setBorderThickness(unsigned int thickness, bool refresh = true, bool resize = true);

        //! Set the border margin.
        /*!
        The border margin is the space between the border an the inner rectangle of the window on which the
        widgets or child windows will be drawn.
        \param margin		the border margin
        \param refresh		refresh the window after changing the margin?
        \param resize		resize the window after changing the margin?
        */
        void setBorderMargin(unsigned int margin, bool refresh = true, bool resize = true);

        //! Should the window border uses round corners?
        /*!
        \param rcorners	the round corners status
        \param refresh	refresh the window after changing the rcorners?
        */
        void setBorderRCorners(bool rcorners, bool refresh = true);

        //! Update the current window settings with settings from another theme class.
        /*!
        \param themeClass	source theme class
        \note Only this parameters which are set within the source theme class will be updated.
        */
        void updateFromThemeClass(MMSWindowClass *themeClass);

    // friends
	friend class MMSWindowManager;
    friend class MMSInputManager;
    friend class MMSWindowAction;
    friend class MMSMainWindow;
    friend class MMSPopupWindow;
    friend class MMSRootWindow;
    friend class MMSChildWindow;
    friend class MMSWidget;
    friend class MMSHBoxWidget;
    friend class MMSVBoxWidget;
    friend class MMSLabelWidget;
    friend class MMSButtonWidget;
    friend class MMSImageWidget;
    friend class MMSProgressBarWidget;
    friend class MMSMenuWidget;
    friend class MMSTextBoxWidget;
    friend class MMSArrowWidget;
    friend class MMSSliderWidget;
    friend class MMSInputWidget;
    friend class MMSCheckBoxWidget;
};

#endif /*MMSWINDOW_H_*/
