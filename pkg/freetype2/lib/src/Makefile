PKGDIR   ?= ../..
L4DIR    ?= $(PKGDIR)/../..

TARGET    = libfreetype2.a libfreetype2.so
SRC_C     =\
		autofit/autofit.c\
		bdf/bdf.c\
		cff/cff.c\
		base/ftbase.c\
		base/ftbbox.c\
		base/ftbdf.c\
		base/ftbitmap.c\
		bzip2/ftbzip2.c\
		cache/ftcache.c\
		base/ftcid.c\
		base/ftdebug.c\
		base/ftfstype.c\
		base/ftgasp.c\
		base/ftglyph.c\
		base/ftgxval.c\
		gzip/ftgzip.c\
		base/ftinit.c\
		base/ftlcdfil.c\
		lzw/ftlzw.c\
		base/ftmm.c\
		base/ftotval.c\
		base/ftpatent.c\
		base/ftpfr.c\
		base/ftstroke.c\
		base/ftsynth.c\
		base/ftsystem.c\
		base/fttype1.c\
		base/ftwinfnt.c\
		base/ftxf86.c\
		pcf/pcf.c\
		pfr/pfr.c\
		psaux/psaux.c\
		pshinter/pshinter.c\
		psnames/psnames.c\
		raster/raster.c\
		sfnt/sfnt.c\
		smooth/smooth.c\
		truetype/truetype.c\
		cid/type1cid.c\
		type1/type1.c\
		type42/type42.c\
		winfonts/winfnt.c


PRIVATE_INCDIR   = $(PKGDIR_ABS)/include
DEFINES     = -DFT2_BUILD_LIBRARY

include $(L4DIR)/mk/lib.mk
