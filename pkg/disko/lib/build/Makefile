PKGDIR		?= ../../
L4DIR		?= $(PKGDIR)/../..

PC_FILENAME	= libdisko
TARGET		= libdisko.a
# libdisko.so

LIBS		= $(GCCLIB)
REQUIRES_LIBS	= libstdc++ libpng freetype2 libsigc++ libxml2 libdl libpthread libc_minimal_l4re libc_be_math \
	libc_be_file_stdin libc_be_l4refile libc_be_l4re l4re l4re_c libwhefs \
	input


PRIVATE_INCDIR	= $(PKGDIR)/include $(PKGDIR_ABS)/../freetype2/include \
	$(PKGDIR_ABS)/../libsigc++/include $(PKGDIR_ABS)/../libxml2/include \
	$(PKGDIR_ABS)/../libwhefs/include

DEFINES		=  -D__HAVE_PNG__ -D__HAVE_DL__ -D__L4_RE__ -D__HAVE_L4_FB__ -D__HAVE_PF_ALL__ -DMMSLOGGER_STDOUT_ONLY -D__ARM__
WARNINGS	= -Wno-unused

CXXFLAGS	+= -fexceptions
LDFLAGS		=  --warn-common --no-undefined

SRC_CC 	= \
	src/mmsinfo/mmsinfo.cc \
	\
	src/mmstools/datasource.cc \
	src/mmstools/mmsdbconnmgr.cc \
	src/mmstools/mmserror.cc \
	src/mmstools/mmsfile.cc \
	src/mmstools/mmsfilesearch.cc \
	src/mmstools/mmsidfactory.cc \
	src/mmstools/mmslogbackend.cc \
	src/mmstools/mmslogger.cc \
	src/mmstools/mmsmutex.cc \
	src/mmstools/mmspulser.cc \
	src/mmstools/mmsrecordset.cc \
	src/mmstools/mmsserverinterface.cc \
	src/mmstools/mmsshlhandler.cc \
	src/mmstools/mmstafffile.cc \
	src/mmstools/mmsthread.cc \
	src/mmstools/mmsthreadserver.cc \
	src/mmstools/mmstimer.cc \
	src/mmstools/mmstypes.cc \
	src/mmstools/tools.cc \
	\
	src/mmsconfig/mmsconfigdata.cc \
	src/mmsconfig/mmsimportpropertydao.cc \
	src/mmsconfig/mmsimportpropertydata.cc \
	src/mmsconfig/mmsimportpropertyservice.cc \
	src/mmsconfig/mmsimportsourcedao.cc \
	src/mmsconfig/mmsimportsourcedata.cc \
	src/mmsconfig/mmsimportsourceservice.cc \
	src/mmsconfig/mmsplugincategorydao.cc \
	src/mmsconfig/mmsplugincategorydata.cc \
	src/mmsconfig/mmsplugindao.cc \
	src/mmsconfig/mmsplugindata.cc \
	src/mmsconfig/mmspluginparameterparser.cc \
	src/mmsconfig/mmspluginpropertydao.cc \
	src/mmsconfig/mmspluginservice.cc \
	src/mmsconfig/mmsplugintypedao.cc \
	src/mmsconfig/mmsplugintypedata.cc \
	src/mmsconfig/mmspropertydata.cc \
	src/mmsconfig/mmsrcparser.cc \
	\
	src/mmsgui/mmscheckboxwidget.cc \
	src/mmsgui/mmsfontmanager.cc \
	src/mmsgui/mmsdialogmanager.cc \
	src/mmsgui/mmsprogressbarwidget.cc \
	src/mmsgui/mmswindow.cc \
	src/mmsgui/mmsarrowwidget.cc \
	src/mmsgui/mmsmenuwidget.cc \
	src/mmsgui/mmsmainwindow.cc \
	src/mmsgui/mmstextboxwidget.cc \
	src/mmsgui/mmsbuttonwidget.cc \
	src/mmsgui/mmswidget.cc \
	src/mmsgui/mmswindowaction.cc \
	src/mmsgui/mmsvboxwidget.cc \
	src/mmsgui/mmsimagemanager.cc \
	src/mmsgui/mmsfbmanager.cc \
	src/mmsgui/mmslabelwidget.cc \
	src/mmsgui/mmsgifloader.cc \
	src/mmsgui/mmspopupwindow.cc \
	src/mmsgui/mmshboxwidget.cc \
	src/mmsgui/mmssliderwidget.cc \
	src/mmsgui/mmsrootwindow.cc \
	src/mmsgui/mmsguitools.cc \
	src/mmsgui/mmswindowmanager.cc \
	src/mmsgui/mmslabelwidgetthread.cc \
	src/mmsgui/mmsgapwidget.cc \
	src/mmsgui/mmsborder.cc \
	src/mmsgui/mmsinputwidget.cc \
	src/mmsgui/mmsimagewidgetthread.cc \
	src/mmsgui/mmsinputwidgetthread.cc \
	src/mmsgui/mmsimagewidget.cc \
	src/mmsgui/mmschildwindow.cc \
	\
	src/mmsgui/additional/mmsfiledialog.cc \
	src/mmsgui/additional/mmsinputcontrol.cc \
	src/mmsgui/additional/mmsguicontrol.cc \
	\
	src/mmsgui/theme/mmsdescriptionclass.cc \
	src/mmsgui/theme/mmschildwindowclass.cc \
	src/mmsgui/theme/mmstextboxwidgetclass.cc \
	src/mmsgui/theme/mmsmenuwidgetclass.cc \
	src/mmsgui/theme/mmsthememanager.cc \
	src/mmsgui/theme/mmsbuttonwidgetclass.cc \
	src/mmsgui/theme/mmswidgetclass.cc \
	src/mmsgui/theme/mmswindowclass.cc \
	src/mmsgui/theme/mmsarrowwidgetclass.cc \
	src/mmsgui/theme/mmsthemeclass.cc \
	src/mmsgui/theme/mmsmainwindowclass.cc \
	src/mmsgui/theme/mmscheckboxwidgetclass.cc \
	src/mmsgui/theme/mmsimagewidgetclass.cc \
	src/mmsgui/theme/mmsthemebase.cc \
	src/mmsgui/theme/mmslabelwidgetclass.cc \
	src/mmsgui/theme/mmspopupwindowclass.cc \
	src/mmsgui/theme/mmstemplateclass.cc \
	src/mmsgui/theme/mmssliderwidgetclass.cc \
	src/mmsgui/theme/mmstextbaseclass.cc \
	src/mmsgui/theme/mmstheme.cc \
	src/mmsgui/theme/mmsprogressbarwidgetclass.cc \
	src/mmsgui/theme/mmsborderclass.cc \
	src/mmsgui/theme/mmsinputwidgetclass.cc \
	src/mmsgui/theme/mmsrootwindowclass.cc \
	\
	src/mmsgui/fb/stretchblit/mmsfb_stretchblit_yuy2_to_yv12.cc \
	src/mmsgui/fb/stretchblit/mmsfb_stretchblit_blend_coloralpha_airgb_to_airgb.cc \
	src/mmsgui/fb/stretchblit/mmsfb_stretchblit_blend_airgb_to_airgb.cc \
	src/mmsgui/fb/stretchblit/mmsfb_stretchblit_ayuv_to_ayuv.cc \
	src/mmsgui/fb/stretchblit/mmsfb_stretchblit_blend_argb_to_argb.cc \
	src/mmsgui/fb/stretchblit/mmsfb_stretchblit_rgb24_to_argb.cc \
	src/mmsgui/fb/stretchblit/mmsfb_stretchblit_blend_argb_to_rgb32.cc \
	src/mmsgui/fb/stretchblit/mmsfb_stretchblit_blend_coloralpha_argb4444_to_argb4444.cc \
	src/mmsgui/fb/stretchblit/mmsfb_stretchblit_blend_coloralpha_argb_to_argb.cc \
	src/mmsgui/fb/stretchblit/mmsfb_stretchblit_i420_to_yv12.cc \
	src/mmsgui/fb/stretchblit/mmsfb_stretchblit_rgb16_to_rgb16.cc \
	src/mmsgui/fb/stretchblit/mmsfb_stretchblit_yv12_to_yv12.cc \
	src/mmsgui/fb/stretchblit/mmsfb_stretchblit_blend_coloralpha_ayuv_to_ayuv.cc \
	src/mmsgui/fb/stretchblit/mmsfb_stretchblit_rgb24_to_rgb32.cc \
	src/mmsgui/fb/stretchblit/mmsfb_stretchblit_airgb_to_airgb.cc \
	src/mmsgui/fb/stretchblit/mmsfb_stretchblit_blend_ayuv_to_ayuv.cc \
	src/mmsgui/fb/stretchblit/mmsfb_stretchblit_blend_argb4444_to_argb4444.cc \
	src/mmsgui/fb/stretchblit/mmsfb_stretchblit_argb_to_argb.cc \
	src/mmsgui/fb/stretchblit/mmsfb_stretchblit_rgb32_to_rgb32.cc \
	\
	src/mmsgui/fb/drawstring/mmsfb_drawstring_blend_coloralpha_argb.cc \
	src/mmsgui/fb/drawstring/mmsfb_drawstring_blend_rgb16.cc \
	src/mmsgui/fb/drawstring/mmsfb_drawstring_blend_argb4444.cc \
	src/mmsgui/fb/drawstring/mmsfb_drawstring_blend_coloralpha_rgb16.cc \
	src/mmsgui/fb/drawstring/mmsfb_drawstring_blend_argb.cc \
	\
	src/mmsgui/fb/drawline/mmsfb_drawline_argb.cc \
	src/mmsgui/fb/drawline/mmsfb_drawline_argb4444.cc \
	src/mmsgui/fb/drawline/mmsfb_drawline_blend_argb.cc \
	\
	src/mmsgui/fb/blit/mmsfb_blit_blend_argb_to_airgb.cc \
	src/mmsgui/fb/blit/mmsfb_blit_argb_to_rgb16.cc \
	src/mmsgui/fb/blit/mmsfb_blit_rgb16_to_argb.cc \
	src/mmsgui/fb/blit/mmsfb_blit_yv12_to_yv12.cc \
	src/mmsgui/fb/blit/mmsfb_blit_rgb24_to_rgb24.cc \
	src/mmsgui/fb/blit/mmsfb_blit_blend_coloralpha_argb_to_yv12.cc \
	src/mmsgui/fb/blit/mmsfb_blit_rgb16_to_rgb32.cc \
	src/mmsgui/fb/blit/mmsfb_blit_ayuv_to_ayuv.cc \
	src/mmsgui/fb/blit/mmsfb_blit_blend_argb4444_to_argb4444.cc \
	src/mmsgui/fb/blit/mmsfb_blit_argb_to_rgb32.cc \
	src/mmsgui/fb/blit/mmsfb_blit_blend_coloralpha_airgb_to_airgb.cc \
	src/mmsgui/fb/blit/mmsfb_blit_blend_argb4444_to_rgb32.cc \
	src/mmsgui/fb/blit/mmsfb_blit_i420_to_yv12.cc \
	src/mmsgui/fb/blit/mmsfb_blit_blend_argb_to_argb3565.cc \
	src/mmsgui/fb/blit/mmsfb_blit_argb_to_argb.cc \
	src/mmsgui/fb/blit/mmsfb_blit_airgb_to_airgb.cc \
	src/mmsgui/fb/blit/mmsfb_blit_yuy2_to_yv12.cc \
	src/mmsgui/fb/blit/mmsfb_blit_rgb32_to_rgb32.cc \
	src/mmsgui/fb/blit/mmsfb_blit_argb3565_to_argb3565.cc \
	src/mmsgui/fb/blit/mmsfb_blit_ayuv_to_rgb16.cc \
	src/mmsgui/fb/blit/mmsfb_blit_blend_coloralpha_argb_to_argb.cc \
	src/mmsgui/fb/blit/mmsfb_blit_blend_airgb_to_airgb.cc \
	src/mmsgui/fb/blit/mmsfb_blit_rgb24_to_yv12.cc \
	src/mmsgui/fb/blit/mmsfb_blit_argb_to_yv12.cc \
	src/mmsgui/fb/blit/mmsfb_blit_blend_coloralpha_argb_to_rgb32.cc \
	src/mmsgui/fb/blit/mmsfb_blit_blend_coloralpha_argb4444_to_rgb32.cc \
	src/mmsgui/fb/blit/mmsfb_blit_airgb_to_rgb16.cc \
	src/mmsgui/fb/blit/mmsfb_blit_blend_argb_to_yv12.cc \
	src/mmsgui/fb/blit/mmsfb_blit_blend_airgb_to_rgb16.cc \
	src/mmsgui/fb/blit/mmsfb_blit_blend_argb_to_rgb24.cc \
	src/mmsgui/fb/blit/mmsfb_blit_yv12_to_rgb32.cc \
	src/mmsgui/fb/blit/mmsfb_blit_rgb24_to_rgb32.cc \
	src/mmsgui/fb/blit/mmsfb_blit_blend_argb_to_rgb16.cc \
	src/mmsgui/fb/blit/mmsfb_blit_argb_to_argb3565.cc \
	src/mmsgui/fb/blit/mmsfb_blit_blend_coloralpha_ayuv_to_yv12.cc \
	src/mmsgui/fb/blit/mmsfb_blit_blend_argb_to_bgr24.cc \
	src/mmsgui/fb/blit/mmsfb_blit_blend_argb_to_rgb32.cc \
	src/mmsgui/fb/blit/mmsfb_blit_blend_ayuv_to_rgb16.cc \
	src/mmsgui/fb/blit/mmsfb_blit_argb4444_to_argb4444.cc \
	src/mmsgui/fb/blit/mmsfb_blit_blend_ayuv_to_ayuv.cc \
	src/mmsgui/fb/blit/mmsfb_blit_coloralpha_rgb32_to_rgb32.cc \
	src/mmsgui/fb/blit/mmsfb_blit_yuy2_to_yuy2.cc \
	src/mmsgui/fb/blit/mmsfb_blit_i420_to_i420.cc \
	src/mmsgui/fb/blit/mmsfb_blit_blend_coloralpha_argb_to_bgr24.cc \
	src/mmsgui/fb/blit/mmsfb_blit_blend_argb_to_bgr555.cc \
	src/mmsgui/fb/blit/mmsfb_blit_blend_ayuv_to_yv12.cc \
	src/mmsgui/fb/blit/mmsfb_blit_coloralpha_argb_to_rgb32.cc \
	src/mmsgui/fb/blit/mmsfb_blit_coloralpha_bgr24_to_bgr24.cc \
	src/mmsgui/fb/blit/mmsfb_blit_blend_coloralpha_ayuv_to_ayuv.cc \
	src/mmsgui/fb/blit/mmsfb_blit_rgb16_to_rgb16.cc \
	src/mmsgui/fb/blit/mmsfb_blit_bgr24_to_bgr24.cc \
	src/mmsgui/fb/blit/mmsfb_blit_rgb24_to_argb.cc \
	src/mmsgui/fb/blit/mmsfb_blit_blend_argb_to_argb.cc \
	src/mmsgui/fb/blit/mmsfb_blit_blend_coloralpha_argb4444_to_argb4444.cc \
	src/mmsgui/fb/blit/mmsfb_blit_argb_to_rgb24.cc \
	src/mmsgui/fb/blit/mmsfb_blit_bgr555_to_bgr555.cc \
	\
	src/mmsgui/fb/fillrectangle/mmsfb_fillrectangle_bgr555.cc \
	src/mmsgui/fb/fillrectangle/mmsfb_fillrectangle_rgb32.cc \
	src/mmsgui/fb/fillrectangle/mmsfb_fillrectangle_blend_rgb16.cc \
	src/mmsgui/fb/fillrectangle/mmsfb_fillrectangle_ayuv.cc \
	src/mmsgui/fb/fillrectangle/mmsfb_fillrectangle_bgr24.cc \
	src/mmsgui/fb/fillrectangle/mmsfb_fillrectangle_rgb24.cc \
	src/mmsgui/fb/fillrectangle/mmsfb_fillrectangle_rgb16.cc \
	src/mmsgui/fb/fillrectangle/mmsfb_fillrectangle_yv12.cc \
	src/mmsgui/fb/fillrectangle/mmsfb_fillrectangle_argb3565.cc \
	src/mmsgui/fb/fillrectangle/mmsfb_fillrectangle_yuy2.cc \
	src/mmsgui/fb/fillrectangle/mmsfb_fillrectangle_blend_argb.cc \
	src/mmsgui/fb/fillrectangle/mmsfb_fillrectangle_i420.cc \
	src/mmsgui/fb/fillrectangle/mmsfb_fillrectangle_argb.cc \
	src/mmsgui/fb/fillrectangle/mmsfb_fillrectangle_blend_argb4444.cc \
	src/mmsgui/fb/fillrectangle/mmsfb_fillrectangle_blend_ayuv.cc \
	src/mmsgui/fb/fillrectangle/mmsfb_fillrectangle_argb4444.cc \
	\
	src/mmsgui/fb/mmsfb.cc \
	src/mmsgui/fb/mmsfbbackendinterface.cc \
	src/mmsgui/fb/mmsfbsurface.cc \
	src/mmsgui/fb/mmsfbconv.cc \
	src/mmsgui/fb/mmsfbbase.cc \
	src/mmsgui/fb/mmsfbsurfacemanager.cc \
	src/mmsgui/fb/mmsfbwindowmanager.cc \
	src/mmsgui/fb/mmsfbwindow.cc \
	src/mmsgui/fb/mmsfbfont.cc \
	src/mmsgui/fb/mmsfbwindowmanagerthread.cc \
	src/mmsgui/fb/mmsfblayer.cc \
	src/mmsgui/fb/mmsfbl4re.cc \
	\
	src/mmsgui/3d/mms3dsphere.cc \
	src/mmsgui/3d/mms3dtorus.cc \
	src/mmsgui/3d/mms3dmaterial.cc \
	src/mmsgui/3d/mms3dmatrixstack.cc \
	src/mmsgui/3d/mms3dtexture.cc \
	src/mmsgui/3d/mms3dscene.cc \
	src/mmsgui/3d/mms3dpolygonmesh.cc \
	src/mmsgui/3d/mms3dobject.cc \
	src/mmsgui/3d/mms3drectangle.cc \
	src/mmsgui/3d/mms3dcylinder.cc \
	\
	src/mmsbase/mmsbackendpluginhandler.cc \
	src/mmsbase/mmscentralpluginhandler.cc \
	src/mmsbase/mmscoreservicemanager.cc \
	src/mmsbase/mmsevent.cc \
	src/mmsbase/mmseventsignup.cc \
	src/mmsbase/mmsimportpluginhandler.cc \
	src/mmsbase/mmsmusicinterface.cc \
	src/mmsbase/mmsosdpluginhandler.cc \
	\
	src/mmscore/mmseventdispatcher.cc \
	src/mmscore/mmseventsignupmanager.cc \
	src/mmscore/mmseventthread.cc \
	src/mmscore/mmsimportscheduler.cc \
	src/mmscore/mmsinit.cc \
	src/mmscore/mmsperf.cc \
	src/mmscore/mmsperfinterface.cc \
	src/mmscore/mmspluginmanager.cc \
	src/mmscore/mmsswitcher.cc \
	src/mmscore/mmsswitcherthread.cc \
	src/mmscore/mmstranslator.cc \
	\
	src/mmsinput/mmsinputmapper.cc \
	src/mmsinput/mmsinputsubscription.cc \
	src/mmsinput/mmsinputmanager.cc \
	src/mmsinput/mmskeymap.cc \
	src/mmsinput/mmsinputthread.cc \
	src/mmsinput/mmsinputl4rehandler.cc \
	src/mmsinput/mmsinputhandler.cc \
	src/mmsinput/mmsinputgesture.cc \
	\
	src/mmstools/vfswrapper.cc



vpath %.cc $(SRC_DIR)/../contrib/

include $(L4DIR)/mk/lib.mk
