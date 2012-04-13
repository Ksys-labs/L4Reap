#
# SConstruct
# disko build script
#
# Process this file with 'scons' to build the project.
# For more information, please visit: http://www.scons.org/ .
#
# Usage:
#
#   . scons               build the library
#   . scons -h            see available configuration options
#   . scons opt=value     set a configuration option
#   . scons check         perform dependency checking
#   . scons install       install library and include files
#   . scons doc           build documentation for the project (Doxygen)
#

import os, sys, string, re, SCons

#######################################################################
# Scons configuration                                                 #
#######################################################################
SetOption('implicit_cache', 1)

#######################################################################
# Version                                                             #
#######################################################################
packageVersionMajor = 1
packageVersionMinor = 9
packageVersionMicro = 0
packageVersionRC    = '-dev'

# Package information
packageName        = 'disko'
packageRealName    = 'Disko Framework'
packageDescription = 'Disko application framework for embedded devices (http://www.diskohq.com)'
packageVersion     = '%d.%d.%d%s' % (packageVersionMajor, packageVersionMinor, packageVersionMicro, packageVersionRC)

#######################################################################
# Get SCons version (copied from internal scons function)             #
#######################################################################
def GetSconsVersion():
	"""Split a version string into major, minor and (optionally)
	   revision parts.
	   This is complicated by the fact that a version string can be
	   something like 3.2b1."""
	
	version = string.split(string.split(SCons.__version__, ' ')[0], '.')
	v_major = int(version[0])
	v_minor = int(re.match('\d+', version[1]).group())
	if len(version) >= 3:
		v_revision = int(re.match('\d+', version[2]).group())
	else:
		v_revision = 0
		
	return v_major, v_minor, v_revision

sconsVersion = GetSconsVersion()

#######################################################################
# Help text                                                           #
#######################################################################
Help("Type: 'scons [options]' to build disko.\n" +
     "      'scons [options] check' to check the requirements for building disko.\n" +
     "      'scons -c' to clean.\n" +
     "      'scons doc' to create the API reference (doxygen has to be installed).\n" +
     "      'scons install' to install disko.\n\n" +
     "The following options are available:\n")



#######################################################################
# Helper functions                                                    #
#######################################################################
def PathIsDirCreateNone(key, value, env):
	if(value != 'none'):
		if sconsVersion < (0,98,1):
			PathOption.PathIsDirCreate(key, value, env)
		else:
			PathVariable.PathIsDirCreate(key, value, env)

#######################################################################
# Command line options                                                #
#######################################################################
be = ['dfb', 'fbdev', 'x11']
ot = ['stdfb', 'matroxfb', 'viafb', 'omapfb', 'davincifb', 'xshm', 'xvshm', 'gl2', 'gles2']
pf = ['argb', 'airgb', 'argb4444', 'argb3565', 'rgb16', 'rgb24', 'rgb32', 'bgr24', 'bgr555', 'ayuv', 'yv12', 'i420', 'yuy2']

if sconsVersion < (0,98,1):
	opts = Options('disko.conf')
	opts.AddOptions(
	PathOption('prefix',              'Installation directory', '/usr', PathOption.PathIsDirCreate),
	PathOption('destdir',             'Installation directory for cross-compile', 'none', PathIsDirCreateNone),
	BoolOption('debug',               'Build with debug symbols and without optimize', False),
	BoolOption('size',                'Optimize for size (only if debug=n)', False),
	BoolOption('messages',            'Build with logfile support', False),
	BoolOption('profile',             'Build with profiling support (includes debug option)', False),
	BoolOption('cross',               'Cross compile (to avoid some system checks)', False),
	BoolOption('use_sse',             'Use SSE optimization', False),
    BoolOption('enable_swscale',      'Build with swscale support', False),
	BoolOption('use_dl',              'Use dynamic linking support', True),
	ListOption('graphics_backend',    'Set graphics backend', 'none', be),
	ListOption('graphics_outputtype', 'Set graphics outputtype', 'all', ot),
	ListOption('pixelformats'         'Supported pixelformats', 'all', pf),
	ListOption('database',            'Set database backend', 'sqlite3', ['sqlite3', 'mysql', 'odbc']),
	ListOption('media',               'Set media backend', 'all', ['xine', 'gstreamer']),
	ListOption('images',              'Set image backends', 'all', ['png', 'jpeg', 'tiff']),
	BoolOption('enable_alsa',         'Build with ALSA support', True),
	BoolOption('enable_crypt',        'Build with mmscrypt support', True),
	BoolOption('enable_flash',        'Build with mmsflash support', False),
	BoolOption('enable_sip',          'Build with mmssip support', False),
	BoolOption('enable_curl',         'Build with curl support', True),
	BoolOption('enable_mail',         'Build with email support', False),
	BoolOption('enable_fribidi',      'Build with fribidi support', False),
	BoolOption('enable_actmon',       'Build activity monitor', False),
	BoolOption('enable_tools',        'Build disko tools', False),
	BoolOption('static_lib',          'Create statically linked library', False),
	BoolOption('big_lib',             'Create one big shared library', True))
else:
	opts = Variables('disko.conf')
	opts.AddVariables(
	PathVariable('prefix',              'Installation directory', '/usr', PathVariable.PathIsDirCreate),
	PathVariable('destdir',             'Installation directory for cross-compile', 'none', PathIsDirCreateNone),
	BoolVariable('debug',               'Build with debug symbols and without optimize', False),
	BoolVariable('size',                'Optimize for size (only if debug=n)', False),
	BoolVariable('messages',            'Build with logfile support', False),
	BoolVariable('profile',             'Build with profiling support (includes debug option)', False),
	BoolVariable('cross',               'Cross compile (to avoid some system checks)', False),
	BoolVariable('use_sse',             'Use SSE optimization', False),
    BoolVariable('enable_swscale',      'Build with swscale support', False),
	BoolVariable('use_dl',              'Use dynamic linking support', True),
	ListVariable('graphics_backend',    'Set graphics backend', 'none', be),
	ListVariable('graphics_outputtype', 'Set graphics outputtype', 'all', ot),
	ListVariable('pixelformats',        'Supported pixelformats', 'all', pf),   		 
	ListVariable('database',            'Set database backend', 'sqlite3', ['sqlite3', 'mysql', 'odbc']),
	ListVariable('media',               'Set media backend', 'all', ['xine', 'gstreamer']),
	ListVariable('images',              'Set image backends', 'all', ['png', 'jpeg', 'tiff']),
	BoolVariable('enable_alsa',         'Build with ALSA support', True),
	BoolVariable('enable_crypt',        'Build with mmscrypt support', True),
	BoolVariable('enable_flash',        'Build with mmsflash support', False),
	BoolVariable('enable_sip',          'Build with mmssip support', False),
	BoolVariable('enable_curl',         'Build with curl support', True),
	BoolVariable('enable_mail',         'Build with email support', False),
	BoolVariable('enable_fribidi',      'Build with fribidi support', False),
	BoolVariable('enable_actmon',       'Build activity monitor', False),
	BoolVariable('enable_tools',        'Build disko tools', False),
	BoolVariable('static_lib', 	        'Create statically linked library', False),
    BoolVariable('big_lib',             'Create one big shared library', True))

env = Environment(ENV = os.environ, CPPPATH = os.getcwd() + '/inc')

env['LIBS'] = []
env['LIBPATH'] = ''
env['diskoSources'] = []

opts.Update(env)
opts.Save('disko.conf', env)
Help(opts.GenerateHelpText(env))

# Here are our installation paths:
if os.environ.has_key('DESTDIR'):
	env['destdir'] = os.environ['DESTDIR']
if env['destdir'] != 'none':
	idir_prefix = env['destdir']
	if env['prefix'] != 'none':
		idir_prefix += ('/' + env['prefix'])
else:
	idir_prefix = env['prefix']

idir_lib    = idir_prefix + '/lib'
idir_bin    = idir_prefix + '/bin'
idir_inc    = idir_prefix + '/include/disko'
idir_data   = idir_prefix + '/share/disko'

# link with -rpath
# env['LINKFLAGS'].extend(['-Wl,-rpath=' + env['prefix'] + '/lib/disko'])

# extra flags
if env['messages']:
	env['CCFLAGS'].extend(['-D__ENABLE_LOG__'])

if env['profile']:
	env['CCFLAGS'].extend(['-pg'])
	env.Replace(debug = 1)

if env['debug']:
	if env['size']:
		print 'Warning: You cannot use the size option if debugging support is enabled!'
	env['CCFLAGS'].extend(['-O3', '-g', '-Wall', '-D__ENABLE_DEBUG__'])
else:
	if env['size']:
		env['CCFLAGS'].extend(['-Os'])
	else:
		if not os.environ.has_key('CXXFLAGS'):
			env['CCFLAGS'].extend(['-O3', '-g'])
	env['LINKFLAGS'].extend(['-s'])

# check which sse version to use
if env['use_sse']:
	if not env['cross'] and os.access('/proc/cpuinfo', os.R_OK):
		for l in open('/proc/cpuinfo'):
			if l.startswith('flags\t'): break;
		found = False
		for sse_str in ['sse5', 'sse4.2', 'sse4.1', 'sse4a', 'sse4', 'sse3', 'sse2', 'sse']:
			if l.find(sse_str) != -1:
				env['CCFLAGS'].extend(['-m'+ sse_str, '-mfpmath=sse', '-D__HAVE_SSE__'])
				found = True
				break
		env['use_sse'] = found
	else:
		env['CCFLAGS'].extend(['-msse2', '-mfpmath=sse', '-D__HAVE_SSE__'])

# use environment variables to override defaults
if os.environ.has_key('CC'):
	env['CC'] = [os.environ['CC'].split()]
if os.environ.has_key('CXX'):
	env['CXX'] = [os.environ['CXX'].split()] 
if os.environ.has_key('CXXFLAGS'):
	env['CCFLAGS'].extend(os.environ['CXXFLAGS'].split())
if os.environ.has_key('LDFLAGS'):
	env['LINKFLAGS'].extend(os.environ['LDFLAGS'].split())
if os.environ.has_key('PKG_CONFIG'):
	env['PKG_CONFIG'] = os.environ['PKG_CONFIG']
else:
	env['PKG_CONFIG'] = 'pkg-config'

# format output
#env['SHCXXCOMSTR']  = '  [CXX]    $SOURCE'
#env['SHLINKCOMSTR'] = '  [LD]     $TARGET'
#env['CXXCOMSTR']    = '  [CXX]    $SOURCE'
#env['LINKCOMSTR']   = '  [LD]     $TARGET'
#env['ARCOMSTR']     = '  [AR]     $TARGET'
#env['RANLIBCOMSTR'] = '  [RANLIB] $TARGET'

#######################################################################
# Subdirectories                                                      #
#######################################################################
diskoLibs  = ["mmsinfo",
              "mmstools",
              "mmsconfig",
              "mmsgui",
              "mmsbase",
              "mmsinput",
              "mmscore",
              "mmsmedia"]
if env['enable_flash']:
	diskoLibs.extend(["mmsflash"])
if env['enable_sip']:
	diskoLibs.extend(["mmssip"])
	
diskoTools = []

if env['enable_tools']:	
	diskoTools = ["taff","diskoappctrl"]

if env['enable_actmon']:	
	diskoTools.extend(["actmon"])

#######################################################################
# Helper functions                                                    #
#######################################################################
def checkOutputtypes(backends, outputtypes):
	for b in backends:
		if b in env['graphics_backend']:
			return
			
	for o in outputtypes:
		if o in env['graphics_outputtype']:
			print '\n*** \'' + o + '\' outputtype disabled (depends on %s)!' % ' or '.join(backends)
			env['graphics_outputtype'].remove(o)

def checkOptions(context):
	context.Message('Checking for options...')
	# check if graphics backend was chosen
	if not env['graphics_backend']:
		print '\nPlease choose a graphics backend by using:'
		print '  \'scons graphics_backend=dfb\'   or'
		print '  \'scons graphics_backend=fbdev\' or'
		print '  \'scons graphics_backend=x11\'   or'
		print '  \'scons graphics_backend=all\'\n'
		Exit(1)

	checkOutputtypes(['x11'],   ['xshm', 'xvshm', 'gl2'])
	checkOutputtypes(['fbdev'], ['gles2'])
	checkOutputtypes(['dfb'],   ['viafb'])
	checkOutputtypes(['fbdev', 'dfb'], ['stdfb', 'matroxfb', 'davincifb', 'omapfb'])
	
	if not env['database']:
		print '\nPlease choose a database by using:'
		print '  \'scons database=sqlite3\' or'
		print '  \'scons database=mysql\' or'
		print '  \'scons database=odbc\'\n'
		Exit(1)
		
	# to avoid 'error: no result' msg
	context.Result(True)
	
	return True

def tryConfigCommand(context, cmd):
	ret = context.TryAction(cmd)[0]
	if ret:
		try:
			context.env.ParseConfig(cmd)
		except OSError:
			ret = 0
	return ret

def checkPKGConfig(context, version):
	context.Message('Checking for pkg-config... ')
	ret = context.TryAction(context.env['PKG_CONFIG'] + ' --atleast-pkgconfig-version=%s' % version)[0]
	context.Result(ret)
	return ret

def checkXineBlDvb(context):
	context.Message('Checking for xine bldvb input plugin... ')
	pipe = os.popen(context.env['PKG_CONFIG'] + ' --variable=plugindir libxine')
 	xinePluginPath = pipe.read()
 	pipe.close()
	if xinePluginPath != "" and os.access(xinePluginPath.rstrip('\n') + '/xineplug_inp_bldvb.so', os.R_OK):
		ret = True
	else:
		ret = False
		
	context.Result(ret)
	return ret

def checkGstDiskoVideosink(context):
	context.Message('Checking for gstreamer plugin diskovideosink... ')
	pipe = os.popen(context.env['PKG_CONFIG'] + ' --variable=pluginsdir gstreamer-0.10')
 	gstPluginPath = pipe.read()
 	pipe.close()
	ret = (gstPluginPath != "" and os.access(gstPluginPath.rstrip('\n') + '/libgstdiskovideosink.so', os.R_OK))
	context.Result(ret)
	return ret

def checkPKG(context, name):
	return tryConfigCommand(context, context.env['PKG_CONFIG'] + ' --libs --cflags \'%s\'' % name)

def checkConf(context, name):
	if name.find(' '):
		return False
		
	seperators = ['-', '_']
	for sep in seperators:
		configcall = '%s%sconfig --libs --cflags' % (name.lower(), sep)
		if(tryConfigCommand(context, configcall)):
			return True
	return False

def checkSimpleLib(context, liblist, header = '', lang = 'c++', required = 1):
	for lib in liblist:
		context.Message('Checking for %s... ' % lib)
		ret = checkPKG(context, lib)
		if ret:
			context.Result(True)
			return True

		ret = checkConf(context, lib)
		if ret:
			context.Result(True)
			return True

		# redirect stdout to suppress messages from built in checks
		sys.stdout = open('/dev/null', 'a')
		if len(header):
			ret = conf.CheckLibWithHeader(liblist, header, lang)
		else:
			ret = conf.CheckLib(lib)
		sys.stdout.close()
		sys.stdout = sys.__stdout__
		if ret:
			context.Result(True)
			return True

	context.Result(False)

	if required:
		print 'required lib %s not found' % lib
		Exit(1)

	return False

def checkBacktrace(context):
	backtrace_test = """
	#include <execinfo.h>
	int main(int argc, char **argv) {
			void *array[1];
			backtrace(array, 1);
			backtrace_symbols(array, 1);
			return 0;
	}	
	"""

	context.Message('Checking for backtrace_symbols()... ')
	result = context.TryLink(backtrace_test, '.c')
	context.Result(result)
	return result
		
def printSummary():
	print '\n********************* Summary *********************\n'
	print 'Prefix:             : %s'   % conf.env['prefix']
	print 'Destdir:            : %s'   % conf.env['destdir']
	print 'Graphic backends    : %s'   % ', '.join(conf.env['graphics_backend'])
	print 'Graphic outputtypes : %s' % ', '.join(conf.env['graphics_outputtype'])

	if len(env['pixelformats']) == len(pf):
		print 'Pixelformats        : all'
	elif len(env['pixelformats']) > 0:
		print 'Pixelformats        : %s' % ', '.join(conf.env['pixelformats'])

	print 'Database backends   : %s'   % ', '.join(conf.env['database'])
	if(conf.env['media']):
		print 'Media backends      : %s' % ', '.join(conf.env['media'])
	else:
		print 'Media backends      : none'
	print 'Image support       : %s' % ', '.join(conf.env['images'])
	print
	if(conf.env['alsa']):
		print 'ALSA support        : yes'
	else:
		print 'ALSA support        : no'
	if(conf.env['mmscrypt']):
		print 'Building mmscrypt   : yes'
	else:
		print 'Building mmscrypt   : no'
	if(conf.env['enable_flash']):
		print 'Building mmsflash   : yes'
	else:
		print 'Building mmsflash   : no'
	if(conf.env['enable_sip']):
		print 'Building mmssip     : yes'
	else:
		print 'Building mmssip     : no'
	if(conf.env['enable_curl']):
		print 'curl support        : yes'
	else:
		print 'curl support        : no'
	if(conf.env['enable_mail']):
		print 'E-Mail support      : yes'
	else:
		print 'E-Mail support      : no'
	if(conf.env['enable_fribidi']):
		print 'FriBidi support     : yes'
	else:
		print 'FriBidi support     : no'
	if(conf.env['enable_actmon']):
		print 'Building actmon     : yes'
	else:
		print 'Building actmon     : no'
	if(conf.env['enable_tools']):
		print 'Building tools      : yes\n'
	else:
		print 'Building tools      : no\n'
	if(conf.env['messages']):
		print 'log messages        : yes'
	else:
		print 'log messages        : no'
	if(conf.env['debug']):
		print 'debug symbols       : yes'
	else:
		print 'debug symbols       : no'
	if(conf.env['profile']):
		print 'profiling info      : yes'
	else:
		print 'profiling info      : no'
	if(conf.env['use_sse']):
		print 'SSE optimization    : yes'
	else:
		print 'SSE optimization    : no'
	if(conf.env['enable_swscale']):
		print 'swscale support     : yes'
	else:
		print 'swscale support     : no'
	if(conf.env.has_key('libdl')):
		print 'use libdl           : yes\n'
	else:
		print 'use libdl           : no\n'
	if(conf.env['static_lib']):
		print 'link type           : static'
	else:
		print 'link type           : shared'
	print '\n***************************************************\n'

#######################################################################
# Check dependencies                                                  #
#######################################################################
if not ('-c' in sys.argv or '-h' in sys.argv):
	conf = Configure(env,
                     custom_tests = {'checkOptions' : checkOptions,
                                     'checkPKGConfig' : checkPKGConfig,
                                     'checkXineBlDvb' : checkXineBlDvb,
                                     'checkGstDiskoVideosink' : checkGstDiskoVideosink,
                                     'checkConf' : checkConf,
                                     'checkPKG' : checkPKG,
                                     'checkSimpleLib' : checkSimpleLib,
                                     'checkBacktrace' : checkBacktrace},
                     conf_dir = 'build/.sconf_temp',
                     log_file = 'build/.config.log',
                     clean = False,
                     help  = False)

	conf.checkOptions()

	# checks that are required everytime
	if not conf.checkPKGConfig('0.8'):
		Exit(1)
	conf.checkSimpleLib(['sigc++-2.0'],        'sigc++-2.0/sigc++/sigc++.h')
	conf.checkSimpleLib(['libxml-2.0 >= 2.6'], 'libxml2/libxml/parser.h')
	if (env['enable_curl']):
		conf.checkSimpleLib(['libcurl'],           'curl/curl.h')
		conf.env['CCFLAGS'].extend(['-D__HAVE_CURL__'])	
	conf.checkSimpleLib(['freetype2'],         'freetype/freetype.h')

	if conf.CheckLibWithHeader(['libiconv'], ['iconv.h'], 'c++'):
		conf.env['libiconv'] = True

	if conf.CheckHeader(['wordexp.h']):
		conf.env['CCFLAGS'].extend(['-D__HAVE_WORDEXP__'])

	if('png' in env['images']):
		if conf.checkSimpleLib(['libpng'], ['png.h'], required = 0):
			conf.env['CCFLAGS'].extend(['-D__HAVE_PNG__'])
		elif conf.CheckLibWithHeader(['libpng'], ['png.h'], 'c++'):
			conf.env['CCFLAGS'].extend(['-D__HAVE_PNG__'])
		else:
			conf.env['images'].remove('png')

	if('jpeg' in env['images']):
		if conf.checkSimpleLib(['libjpeg'], ['cstdio', 'jpeglib.h'], required = 0):
			conf.env['CCFLAGS'].extend(['-D__HAVE_JPEG__'])
		elif conf.CheckLibWithHeader(['libjpeg'], ['cstdio', 'jpeglib.h'], 'c++'):
			conf.env['CCFLAGS'].extend(['-D__HAVE_JPEG__'])
		else:
			conf.env['images'].remove('jpeg')

	if('tiff' in env['images']):
		if conf.checkSimpleLib(['libtiff'], ['tiffio.h'], required = 0):
			conf.env['CCFLAGS'].extend(['-D__HAVE_TIFF__'])
		elif conf.CheckLibWithHeader(['libtiff'], ['tiffio.h'], 'c++'):
			conf.env['CCFLAGS'].extend(['-D__HAVE_TIFF__'])
		else:
			conf.env['images'].remove('tiff')

	# checks required if using dynamic linking support
	if(env['use_dl']):
		if conf.CheckLibWithHeader(['libdl'], ['dlfcn.h'], 'c++'):
			conf.env['CCFLAGS'].extend(['-D__HAVE_DL__'])
			conf.env['libdl'] = True

	# checks required if building DirectFB backend
	if('dfb' in env['graphics_backend']):
		conf.checkSimpleLib(['directfb'],   'directfb/directfb.h')
		conf.env['CCFLAGS'].extend(['-D__HAVE_DIRECTFB__'])
		
	# checks required if building fbdev backend
	if('fbdev' in env['graphics_backend']):
		conf.env['CCFLAGS'].extend(['-D__HAVE_FBDEV__'])

	# checks for building OpenGL ES 2.0 backend
	if('gles2' in env['graphics_outputtype']):
		if conf.CheckLibWithHeader(['GLESv2'], 'GLES2/gl2.h', 'c++', 'glGenFramebuffers(0,(GLuint*)0);'):
			conf.env['CCFLAGS'].extend(['-D__HAVE_OPENGL__'])
			conf.env['CCFLAGS'].extend(['-D__HAVE_GLES2__'])
		else:
			conf.env['graphics_outputtype'].remove('gles2')
		if conf.CheckLibWithHeader(['EGL'], 'EGL/egl.h', 'c++', 'return eglGetError();'):
			conf.env['CCFLAGS'].extend(['-D__HAVE_EGL__'])

	# checks required if building X11 backend
	if('x11' in env['graphics_backend']):
		conf.checkSimpleLib(['x11'],	   'X11/Xlib.h')
		conf.checkSimpleLib(['xxf86vm'],   'X11/extensions/xf86vmode.h')
		conf.env['CCFLAGS'].extend(['-D__HAVE_XLIB__'])
		# TODO: actually XV doesn't depend on XRender/XComposite, but for now it won't compile without it
		if conf.checkSimpleLib(['xv'], 'X11/extensions/Xvlib.h', 0):
			if conf.checkSimpleLib(['xrender'], 'X11/extensions/Xrender.h', 0):
				conf.env['CCFLAGS'].extend(['-D__HAVE_XRENDER__'])
				if conf.checkSimpleLib(['xcomposite'], 'X11/extensions/Xcomposite.h', 0):
					conf.env['CCFLAGS'].extend(['-D__HAVE_XCOMPOSITE__', '-D__HAVE_XV__'])

		# checks for OpenGL and X11 backend
		if 'gl2' in conf.env['graphics_outputtype']:
			if conf.CheckLib('GLEW', 'glGenFramebuffersEXT'):
				conf.env['LIBS'].append('GLEW')
				if conf.checkSimpleLib(['gl'],   'GL/gl.h'):
					conf.env['CCFLAGS'].extend(['-D__HAVE_OPENGL__'])
					if conf.CheckLib('GL', 'glBlendEquation'):
						conf.env['CCFLAGS'].extend(['-D__HAVE_GL2__'])	
					conf.checkSimpleLib(['glu'],  'GL/glu.h')
					if conf.CheckCXXHeader('GL/glx.h') and conf.CheckLib('GL', 'glXCreateContext'):
						conf.env['CCFLAGS'].extend(['-D__HAVE_GLX__'])
			else:
				conf.env['graphics_outputtype'].remove('gl2')
	
	# check if OpenGL 2.0 and OpenGL ES are both activated
	if 'gl2' in conf.env['graphics_outputtype'] and 'gles2' in conf.env['graphics_outputtype']:
		print '\nOpenGL 2.0 and OpenGL ES 2.0 support is mutually exclusive.' 
		print 'You have to choose between one of them by using:'
		print '  \'scons graphics_outputtype=gl2\' or'
		print '  \'scons graphics_outputtype=gles2\''
		Exit(1)
	
	
	# pixelformats (no effect when using OpenGL or OpenGL ES
	if len(conf.env['graphics_outputtype']) == 1 and ('gl2' in conf.env['graphics_outputtype'] or 'gles2' in conf.env['graphics_outputtype']):
		conf.env['pixelformats'][:] = [];

	if len(env['pixelformats']) == len(pf):
		env['CCFLAGS'].extend(['-D__HAVE_PF_ALL__'])
	else:
		if 'argb' in env['pixelformats']:
			env['CCFLAGS'].extend(['-D__HAVE_PF_ARGB__'])
		if 'airgb' in env['pixelformats']:
			env['CCFLAGS'].extend(['-D__HAVE_PF_AiRGB__'])
		if 'argb4444' in env['pixelformats']:
			env['CCFLAGS'].extend(['-D__HAVE_PF_ARGB4444__'])
		if 'argb3565' in env['pixelformats']:
			env['CCFLAGS'].extend(['-D__HAVE_PF_ARGB3565__'])
		if 'rgb16' in env['pixelformats']:
			env['CCFLAGS'].extend(['-D__HAVE_PF_RGB16__'])
		if 'rgb24' in env['pixelformats']:
			env['CCFLAGS'].extend(['-D__HAVE_PF_RGB24__'])
		if 'rgb32' in env['pixelformats']:
			env['CCFLAGS'].extend(['-D__HAVE_PF_RGB32__'])
		if 'bgr24' in env['pixelformats']:
			env['CCFLAGS'].extend(['-D__HAVE_PF_BGR24__'])
		if 'bgr555' in env['pixelformats']:
			env['CCFLAGS'].extend(['-D__HAVE_PF_BGR555__'])
		if 'ayuv' in env['pixelformats']:
			env['CCFLAGS'].extend(['-D__HAVE_PF_AYUV__'])
		if 'yv12' in env['pixelformats']:
			env['CCFLAGS'].extend(['-D__HAVE_PF_YV12__'])
		if 'i420' in env['pixelformats']:
			env['CCFLAGS'].extend(['-D__HAVE_PF_I420__'])
		if 'yuy2' in env['pixelformats']:
			env['CCFLAGS'].extend(['-D__HAVE_PF_YUY2__'])

	# checks required if building mmsmedia
	if('xine' in env['media']):
		xine_str = 'libxine'
		if('x11' in env['graphics_backend']):
			xine_str += ' >= 1.1.15'
			
		if not conf.checkSimpleLib([xine_str], 'xine.h', required = 0):
			print '***************************************************\n'
			print 'Xine not found!'
			print 'Disabling xine media backend'
			print '\n***************************************************'
			env['media'].remove('xine')
		else:
			conf.env['CCFLAGS'].extend(['-D__HAVE_MMSMEDIA__', '-DXINE_DISABLE_DEPRECATED_FEATURES', '-D__HAVE_XINE__'])
			if conf.checkXineBlDvb():
				conf.env['CCFLAGS'].extend(['-D__HAVE_XINE_BLDVB__'])

	if('gstreamer' in env['media']):
		if not conf.checkSimpleLib(['gstreamer-0.10 >= 0.10.22'], 'gst/gst.h', required = 0) or	not conf.checkSimpleLib(['gstreamer-plugins-base-0.10'], 'gst/gst.h', required = 0):
			print '***************************************************\n'
			print 'GStreamer not found or version is older than 0.10.22!'
			print 'Disabling gstreamer media backend'
			print '\n***************************************************'
			env['media'].remove('gstreamer')
		elif not conf.checkGstDiskoVideosink():
			print '***************************************************\n'
			print 'GStreamer plugin diskovideosink not found!'
			print 'Disabling gstreamer media backend'
			print '\n***************************************************'
			env['media'].remove('gstreamer')
		else:
			conf.env['CCFLAGS'].extend(['-D__HAVE_MMSMEDIA__', '-D__HAVE_GSTREAMER__'])

	# check for ALSA
	if env['enable_alsa']:
		if conf.checkSimpleLib(['alsa'], 'alsa/version.h', required = 0):
			conf.env['CCFLAGS'].extend(['-D__HAVE_MMSMEDIA__', '-D__HAVE_MIXER__'])
			conf.env['alsa'] = 1
		else:
			conf.env['alsa'] = 0
	else:
		conf.env['alsa'] = 0
		
	# checks required for database backends
	if 'sqlite3' in env['database']:
		conf.checkSimpleLib(['sqlite3'], 'sqlite3.h')
		conf.env['CCFLAGS'].extend(['-D__ENABLE_SQLITE__'])
	if 'mysql' in env['database']:
		conf.checkSimpleLib(['mysql'],      'mysql.h')
		conf.env['CCFLAGS'].extend(['-D__ENABLE_MYSQL__'])
	if 'odbc' in env['database']:
		if conf.CheckCXXHeader('/usr/include/sql.h'):
			conf.env.Append(LIBS = 'odbc')
			conf.env['CCFLAGS'].extend(['-D__ENABLE_FREETDS__'])
		elif conf.CheckCXXHeader('/usr/local/include/sql.h'):
			conf.env.Append(LIBPATH = '/usr/local/lib', LIBS = 'odbc')
			conf.env['CCFLAGS'].extend(['-D__ENABLE_FREETDS__', '-I/usr/local/include'])
		else:
			Exit(1)

	# check for openssl
	if env['enable_crypt']:
		if not conf.checkSimpleLib(['openssl'],    'openssl/conf.h', required = 0):
			conf.env['mmscrypt'] = 0
			env['enable_crypt'] = False
		else:
			conf.env['CCFLAGS'].extend(['-D__HAVE_MMSCRYPT__'])
			conf.env['mmscrypt'] = 1
	else:
		conf.env['mmscrypt'] = 0
		env['enable_crypt'] = False

	# checks required if building mmsflash
	if(env['enable_flash']):
		if conf.checkSimpleLib(['swfdec-0.9'], 'swfdec-0.9/swfdec/swfdec.h', required = 0):
			conf.env['CCFLAGS'].extend(['-D__HAVE_MMSFLASH__'])
			swfdecversion='0.9'
		else: 
			if conf.checkSimpleLib(['swfdec-0.8'], 'swfdec-0.8/swfdec/swfdec.h'):
				conf.env['CCFLAGS'].extend(['-D__HAVE_MMSFLASH__'])
				swfdecversion='0.8'


	# checks required if building mmssip
	if(env['enable_sip']):
		if conf.checkSimpleLib(['libpj'], 'pjlib.h'):
			conf.checkSimpleLib(['uuid'], 'uuid/uuid.h', required = 0)
			conf.env['CCFLAGS'].extend(['-D__HAVE_MMSSIP__'])

		
	# checks required if building with email support
	if(env['enable_mail']):
		conf.checkSimpleLib(['vmime'], 'vmime.h')
		conf.env['CCFLAGS'].extend(['-D__HAVE_VMIME__'])

	# checks required if building with fribidi support
	if(env['enable_fribidi']):
		conf.checkSimpleLib(['fribidi'], 'fribidi.h')
		conf.env['CCFLAGS'].extend(['-D__HAVE_FRIBIDI__'])

	# checks required if building with swscale support
	if(env['enable_swscale']):
		conf.checkSimpleLib(['libswscale'], 'libswscale/swscale.h')
		conf.env['CCFLAGS'].extend(['-D__HAVE_SWSCALE__']) 

	# checks required if building activity monitor
	if(env['enable_actmon']):
		conf.env['CCFLAGS'].extend(['-D__ENABLE_ACTMON__'])

	# check for backtrace_symbols() support
	if conf.checkBacktrace():
		conf.env['CCFLAGS'].extend(['-D__HAVE_BACKTRACE__'])

	env2 = conf.Finish()
	if env2:
		env = env2
		env['LIBS'].extend(['pthread', 'z'])
		printSummary()
		
	if 'check' in BUILD_TARGETS:
		Exit(0)

#######################################################################
# Creating pkg-config file                                            #
#######################################################################
# TODO: handle disko_pc_libs                                          #
if 'install' in BUILD_TARGETS:
	disko_pc = open('disko.pc', 'w')
	disko_pc_requires = 'libxml-2.0 >= 2.6, sigc++-2.0, freetype2'
	if (env['enable_curl']):
		disko_pc_requires += ', libcurl'
	if env['LIBPATH']:
		disko_pc_libs_private = '-L%s' % ' -L'.join(env['LIBPATH'])
	else:
		disko_pc_libs_private = ''

	disko_pc_libs_private += ' -lpthread -lz'
		
	if env['big_lib'] or env['static_lib']:
		disko_pc_libs = '-ldisko'
	else:
		disko_pc_libs = '-lmmsinfo -lmmsconfig -lmmstools -lmmsgui -lmmsinput -lmmsbase -lmmscore'

	if env.has_key('libiconv'):
		disko_pc_libs_private += ' -liconv'
	
	if 'png' in env['images']:
		disko_pc_requires += ', libpng >= 1.2'

	if 'jpeg' in env['images']:
		if(env['static_lib']):
			disko_pc_libs += ' -ljpeg'
		else:
			disko_pc_libs_private += ' -ljpeg'

	if 'tiff' in env['images']:
		if(env['static_lib']):
			disko_pc_libs += ' -ltiff'
		else:
			disko_pc_libs_private += ' -ltiff'

	if env.has_key('libdl'):
		disko_pc_libs_private += ' -ldl'

	if 'dfb' in env['graphics_backend']:
		disko_pc_requires += ', directfb'
	 
	if 'x11' in env['graphics_backend']:
		disko_pc_requires += ', x11, xv, xxf86vm, xcomposite, xrender'
		if '-D__HAVE_OPENGL__' in env['CCFLAGS']:
			disko_pc_requires += ', gl, glu'
	
	for l in ['GLEW', 'GLESv2' , 'EGL']:
		if l in env['LIBS']:
			disko_pc_libs += ' -l%s' % l

	if env['alsa']:
	 	disko_pc_requires += ', alsa'
	
	if env['media'] and env['media'] != 'none':
		if not env['big_lib'] and not env['static_lib']:
			disko_pc_libs += ' -lmmsmedia'
		
		if 'xine' in env['media']:
			if('x11' in env['graphics_backend']):
				disko_pc_requires += ', libxine >= 1.1.15'
			else:
				disko_pc_requires += ', libxine'

		if 'gstreamer' in env['media']:
			disko_pc_requires += ', gstreamer-0.10'

	if env['enable_flash']:
		disko_pc_requires += ', swfdec-' + swfdecversion
		if not env['big_lib'] and not env['static_lib']:
			disko_pc_libs += ' -lmmsflash'

	if env['enable_sip']:
		disko_pc_requires += ', libpj'
		if not env['big_lib'] and not env['static_lib']:
			disko_pc_libs += ' -lmmssip'
		if('uuid' in env['LIBS']):
			disko_pc_requires += ', uuid'
		
	if env['enable_mail']:
		disko_pc_requires += ', vmime'
		
	if env['enable_fribidi']:
		disko_pc_requires += ', fribidi'
		
	if env['enable_crypt']:
		disko_pc_requires += ', openssl'

	if 'sqlite3' in env['database']:
		disko_pc_requires += ', sqlite3'
		
	if 'mysql' in env['database']:
		disko_pc_requires += ', mysql'

	if env['enable_swscale']:
		disko_pc_libs_private += ' -lswscale -lavutil'

	disko_pc.write('prefix=')
	if env['destdir'] and env['destdir'] != 'none':
		disko_pc.write(env['destdir'] + '/')
	disko_pc.write(env['prefix'] + '\n')
	disko_pc.write('exec_prefix=${prefix}\n')
	disko_pc.write('libdir=${exec_prefix}/lib\n')
	disko_pc.write('includedir=${exec_prefix}/include/disko\n\n')
	disko_pc.write('Name: ' + packageRealName + '\n')
	disko_pc.write('Description: ' + packageDescription + '\n')
	disko_pc.write('Version: ' + packageVersion + '\n')
	disko_pc.write('Requires: ' + disko_pc_requires + '\n')
	disko_pc.write('Libs: -L${libdir} ' + disko_pc_libs + '\n')
	disko_pc.write('Libs.private: ' + disko_pc_libs_private + '\n')
	disko_pc.write('Cflags: -I${includedir}/ ')

	skip=0
	for ccflag in env['CCFLAGS']:
		if ccflag == '-isystem':
			skip = 1
		elif skip == 1:
			skip = 0
		elif(type(ccflag).__name__ == 'str'):
			disko_pc.write(' ' + ccflag)
		else:
			disko_pc.write(' ' + ' '.join(ccflag))
	disko_pc.write('\n')
	
	disko_pc.close()

#######################################################################
# Create targets                                                      #
#######################################################################
env['TOP_DIR'] = os.getcwd()
env.Decider('MD5-timestamp')
if(env['enable_tools'] or env['enable_actmon']):
	all = env.Alias('all', ['lib', 'bin'])
else:
	all = env.Alias('all', ['lib'])
install = env.Alias('install', [idir_prefix])
Depends(install, all)
env.Default(all)	

env.Install(idir_inc, env['TOP_DIR'] + '/inc/mms.h')
env.Install(idir_inc, env['TOP_DIR'] + '/inc/disko.h')
env.Install(idir_inc, env['TOP_DIR'] + '/inc/diskoversion.h')
env.Install(idir_prefix + '/lib/pkgconfig', 'disko.pc')
Clean('lib', 'disko.pc')

#######################################################################
#  Documentation                                                      #
#######################################################################
doxygenBuilder = Builder(action = 'doxygen $SOURCE')
env.Append(BUILDERS = { 'DoxygenDoc' : doxygenBuilder })
doxygenDocPath = '(doc)'
env.DoxygenDoc(doxygenDocPath, 'doc/conf/disko.conf')
env.Alias('doc', doxygenDocPath)

#######################################################################
# Building disko                                                      #
#######################################################################
Export('env idir_lib idir_bin idir_inc idir_data')

libList = ""
for libDir in diskoLibs:
        libList += 'build/libs/' + libDir + "/SConscript "

toolList = ""
for toolDir in diskoTools:
        toolList += 'build/tools/' + toolDir + "/SConscript "

VariantDir('build/libs', 'src', duplicate = 0)
SConscript(Split(libList), options = opts)

#######################################################################
# Big Shared Library                                                  #
#######################################################################
if env['big_lib']:
	libdisko_shared = env.SharedLibrary('lib/libdisko', env['diskoSources'])
	env.Install(idir_lib, libdisko_shared)
	Export('libdisko_shared')

#######################################################################
# Static Library                                                      #
#######################################################################
if env['static_lib']:
	libdisko_static = env.StaticLibrary('lib/libdisko', env['diskoSources'])
	env.Install(idir_lib, libdisko_static)
	Export('libdisko_static')

#######################################################################
# Tools                                                               #
#######################################################################
VariantDir('build/tools', 'tools', duplicate = 0)
SConscript(Split(toolList), options = opts)

