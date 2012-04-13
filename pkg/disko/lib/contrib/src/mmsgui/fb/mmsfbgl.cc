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

#include "mmsgui/fb/mmsfbgl.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#ifdef __HAVE_OPENGL__

#define INITCHECK if (!this->initialized) return false;

#define GET_ATTR(name,str) \
	eglGetConfigAttrib(eglDisplay, eglConfig[i], name, &value); \
	printf("  %s = %x\n", str, value);


#define OGL_CALC_COORD(v1, v2) (((v1)<(v2)) ? (float)(v1) : (float)(v1) + 0.99)
#define OGL_CALC_2X_N(v1, v2, width)	(OGL_CALC_COORD(v1, v2) / (width))
#define OGL_CALC_2Y_N(v1, v2, height)	(OGL_CALC_COORD(v1, v2) / (height))

#define OGL_CALC_COORD_MIDDLE(v1, v2) (((v1)<(v2)) ? (float)(v1) + 0.49 : (float)(v1) + 0.51)


MMSFBGL::MMSFBGL() {
	// init vars...
	this->initialized = false;
	this->VSMatrixLoc_initialized = false;
	this->FSColorLoc_initialized = false;
	this->FSTextureLoc_initialized = false;
	this->VSTexCoordLoc_initialized = false;

	// default framebuffer object is always 0, so set bound_fbo to 0
	this->bound_fbo = 0;
}

MMSFBGL::~MMSFBGL() {
	terminate();
}


//////////////////////////////

//#define ERROR_CHECK_EXIT
//#define ERROR_CHECK_RETURN
#define ERROR_CHECK_PRINT

#ifdef ERROR_CHECK_EXIT
#define ERROR_CHECK_VOID(where) if (!getError(where, __LINE__)) exit(1);
#define ERROR_CHECK_BOOL(where) if (!getError(where, __LINE__)) exit(1);
#endif

#ifdef ERROR_CHECK_RETURN
#define ERROR_CHECK_VOID(where) if (!getError(where, __LINE__)) return;
#define ERROR_CHECK_BOOL(where) if (!getError(where, __LINE__)) return false;
#endif

#ifdef ERROR_CHECK_PRINT
#define ERROR_CHECK_VOID(where) getError(where, __LINE__);
#define ERROR_CHECK_BOOL(where) getError(where, __LINE__);
#endif

#ifndef ERROR_CHECK_VOID
#define ERROR_CHECK_VOID(where)
#endif

#ifndef ERROR_CHECK_BOOL
#define ERROR_CHECK_BOOL(where)
#endif

//////////////////////////////


bool MMSFBGL::getError(const char* where, int line) {
#ifdef __HAVE_OPENGL__
	int err = glGetError();
	if (err != GL_NO_ERROR) {
		// try to get description
		const char *desc = "unknown";
		switch (err) {
		case GL_INVALID_ENUM:
			desc = "GL_INVALID_ENUM";
			break;
		case GL_INVALID_VALUE:
			desc = "GL_INVALID_VALUE";
			break;
		case GL_INVALID_OPERATION:
			desc = "GL_INVALID_OPERATION";
			break;
		case GL_OUT_OF_MEMORY:
			desc = "GL_OUT_OF_MEMORY";
			break;
#ifdef GL_STACK_OVERFLOW
		case GL_STACK_OVERFLOW:
			desc = "GL_STACK_OVERFLOW";
			break;
#endif
#ifdef GL_STACK_UNDERFLOW
		case GL_STACK_UNDERFLOW:
			desc = "GL_STACK_UNDERFLOW";
			break;
#endif
#ifdef GL_TABLE_TOO_LARGE
		case GL_TABLE_TOO_LARGE:
			desc = "GL_TABLE_TOO_LARGE";
			break;
#endif
#ifdef GL_INVALID_FRAMEBUFFER_OPERATION
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			desc = "GL_INVALID_FRAMEBUFFER_OPERATION";
			break;
#endif
		}

		// print error to stdout
		printf("MMSFBGL: ERR 0x%x (%s) AT LINE %d, %s\n", err, desc, line, where);
		return false;
	}
#endif

#ifdef __HAVE_EGL__
	EGLint iErr = eglGetError();
	if (iErr != EGL_SUCCESS) {
		printf("MMSFBGL: ERR 0x%x AT LINE %d, %s\n", iErr, line, where);
		return false;
	}
#endif

	return true;
}


bool MMSFBGL::buildShader(MMSFBGL_SHADER_TYPE shader_type, const char *shader_code, GLuint *shader) {

#ifdef __HAVE_GLES2__

	// create the shader object
	*shader = glCreateShader((shader_type==MMSFBGL_SHADER_TYPE_FRAGMENT_SHADER)?GL_FRAGMENT_SHADER:GL_VERTEX_SHADER);
	ERROR_CHECK_BOOL("glCreateShader()");

	// load the source code into it
	glShaderSource(*shader, 1, (const char**)&shader_code, NULL);
	ERROR_CHECK_BOOL("glShaderSource()");

	// compile the source code
	glCompileShader(*shader);
	ERROR_CHECK_BOOL("glCompileShader()");

	// check if compilation succeeded
	GLint compiled;
	glGetShaderiv(*shader, GL_COMPILE_STATUS, &compiled);
	ERROR_CHECK_BOOL("glGetShaderiv()");

	if (!compiled) {
		// an error happened, first retrieve the length of the log message
		int i32InfoLogLength, i32CharsWritten;
		glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &i32InfoLogLength);
		ERROR_CHECK_BOOL("glGetShaderiv()");

		// allocate enough space for the message and retrieve it
		char* pszInfoLog = new char[i32InfoLogLength];
		glGetShaderInfoLog(*shader, i32InfoLogLength, &i32CharsWritten, pszInfoLog);
		ERROR_CHECK_BOOL("glGetShaderInfoLog()");

		// displays the error
		printf("Failed to compile %s shader: %s\n",
				(shader_type==MMSFBGL_SHADER_TYPE_FRAGMENT_SHADER)?"fragment":"vertex",
				pszInfoLog);

		// freeing memory
		delete [] pszInfoLog;
		glDeleteShader(*shader);
		ERROR_CHECK_BOOL("glDeleteShader()");
		*shader = 0;
		return false;
	}

	return true;

#else

	return false;

#endif
}

bool MMSFBGL::linkProgram(GLuint fragment_shader, GLuint vertex_shader, GLuint *program) {

#ifdef __HAVE_GLES2__

	// create the shader program
    *program = glCreateProgram();
	ERROR_CHECK_BOOL("glCreateProgram()");

	// attach the fragment and vertex shaders to it
    glAttachShader(*program, fragment_shader);
	ERROR_CHECK_BOOL("glAttachShader(*program, fragment_shader)");
    glAttachShader(*program, vertex_shader);
	ERROR_CHECK_BOOL("glAttachShader(*program, vertex_shader)");

	// bind the vertex attribute to specified location
    glBindAttribLocation(*program, MMSFBGL_VSV_LOC, "VSVertex");
	ERROR_CHECK_BOOL("glBindAttribLocation(*program, MMSFBGL_VSV_LOC, \"VSVertex\")");

	// link the program
    glLinkProgram(*program);
	ERROR_CHECK_BOOL("glLinkProgram()");

	// check if linking succeeded in the same way we checked for compilation success
    GLint linked;
    glGetProgramiv(*program, GL_LINK_STATUS, &linked);
	ERROR_CHECK_BOOL("glGetProgramiv()");

	if (!linked) {
		// an error happened, first retrieve the length of the log message
		int ui32InfoLogLength, ui32CharsWritten;
		glGetProgramiv(*program, GL_INFO_LOG_LENGTH, &ui32InfoLogLength);
		ERROR_CHECK_BOOL("glGetProgramiv()");

		// allocate enough space for the message and retrieve it
		char* pszInfoLog = new char[ui32InfoLogLength];
		glGetProgramInfoLog(*program, ui32InfoLogLength, &ui32CharsWritten, pszInfoLog);
		ERROR_CHECK_BOOL("glGetProgramInfoLog()");

		// displays the error
		printf("Failed to link program: %s\n", pszInfoLog);

		// freeing memory
		delete [] pszInfoLog;
		glDeleteProgram(*program);
		ERROR_CHECK_BOOL("glDeleteProgram()");
		*program = 0;
		return false;
	}

	return true;

#else

	return false;

#endif
}

bool MMSFBGL::buildShaderProgram(const char *fragment_shader_code, const char *vertex_shader_code, GLuint *program) {

#ifdef __HAVE_GLES2__

	// create new fragment and vertex shader and attach it to a program
	GLuint fshader;
	GLuint vshader;

	if (buildShader(MMSFBGL_SHADER_TYPE_FRAGMENT_SHADER, fragment_shader_code, &fshader)) {
		if (buildShader(MMSFBGL_SHADER_TYPE_VERTEX_SHADER, vertex_shader_code, &vshader)) {
			if (linkProgram(fshader, vshader, program)) {
				glDeleteShader(fshader);
				ERROR_CHECK_BOOL("glDeleteShader(fshader)");
				glDeleteShader(vshader);
				ERROR_CHECK_BOOL("glDeleteShader(vshader)");
				return true;
			}
			else {
				glDeleteShader(fshader);
				ERROR_CHECK_BOOL("glDeleteShader(fshader)");
				glDeleteShader(vshader);
				ERROR_CHECK_BOOL("glDeleteShader(vshader)");
			}
		}
		else {
			glDeleteShader(fshader);
			ERROR_CHECK_BOOL("glDeleteShader(fshader)");
		}
	}

#endif

	return false;
}

bool MMSFBGL::buildShaderProgram4Drawing(GLuint *program) {
	// fragment and vertex shaders code
	const char* fragment_shader =
		"uniform mediump vec4 FSColor;	\n"
		"void main (void)				\n"
		"{								\n"
		"	gl_FragColor = FSColor;		\n"
		"}								\n";

	const char* vertex_shader =
		"attribute highp vec4	VSVertex;		\n"
		"uniform mediump mat4	VSMatrix;		\n"
		"void main(void)						\n"
		"{										\n"
		"	gl_Position = VSMatrix * VSVertex;	\n"
		"}										\n";

	return buildShaderProgram(fragment_shader, vertex_shader, program);
}

bool MMSFBGL::buildShaderProgram4Blitting(GLuint *program) {
	// fragment and vertex shaders code
	const char* fragment_shader =
		"precision mediump float;								\n"
		"varying vec2 v_texCoord;								\n"
		"uniform sampler2D FSTexture;							\n"
		"void main()											\n"
		"{														\n"
		"	gl_FragColor = texture2D(FSTexture, v_texCoord);	\n"
		"}														\n";

	const char* vertex_shader =
		"attribute vec4 VSVertex;				\n"
		"attribute vec2 VSTexCoord;				\n"
		"varying vec2 v_texCoord;				\n"
		"uniform mediump mat4 VSMatrix;			\n"
		"void main()							\n"
		"{										\n"
		"   gl_Position = VSMatrix * VSVertex;	\n"
		"   v_texCoord = VSTexCoord;			\n"
		"}										\n";

	return buildShaderProgram(fragment_shader, vertex_shader, program);
}


bool MMSFBGL::buildShaderProgram4ModulateBlitting(GLuint *program) {
	// fragment and vertex shaders code
	const char* fragment_shader =
		"precision mediump float;										\n"
		"varying vec2 v_texCoord;										\n"
		"uniform sampler2D FSTexture;									\n"
		"uniform mediump vec4 FSColor;									\n"
		"void main()													\n"
		"{																\n"
		"	gl_FragColor = FSColor * texture2D(FSTexture, v_texCoord);	\n"
		"}																\n";

	const char* vertex_shader =
		"attribute vec4 VSVertex;				\n"
		"attribute vec2 VSTexCoord;				\n"
		"varying vec2 v_texCoord;				\n"
		"uniform mediump mat4 VSMatrix;			\n"
		"void main()							\n"
		"{										\n"
		"   gl_Position = VSMatrix * VSVertex;	\n"
		"   v_texCoord = VSTexCoord;			\n"
		"}										\n";

	return buildShaderProgram(fragment_shader, vertex_shader, program);
}


bool MMSFBGL::buildShaderProgram4BlittingFromAlpha(GLuint *program) {
	// fragment and vertex shaders code
	const char* fragment_shader =
		"precision mediump float;												\n"
		"varying vec2 v_texCoord;												\n"
		"uniform sampler2D FSTexture;											\n"
		"uniform mediump vec4 FSColor;											\n"
		"void main()															\n"
		"{																		\n"
		"   vec4 baseColor = texture2D(FSTexture, v_texCoord);          		\n"
		"	gl_FragColor = vec4(FSColor.r, FSColor.g, FSColor.b, baseColor.a);	\n"
		"}																		\n";

	const char* vertex_shader =
		"attribute vec4 VSVertex;				\n"
		"attribute vec2 VSTexCoord;				\n"
		"varying vec2 v_texCoord;				\n"
		"uniform mediump mat4 VSMatrix;			\n"
		"void main()							\n"
		"{										\n"
		"   gl_Position = VSMatrix * VSVertex;	\n"
		"   v_texCoord = VSTexCoord;			\n"
		"}										\n";

	return buildShaderProgram(fragment_shader, vertex_shader, program);
}


bool MMSFBGL::buildShaderProgram4ModulateBlittingFromAlpha(GLuint *program) {
	// fragment and vertex shaders code
	const char* fragment_shader =
		"precision mediump float;															\n"
		"varying vec2 v_texCoord;															\n"
		"uniform sampler2D FSTexture;														\n"
		"uniform mediump vec4 FSColor;														\n"
		"void main()																		\n"
		"{																					\n"
		"   vec4 baseColor = texture2D(FSTexture, v_texCoord);          					\n"
		"	gl_FragColor = vec4(FSColor.r, FSColor.g, FSColor.b, FSColor.a * baseColor.a);	\n"
		"}																					\n";

	const char* vertex_shader =
		"attribute vec4 VSVertex;				\n"
		"attribute vec2 VSTexCoord;				\n"
		"varying vec2 v_texCoord;				\n"
		"uniform mediump mat4 VSMatrix;			\n"
		"void main()							\n"
		"{										\n"
		"   gl_Position = VSMatrix * VSVertex;	\n"
		"   v_texCoord = VSTexCoord;			\n"
		"}										\n";

	return buildShaderProgram(fragment_shader, vertex_shader, program);
}


bool MMSFBGL::initShaders() {

#ifdef __HAVE_GLES2__

	printf("initializing shaders...\n");

	// current program object not set
	this->po_current = 0;

	// build all available shaders...
	buildShaderProgram4Drawing(&this->po_draw);
	buildShaderProgram4Blitting(&this->po_blit);
	buildShaderProgram4ModulateBlitting(&this->po_modulateblit);
	buildShaderProgram4BlittingFromAlpha(&this->po_blit_fromalpha);
	buildShaderProgram4ModulateBlittingFromAlpha(&this->po_modulateblit_fromalpha);

    return true;

#else

    return false;

#endif
}

void MMSFBGL::deleteShaders() {

#ifdef __HAVE_GLES2__

	if (this->po_draw) {
		glDeleteProgram(this->po_draw);
		ERROR_CHECK_VOID("glDeleteProgram(this->po_draw)");
		this->po_draw = 0;
	}

	if (this->po_blit) {
		glDeleteProgram(this->po_blit);
		ERROR_CHECK_VOID("glDeleteProgram(this->po_blit)");
		this->po_blit = 0;
	}

	if (this->po_modulateblit) {
		glDeleteProgram(this->po_modulateblit);
		ERROR_CHECK_VOID("glDeleteProgram(this->po_modulateblit)");
		this->po_modulateblit = 0;
	}

	if (this->po_blit_fromalpha) {
		glDeleteProgram(this->po_blit_fromalpha);
		ERROR_CHECK_VOID("glDeleteProgram(this->po_blit_fromalpha)");
		this->po_blit_fromalpha = 0;
	}

	if (this->po_modulateblit_fromalpha) {
		glDeleteProgram(this->po_modulateblit_fromalpha);
		ERROR_CHECK_VOID("glDeleteProgram(this->po_modulateblit_fromalpha)");
		this->po_modulateblit_fromalpha = 0;
	}

	this->po_current = 0;

#endif
}


#ifdef __HAVE_XLIB__
bool MMSFBGL::init(Display *x_display, int x_screen, Window x_window, int w, int h) {

	if (this->initialized) {
		// already initialized
		return false;
	}

	printf("initializing...\n");

#ifdef __HAVE_EGL__
	//TODO: implement EGL for XLIB
	return false;
#endif

#ifdef __HAVE_GLX__

	this->x_display = x_display;
	this->x_window = x_window;

	int glxMajor, glxMinor;
	glXQueryVersion(x_display, &glxMajor, &glxMinor);
	printf("GLX-Version %d.%d\n", glxMajor, glxMinor);
	int attribList[] =
		{GLX_RGBA,
		GLX_RED_SIZE, 8,
		GLX_GREEN_SIZE, 8,
		GLX_BLUE_SIZE, 8,
		GLX_DEPTH_SIZE, 16,
		GLX_DOUBLEBUFFER,
		None};
	this->xvi = glXChooseVisual(x_display, x_screen, attribList);
	if (this->xvi == NULL) {
		int attribList[] =
				{GLX_RGBA,
				GLX_RED_SIZE, 8,
				GLX_GREEN_SIZE, 8,
				GLX_BLUE_SIZE, 8,
				None};
		this->xvi = glXChooseVisual(x_display, x_screen, attribList);
		printf("singlebuffered rendering will be used, no doublebuffering available\n");
		if(this->xvi == NULL) {
			printf("shit happens.... \n");
			return false;
		}
	}
	else {
		printf("doublebuffered rendering available\n");
	}

	// create a GLX context
	this->glx_context = glXCreateContext(x_display, this->xvi, 0, GL_TRUE);
	if (!this->glx_context) {
		printf("context generation failed...\n");
		return false;
	}

	if(glXMakeCurrent(x_display, x_window, this->glx_context) != True) {
		printf("make current failed\n");
		return false;
	}

	if (glXIsDirect(x_display, this->glx_context))
		printf("DRI enabled\n");
	else
		printf("no DRI available\n");

	XMapRaised(x_display, x_window);
	XFlush(x_display);

	// init extension pointers
	GLenum err=glewInit();
	if(err!=GLEW_OK) {
		//problem: glewInit failed, something is seriously wrong
		printf("Error: %s\n", glewGetErrorString(err));
		return false;
	}

	// wrapper successfully initialized
	this->initialized = true;
	this->screen_width = w;
	this->screen_height = h;
	printf("SCREEN WIDTH = %d, HEIGHT = %d\n", this->screen_width, this->screen_height);

	return true;

#else
	return false;
#endif
}

#else

bool MMSFBGL::init() {

	if (this->initialized) {
		// already initialized
		return false;
	}

	printf("initializing...\n");

#ifdef __HAVE_EGL__
	/*
		Step 1 - Get the default display.
		EGL uses the concept of a "display" which in most environments
		corresponds to a single physical screen. Since we usually want
		to draw to the main screen or only have a single screen to begin
		with, we let EGL pick the default display.
		Querying other displays is platform specific.
	*/
	eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (eglDisplay == EGL_NO_DISPLAY) {
		printf("Error: eglGetDisplay() returned EGL_NO_DISPLAY.\n");
		terminate();
		return false;
	}

	/*
		Step 2 - Initialize EGL.
		EGL has to be initialized with the display obtained in the
		previous step. We cannot use other EGL functions except
		eglGetDisplay and eglGetError before eglInitialize has been
		called.
		If we're not interested in the EGL version number we can just
		pass NULL for the second and third parameters.
	*/
	EGLint iMajorVersion, iMinorVersion;
	if (!eglInitialize(eglDisplay, &iMajorVersion, &iMinorVersion))
	{
		printf("Error: eglInitialize() failed.\n");
		terminate();
		return false;
	}

	/*
		Step 3 - Make OpenGL ES the current API.
		EGL provides ways to set up OpenGL ES and OpenVG contexts
		(and possibly other graphics APIs in the future), so we need
		to specify the "current API".
	*/
	eglBindAPI(EGL_OPENGL_ES_API);

	if (eglGetError() != EGL_SUCCESS)
	if (!getError("eglBindAPI"))
	{
		terminate();
		return false;
	}

	/*
		Step 4 - Specify the required configuration attributes.
		An EGL "configuration" describes the pixel format and type of
		surfaces that can be used for drawing.
		For now we just want to use a 16 bit RGB surface that is a
		Window surface, i.e. it will be visible on screen. The list
		has to contain key/value pairs, terminated with EGL_NONE.
	 */
	EGLint pi32ConfigAttribs[5];
	pi32ConfigAttribs[0] = EGL_SURFACE_TYPE;
	pi32ConfigAttribs[1] = EGL_WINDOW_BIT;
	pi32ConfigAttribs[2] = EGL_RENDERABLE_TYPE;
	pi32ConfigAttribs[3] = EGL_OPENGL_ES2_BIT;
	pi32ConfigAttribs[4] = EGL_NONE;

	/*
		Step 5 - Find a config that matches all requirements.
		eglChooseConfig provides a list of all available configurations
		that meet or exceed the requirements given as the second
		argument. In most cases we just want the first config that meets
		all criteria, so we can limit the number of configs returned to 1.
	*/
	int iConfigs;
	if (!eglChooseConfig(eglDisplay, pi32ConfigAttribs, eglConfig, 10, &iConfigs) || (iConfigs < 1))
	{
		printf("Error: eglChooseConfig() failed.\n");
		terminate();
		return false;
	}
	else {
		for (int i = 0; i < iConfigs; i++) {
		   EGLint value;
		   printf("config #%d ***\n", i);
		   GET_ATTR(EGL_BUFFER_SIZE, "EGL_BUFFER_SIZE");
		   GET_ATTR(EGL_ALPHA_SIZE, "EGL_ALPHA_SIZE");
		   GET_ATTR(EGL_BLUE_SIZE, "EGL_BLUE_SIZE");
		   GET_ATTR(EGL_GREEN_SIZE, "EGL_GREEN_SIZE");
		   GET_ATTR(EGL_RED_SIZE, "EGL_RED_SIZE");
		   GET_ATTR(EGL_DEPTH_SIZE, "EGL_DEPTH_SIZE");
		   GET_ATTR(EGL_STENCIL_SIZE, "EGL_STENCIL_SIZE");
		   GET_ATTR(EGL_CONFIG_CAVEAT, "EGL_CONFIG_CAVEAT");
		   GET_ATTR(EGL_CONFIG_ID, "EGL_CONFIG_ID");
		   GET_ATTR(EGL_LEVEL, "EGL_LEVEL");
		   GET_ATTR(EGL_MAX_PBUFFER_HEIGHT, "EGL_MAX_PBUFFER_HEIGHT");
		   GET_ATTR(EGL_MAX_PBUFFER_PIXELS, "EGL_MAX_PBUFFER_PIXELS");
		   GET_ATTR(EGL_MAX_PBUFFER_WIDTH, "EGL_MAX_PBUFFER_WIDTH");
		   GET_ATTR(EGL_NATIVE_RENDERABLE, "EGL_NATIVE_RENDERABLE");
		   GET_ATTR(EGL_NATIVE_VISUAL_ID, "EGL_NATIVE_VISUAL_ID");
		   GET_ATTR(EGL_NATIVE_VISUAL_TYPE, "EGL_NATIVE_VISUAL_TYPE");
		   GET_ATTR(EGL_PRESERVED_RESOURCES, "EGL_PRESERVED_RESOURCES");
		   GET_ATTR(EGL_SAMPLES, "EGL_SAMPLES");
		   GET_ATTR(EGL_SAMPLE_BUFFERS, "EGL_SAMPLE_BUFFERS");
		   GET_ATTR(EGL_SURFACE_TYPE, "EGL_SURFACE_TYPE");
		   GET_ATTR(EGL_TRANSPARENT_TYPE, "EGL_TRANSPARENT_TYPE");
		   GET_ATTR(EGL_TRANSPARENT_BLUE_VALUE, "EGL_TRANSPARENT_BLUE_VALUE");
		   GET_ATTR(EGL_TRANSPARENT_GREEN_VALUE, "EGL_TRANSPARENT_GREEN_VALUE");
		   GET_ATTR(EGL_TRANSPARENT_RED_VALUE, "EGL_TRANSPARENT_RED_VALUE");
		   GET_ATTR(EGL_NONE, "EGL_NONE");
		   GET_ATTR(EGL_BIND_TO_TEXTURE_RGB, "EGL_BIND_TO_TEXTURE_RGB");
		   GET_ATTR(EGL_BIND_TO_TEXTURE_RGBA, "EGL_BIND_TO_TEXTURE_RGBA");
		   GET_ATTR(EGL_MIN_SWAP_INTERVAL, "EGL_MIN_SWAP_INTERVAL");
		   GET_ATTR(EGL_MAX_SWAP_INTERVAL, "EGL_MAX_SWAP_INTERVAL");
		   GET_ATTR(EGL_LUMINANCE_SIZE, "EGL_LUMINANCE_SIZE");
		   GET_ATTR(EGL_ALPHA_MASK_SIZE, "EGL_ALPHA_MASK_SIZE");
		   GET_ATTR(EGL_COLOR_BUFFER_TYPE, "EGL_COLOR_BUFFER_TYPE");
		   GET_ATTR(EGL_RENDERABLE_TYPE, "EGL_RENDERABLE_TYPE");
		   GET_ATTR(EGL_MATCH_NATIVE_PIXMAP, "EGL_MATCH_NATIVE_PIXMAP");
		   GET_ATTR(EGL_CONFORMANT, "EGL_CONFORMANT");
		}
	}


	/*
		Step 6 - Create a surface to draw to.
		Use the config picked in the previous step and the native window
		handle when available to create a window surface. A window surface
		is one that will be visible on screen inside the native display (or
		fullscreen if there is no windowing system).
		Pixmaps and pbuffers are surfaces which only exist in off-screen
		memory.
	*/
	eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig[0], (EGLNativeWindowType) NULL, NULL);

	if (!getError("eglCreateWindowSurface"))
	{
		terminate();
		return false;
	}

	/*
		Step 7 - Create a context.
		EGL has to create a context for OpenGL ES. Our OpenGL ES resources
		like textures will only be valid inside this context
		(or shared contexts).
	*/
	EGLint ai32ContextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
	eglContext = eglCreateContext(eglDisplay, eglConfig[0], NULL, ai32ContextAttribs);
	if (!getError("eglCreateContext"))
	{
		terminate();
		return false;
	}

	/*
		Step 8 - Bind the context to the current thread and use our
		window surface for drawing and reading.
		Contexts are bound to a thread. This means you don't have to
		worry about other threads and processes interfering with your
		OpenGL ES application.
		We need to specify a surface that will be the target of all
		subsequent drawing operations, and one that will be the source
		of read operations. They can be the same surface.
	*/
	eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);
	if (!getError("eglMakeCurrent"))
	{
		terminate();
		return false;
	}

	/*
		Step 9 - Draw something with OpenGL ES.
		At this point everything is initialized and we're ready to use
		OpenGL ES to draw something on the screen.
	*/



	// get the dimension of the screen
	int wh[4];
	glGetIntegerv(GL_VIEWPORT, wh);
	this->screen_width = wh[2];
	this->screen_height = wh[3];
	printf("SCREEN WIDTH = %d, HEIGHT = %d\n", this->screen_width, this->screen_height);


	// init fragment and vertex shaders
	if (initShaders()) {

		// prepare current matrix for shaders
		loadIdentityMatrix(this->current_matrix);

		// prepare current color for shaders
		this->current_color_r = 0;
		this->current_color_g = 0;
		this->current_color_b = 0;
		this->current_color_a = 0;

		// wrapper successfully initialized
		this->initialized = true;
	}

    return true;

#else

    return false;

#endif
}
#endif



bool MMSFBGL::terminate() {

	INITCHECK;

	deleteShaders();

#ifdef __HAVE_EGL__

	/*
		Step 10 - Terminate OpenGL ES and destroy the window (if present).
		eglTerminate takes care of destroying any context or surface created
		with this display, so we don't need to call eglDestroySurface or
		eglDestroyContext here.
	*/
	eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglTerminate(eglDisplay);

#endif

	return true;
}


bool MMSFBGL::getResolution(int *w, int *h) {

	INITCHECK;

	*w = this->screen_width;
	*h = this->screen_height;

	return true;
}


bool MMSFBGL::swap() {

	INITCHECK;

#ifdef __HAVE_GLX__

	glXSwapBuffers(this->x_display, this->x_window);

	return true;

#endif

#ifdef __HAVE_EGL__

	eglSwapBuffers(this->eglDisplay, this->eglSurface);
	ERROR_CHECK_BOOL("eglSwapBuffers()");

	return true;

#else

	return false;

#endif
}



bool MMSFBGL::genTexture(GLuint *tex) {

	INITCHECK;

	// generate a unique texture id
	glGenTextures(1, tex);
	ERROR_CHECK_BOOL("glGenTextures()");

    return true;
}

bool MMSFBGL::deleteTexture(GLuint tex) {

	INITCHECK;

	if (tex) {
		// finishing all operations
		glFinish();
		ERROR_CHECK_BOOL("glFinish()");

		// switch to the primary frame buffer
		// so OpenGL have to finish tasks which are not finished during glFinish()
		GLuint fbo = this->bound_fbo;
		bindFrameBuffer(0);

#ifdef __HAVE_GL2__
		// disabling GL_TEXTURE_2D only useful for the fixed-function pipeline (not for own shaders)
		glDisable(GL_TEXTURE_2D);
		ERROR_CHECK_BOOL("glDisable(GL_TEXTURE_2D)");
#endif

		// detach texture
		glBindTexture(GL_TEXTURE_2D, 0);
		ERROR_CHECK_BOOL("glBindTexture(GL_TEXTURE_2D, 0)");

		// now is safe to delete the texture
		glDeleteTextures(1, &tex);
		ERROR_CHECK_BOOL("glDeleteTextures()");

		// switch back to the saved fbo
		bindFrameBuffer(fbo);
	}

	return true;
}

bool MMSFBGL::bindTexture2D(GLuint tex) {

	INITCHECK;

	// flush all queued commands to the OpenGL server
	// but do NOT wait until all queued commands are finished by the OpenGL server
	glFlush();
	ERROR_CHECK_BOOL("glFlush()");

	// activate texture
    glBindTexture(GL_TEXTURE_2D, tex);
	ERROR_CHECK_BOOL("glBindTexture(GL_TEXTURE_2D, tex)");

    // set min and max filter
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	ERROR_CHECK_BOOL("glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST)");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	ERROR_CHECK_BOOL("glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST)");

    // the texture wraps over at the edges (repeat)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	ERROR_CHECK_BOOL("glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE)");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	ERROR_CHECK_BOOL("glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE)");

    return true;
}

bool MMSFBGL::initTexture2D(GLuint tex, GLenum texture_format, void *buffer, GLenum buffer_format, int sw, int sh) {

	INITCHECK;

	// activate texture
	bindTexture2D(tex);

    // initializing texture from buffer
	glTexImage2D(GL_TEXTURE_2D,
	 	0,
	 	texture_format,
	 	sw,
	 	sh,
	 	0,
	 	buffer_format,
	 	GL_UNSIGNED_BYTE,
	 	buffer);
	ERROR_CHECK_BOOL("glTexImage2D(GL_TEXTURE_2D,...)");

	return true;
}


bool MMSFBGL::initSubTexture2D(GLuint tex, void *buffer, GLenum buffer_format, int sw, int sh, int dx, int dy) {

	INITCHECK;

	// activate texture
	bindTexture2D(tex);

	// overwrite existing texture memory
	glTexSubImage2D(GL_TEXTURE_2D,
					0,
					dx,
					dy,
					sw,
					sh,
					buffer_format,
					GL_UNSIGNED_BYTE,
					buffer);
	ERROR_CHECK_BOOL("glTexSubImage2D(GL_TEXTURE_2D,...)");

	return true;
}

bool MMSFBGL::enableTexture2D(GLuint tex) {

#ifdef __HAVE_GL2__
	// enabling GL_TEXTURE_2D only useful for the fixed-function pipeline (not for own shaders)
	glEnable(GL_TEXTURE_2D);
	ERROR_CHECK_BOOL("glEnable(GL_TEXTURE_2D)");
#endif

	// bind source texture
	bindTexture2D(tex);

#ifdef __HAVE_GLES2__

	if (!this->FSTextureLoc_initialized) {
		// get the location of source texture uniform variable within the fragment shader
		this->FSTextureLoc = -1;
		if (this->po_current) {
			this->FSTextureLoc = glGetUniformLocation(this->po_current, "FSTexture");
			ERROR_CHECK_BOOL("glGetUniformLocation(this->po_current, \"FSTexture\")");

			this->FSTextureLoc_initialized = true;
		}
	}

	if (!this->VSTexCoordLoc_initialized) {
		// get the location of texture coordinates attribute variable within the vertex shader
		this->VSTexCoordLoc = -1;
		if (this->po_current) {
			this->VSTexCoordLoc = glGetAttribLocation(this->po_current, "VSTexCoord");
			ERROR_CHECK_BOOL("glGetAttribLocation(this->po_current, \"VSTexCoord\")");

			this->VSTexCoordLoc_initialized = true;
		}
	}

#endif

	return true;
}


void MMSFBGL::disableTexture2D() {
#ifdef __HAVE_GL2__
	// disabling GL_TEXTURE_2D only useful for the fixed-function pipeline (not for own shaders)
	glDisable(GL_TEXTURE_2D);
	ERROR_CHECK_VOID("glDisable(GL_TEXTURE_2D)");
#endif
}


bool MMSFBGL::genFrameBuffer(GLuint *fbo) {

	INITCHECK;

	// generate a unique FBO id
#ifdef __HAVE_GL2__
	glGenFramebuffersEXT(1, fbo);
	ERROR_CHECK_BOOL("glGenFramebuffersEXT(1, fbo)");
#endif

#ifdef __HAVE_GLES2__
	glGenFramebuffers(1, fbo);
	ERROR_CHECK_BOOL("glGenFramebuffers(1, fbo)");
#endif

    return true;
}


bool MMSFBGL::deleteFrameBuffer(GLuint fbo) {

	INITCHECK;

	// finishing all operations
	glFinish();
	ERROR_CHECK_BOOL("glFinish()");

	// switch to the primary frame buffer
	// so OpenGL have to finish tasks which are not finished during glFinish()
	bindFrameBuffer(0);

#ifdef  __HAVE_GL2__
	if (fbo) {
		glDeleteFramebuffersEXT(1, &fbo);
		ERROR_CHECK_BOOL("glDeleteFramebuffersEXT()");
	}
#endif

#ifdef __HAVE_GLES2__
	if (fbo) {
		glDeleteFramebuffers(1, &fbo);
		ERROR_CHECK_BOOL("glDeleteFramebuffers()");
	}
#endif

	return true;
}


bool MMSFBGL::genRenderBuffer(GLuint *rbo) {

	INITCHECK;

	// generate a unique RBO id
#ifdef __HAVE_GL2__
	glGenRenderbuffersEXT(1, rbo);
	ERROR_CHECK_BOOL("glGenRenderbuffersEXT()");
#endif

#ifdef __HAVE_GLES2__
	glGenRenderbuffers(1, rbo);
	ERROR_CHECK_BOOL("glGenRenderbuffers()");
#endif

    return true;
}


bool MMSFBGL::deleteRenderBuffer(GLuint rbo) {

	INITCHECK;

	// finishing all operations
	glFinish();
	ERROR_CHECK_BOOL("glFinish()");

	// switch to the primary frame buffer
	// so OpenGL have to finish tasks which are not finished during glFinish()
	bindFrameBuffer(0);

#ifdef  __HAVE_GL2__
	if (rbo) {
		glDeleteRenderbuffersEXT(1, &rbo);
		ERROR_CHECK_BOOL("glDeleteRenderbuffersEXT()");
	}
#endif

#ifdef __HAVE_GLES2__
	if (rbo) {
		glDeleteRenderbuffers(1, &rbo);
		ERROR_CHECK_BOOL("glDeleteRenderbuffers()");
	}
#endif

	return true;
}


bool MMSFBGL::attachTexture2FrameBuffer(GLuint fbo, GLuint tex) {

	INITCHECK;

#ifdef  __HAVE_GL2__
	bindFrameBuffer(fbo);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, tex, 0);
	ERROR_CHECK_BOOL("glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, tex, 0)");

	if (glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT) {
		// the GPU does not support current FBO configuration
		printf("MMSFBGL: fatal error while attaching texture to framebuffer failed (GL2)\n");
		return false;
	}
#endif

#ifdef __HAVE_GLES2__
	bindFrameBuffer(fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
	ERROR_CHECK_BOOL("glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0)");

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		// the GPU does not support current FBO configuration
		printf("MMSFBGL: fatal error while attaching texture to framebuffer failed (GLES2)\n");
		return false;
	}
#endif

	return true;
}


bool MMSFBGL::attachRenderBuffer2FrameBuffer(GLuint fbo, GLuint rbo, int width, int height) {

	INITCHECK;

#ifdef  __HAVE_GL2__
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, rbo);
	ERROR_CHECK_BOOL("glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, rbo)");

	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, width, height);
	ERROR_CHECK_BOOL("glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, width, height)");

	bindFrameBuffer(fbo);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, rbo);
	ERROR_CHECK_BOOL("glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, rbo)");

	if (glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT) {
		// the GPU does not support current FBO configuration
		printf("MMSFBGL: fatal error while attaching renderbuffer to framebuffer failed (GL2)\n");
		return false;
	}
#endif

#ifdef __HAVE_GLES2__
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	ERROR_CHECK_BOOL("glBindRenderbuffer(GL_RENDERBUFFER, rbo)");

	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
	ERROR_CHECK_BOOL("glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height)");

	bindFrameBuffer(fbo);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
	ERROR_CHECK_BOOL("glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo)");

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		// the GPU does not support current FBO configuration
		printf("MMSFBGL: fatal error while attaching renderbuffer to framebuffer failed (GLES2)\n");
		return false;
	}
#endif

	return true;
}


bool MMSFBGL::allocTexture(GLuint tex, int width, int height) {

	INITCHECK;

	if (!initTexture2D(tex, GL_RGBA, NULL, GL_RGBA, width, height))
		return false;

	return true;
}


bool MMSFBGL::allocFBO(GLuint fbo, GLuint tex, int width, int height) {

	INITCHECK;

	if (!allocTexture(tex, width, height))
		return false;

	if (!attachTexture2FrameBuffer(fbo, tex))
		return false;

	return true;
}


bool MMSFBGL::allocFBOandRBO(GLuint fbo, GLuint tex, GLuint rbo, int width, int height) {

	INITCHECK;

	if (!allocTexture(tex, width, height))
		return false;

	if (!attachTexture2FrameBuffer(fbo, tex))
		return false;

	if (!attachRenderBuffer2FrameBuffer(fbo, rbo, width, height))
		return false;

	return true;
}



bool MMSFBGL::freeFBO(GLuint fbo, GLuint tex, GLuint rbo) {

	INITCHECK;

	bindFrameBuffer(0);
	deleteRenderBuffer(rbo);
	deleteTexture(tex);
	deleteFrameBuffer(fbo);

	return true;
}


bool MMSFBGL::bindFrameBuffer(GLuint fbo) {

	INITCHECK;

	if (this->bound_fbo != fbo) {
		// going to change the framebuffer object
		this->bound_fbo = fbo;

#ifdef  __HAVE_GL2__
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
		ERROR_CHECK_BOOL("glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo)");
#endif

#ifdef __HAVE_GLES2__
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		ERROR_CHECK_BOOL("glBindFramebuffer(GL_FRAMEBUFFER, fbo)");
#endif
	}

	disableScissor();
	disableDepthTest();
	disableTexture2D();
	disableArrays();

	return true;
}


bool MMSFBGL::setScissor(GLint x, GLint y, GLsizei width, GLsizei height) {

	INITCHECK;

	glScissor(x, y, width, height);
	ERROR_CHECK_BOOL("glScissor()");
	glEnable(GL_SCISSOR_TEST);
	ERROR_CHECK_BOOL("glEnable(GL_SCISSOR_TEST)");

	return true;
}


bool MMSFBGL::disableScissor() {

	INITCHECK;

	glDisable(GL_SCISSOR_TEST);
	ERROR_CHECK_BOOL("glDisable(GL_SCISSOR_TEST)");

	return true;
}


void MMSFBGL::enableBlend(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha) {
	glEnable(GL_BLEND);
	ERROR_CHECK_VOID("glEnable(GL_BLEND)");
	glBlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);
	ERROR_CHECK_VOID("glBlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha)");
}


void MMSFBGL::disableBlend() {
	glDisable(GL_BLEND);
	ERROR_CHECK_VOID("glDisable(GL_BLEND)");
}


void MMSFBGL::enableDepthTest(bool readonly) {
	glEnable(GL_DEPTH_TEST);
	ERROR_CHECK_VOID("glEnable(GL_DEPTH_TEST)");

	if (!readonly) {
		// enable opengl to write into the depth buffer
		glDepthMask(GL_TRUE);
		ERROR_CHECK_VOID("glDepthMask(GL_TRUE)");
	}
	else {
		// write operations to depth buffer are not allowed
		glDepthMask(GL_FALSE);
		ERROR_CHECK_VOID("glDepthMask(GL_FALSE)");
	}

#ifdef __HAVE_GL2__
	glDepthRange(0, 1);
	ERROR_CHECK_VOID("glDepthRange(0, 1)");
#endif
#ifdef __HAVE_GLES2__
	glDepthRangef(0, 1);
	ERROR_CHECK_VOID("glDepthRangef(0, 1)");
#endif
}

void MMSFBGL::disableDepthTest() {
	glDisable(GL_DEPTH_TEST);
	ERROR_CHECK_VOID("glDisable(GL_DEPTH_TEST)");

	glDepthMask(GL_FALSE);
	ERROR_CHECK_VOID("glDepthMask(GL_FALSE)");
}

void MMSFBGL::setDrawingMode() {
#ifdef __HAVE_GLES2__
	useShaderProgram4Drawing();
#endif
}

void MMSFBGL::setTexEnvReplace(GLenum format) {
#ifdef __HAVE_GL2__
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	ERROR_CHECK_VOID("glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE)");
#endif

#ifdef __HAVE_GLES2__
	if (format != GL_ALPHA) {
		useShaderProgram4Blitting();
	}
	else {
		useShaderProgram4BlittingFromAlpha();
	}
#endif
}

void MMSFBGL::setTexEnvModulate(GLenum format) {
#ifdef __HAVE_GL2__
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	ERROR_CHECK_VOID("glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE)");
#endif

#ifdef __HAVE_GLES2__
	if (format != GL_ALPHA) {
		useShaderProgram4ModulateBlitting();
	}
	else {
		useShaderProgram4ModulateBlittingFromAlpha();
	}
#endif
}


void MMSFBGL::disableArrays() {
#ifdef  __HAVE_GL2__
	glDisableClientState(GL_VERTEX_ARRAY);
	ERROR_CHECK_VOID("glDisableClientState(GL_VERTEX_ARRAY)");

	glDisableClientState(GL_NORMAL_ARRAY);
	ERROR_CHECK_VOID("glDisableClientState(GL_NORMAL_ARRAY)");

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	ERROR_CHECK_VOID("glDisableClientState(GL_TEXTURE_COORD_ARRAY)");

	glDisableClientState(GL_INDEX_ARRAY);
	ERROR_CHECK_VOID("glDisableClientState(GL_INDEX_ARRAY)");
#endif
}


bool MMSFBGL::useShaderProgram4Drawing() {
	if (!this->po_draw)
		return false;

	if (this->po_current != this->po_draw) {
		this->po_current = this->po_draw;

		glUseProgram(this->po_current);
		ERROR_CHECK_BOOL("glUseProgram()");

		this->VSMatrixLoc_initialized = false;
		this->FSColorLoc_initialized = false;
		this->FSTextureLoc_initialized = false;
		this->VSTexCoordLoc_initialized = false;
	}

	setCurrentMatrix(this->current_matrix);
	setColor(this->current_color_r, this->current_color_g, this->current_color_b, this->current_color_a);

	return true;
}

bool MMSFBGL::useShaderProgram4Blitting() {
	if (!this->po_blit)
		return false;

	if (this->po_current != this->po_blit) {
		this->po_current = this->po_blit;

		glUseProgram(this->po_current);
		ERROR_CHECK_BOOL("glUseProgram()");

		this->VSMatrixLoc_initialized = false;
		this->FSColorLoc_initialized = false;
		this->FSTextureLoc_initialized = false;
		this->VSTexCoordLoc_initialized = false;
	}

	setCurrentMatrix(this->current_matrix);
	setColor(this->current_color_r, this->current_color_g, this->current_color_b, this->current_color_a);

	return true;
}

bool MMSFBGL::useShaderProgram4ModulateBlitting() {
	if (!this->po_modulateblit)
		return false;

	if (this->po_current != this->po_modulateblit) {
		this->po_current = this->po_modulateblit;

		glUseProgram(this->po_current);
		ERROR_CHECK_BOOL("glUseProgram()");

		this->VSMatrixLoc_initialized = false;
		this->FSColorLoc_initialized = false;
		this->FSTextureLoc_initialized = false;
		this->VSTexCoordLoc_initialized = false;
	}

	setCurrentMatrix(this->current_matrix);
	setColor(this->current_color_r, this->current_color_g, this->current_color_b, this->current_color_a);

	return true;
}


bool MMSFBGL::useShaderProgram4BlittingFromAlpha() {
	if (!this->po_blit_fromalpha)
		return false;

	if (this->po_current != this->po_blit_fromalpha) {
		this->po_current = this->po_blit_fromalpha;

		glUseProgram(this->po_current);
		ERROR_CHECK_BOOL("glUseProgram()");

		this->VSMatrixLoc_initialized = false;
		this->FSColorLoc_initialized = false;
		this->FSTextureLoc_initialized = false;
		this->VSTexCoordLoc_initialized = false;
	}

	setCurrentMatrix(this->current_matrix);
	setColor(this->current_color_r, this->current_color_g, this->current_color_b, this->current_color_a);

	return true;
}


bool MMSFBGL::useShaderProgram4ModulateBlittingFromAlpha() {
	if (!this->po_modulateblit_fromalpha)
		return false;

	if (this->po_current != this->po_modulateblit_fromalpha) {
		this->po_current = this->po_modulateblit_fromalpha;

		glUseProgram(this->po_current);
		ERROR_CHECK_BOOL("glUseProgram()");

		this->VSMatrixLoc_initialized = false;
		this->FSColorLoc_initialized = false;
		this->FSTextureLoc_initialized = false;
		this->VSTexCoordLoc_initialized = false;
	}

	setCurrentMatrix(this->current_matrix);
	setColor(this->current_color_r, this->current_color_g, this->current_color_b, this->current_color_a);

	return true;
}


bool MMSFBGL::setCurrentMatrix(MMS3DMatrix matrix) {

	INITCHECK;

#ifdef __HAVE_GL2__

	glLoadMatrixf((GLfloat*)matrix);

#endif

#ifdef __HAVE_GLES2__

	if (!this->VSMatrixLoc_initialized) {
		// get the location of matrix uniform variable within the vertex shader
		this->VSMatrixLoc = -1;
		if (this->po_current) {
			this->VSMatrixLoc = glGetUniformLocation(this->po_current, "VSMatrix");
			ERROR_CHECK_BOOL("glGetUniformLocation(this->po_current, \"VSMatrix\")");

			this->VSMatrixLoc_initialized = true;
		}
	}

	if (this->VSMatrixLoc >= 0) {
		// store the new matrix for the vertex shader
		glUniformMatrix4fv(this->VSMatrixLoc, 1, GL_FALSE, (GLfloat*)matrix);
		ERROR_CHECK_BOOL("glUniformMatrix4fv(this->VSMatrixLoc, 1, GL_FALSE, (GLfloat*)matrix)");
	}

#endif

	// change the current matrix
	copyMatrix(this->current_matrix, matrix);

	return true;
}


bool MMSFBGL::getCurrentMatrix(MMS3DMatrix matrix) {

	INITCHECK;

	copyMatrix(matrix, this->current_matrix);

	return true;
}


bool MMSFBGL::scaleCurrentMatrix(GLfloat sx, GLfloat sy, GLfloat sz) {

	INITCHECK;

	scaleMatrix(this->current_matrix, sx, sy, sz);
	return setCurrentMatrix(this->current_matrix);
}


bool MMSFBGL::translateCurrentMatrix(GLfloat tx, GLfloat ty, GLfloat tz) {

	INITCHECK;

	translateMatrix(this->current_matrix, tx, ty, tz);
	return setCurrentMatrix(this->current_matrix);
}


bool MMSFBGL::rotateCurrentMatrix(GLfloat angle, GLfloat x, GLfloat y, GLfloat z) {

	INITCHECK;

	rotateMatrix(this->current_matrix, angle, x, y, z);
	return setCurrentMatrix(this->current_matrix);
}



bool MMSFBGL::getParallelProjectionMatrix(MMS3DMatrix result, float left, float right, float bottom, float top, float nearZ, float farZ) {

	INITCHECK;

	// calculate the new matrix
	MMS3DMatrix matrix;
	loadIdentityMatrix(matrix);
	orthoMatrix(matrix, left, right, bottom, top, nearZ, farZ);

	// return the matrix to the caller
	copyMatrix(result, matrix);

	return true;
}


bool MMSFBGL::getCentralProjectionMatrix(MMS3DMatrix result, float left, float right, float bottom, float top, float nearZ, float farZ) {

	INITCHECK;

	// calculate the new matrix
	MMS3DMatrix matrix;
	loadIdentityMatrix(matrix);
	frustumMatrix(matrix, left, right, bottom, top, nearZ, farZ);

	// return the matrix to the caller
	copyMatrix(result, matrix);

	return true;
}


bool MMSFBGL::getPerspectiveMatrix(MMS3DMatrix result, float fovy, float aspect, float nearZ, float farZ) {

	INITCHECK;

	// calculate the new matrix
	MMS3DMatrix matrix;
	loadIdentityMatrix(matrix);
	perspectiveMatrix(matrix, fovy, aspect, nearZ, farZ);

	// return the matrix to the caller
	copyMatrix(result, matrix);

	return true;
}



bool MMSFBGL::setParallelProjection(float left, float right, float bottom, float top, float nearZ, float farZ) {

	INITCHECK;

	// set the model view matrix for the shader
	MMS3DMatrix matrix;
	getParallelProjectionMatrix(matrix, left, right, bottom, top, nearZ, farZ);
	glViewport(0, 0, (left<right)?right-left:left-right, (bottom<top)?top-bottom:bottom-top);
	ERROR_CHECK_BOOL("glViewport()");
	return setCurrentMatrix(matrix);
}


bool MMSFBGL::setCentralProjection(float left, float right, float bottom, float top, float nearZ, float farZ) {

	INITCHECK;

	// set the projection matrix for the shader
	MMS3DMatrix matrix;
	getCentralProjectionMatrix(matrix, left, right, bottom, top, nearZ, farZ);
	glViewport(0, 0, (left<right)?right-left:left-right, (bottom<top)?top-bottom:bottom-top);
	ERROR_CHECK_BOOL("glViewport()");
	return setCurrentMatrix(matrix);
}


bool MMSFBGL::setPerspective(float fovy, float aspect, float nearZ, float farZ) {

	INITCHECK;

	// set the perspective (based on projection matrix) for the shader
	MMS3DMatrix matrix;
	getPerspectiveMatrix(matrix, fovy, aspect, nearZ, farZ);
	GLfloat w, h;
	h = tanf(fovy / 360.0f * MMS3D_PI) * nearZ;
	w = h * aspect;
	glViewport(0, 0, w*2, h*2);
	ERROR_CHECK_BOOL("glViewport()");
	return setCurrentMatrix(matrix);
}


bool MMSFBGL::pushCurrentMatrix() {

	INITCHECK;

	// save the current matrix on the matrix stack
	this->matrix_stack.push(MMSFBGLStackMatrix(this->current_matrix));
	return true;
}

bool MMSFBGL::popCurrentMatrix() {

	INITCHECK;

	if (this->matrix_stack.size() > 0) {
		// restore current matrix from stack
		MMS3DMatrix matrix;
		this->matrix_stack.top().getMatrix(matrix);
		this->matrix_stack.pop();
		setCurrentMatrix(matrix);
		return true;
	}

	return false;
}


bool MMSFBGL::clear(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {

	INITCHECK;

	// specify the clear value for the color buffer
	glClearColor((!r)?0:(r==0xff)?1:(float)r/255,
				 (!g)?0:(g==0xff)?1:(float)g/255,
				 (!b)?0:(b==0xff)?1:(float)b/255,
				 (!a)?0:(a==0xff)?1:(float)a/255);
	ERROR_CHECK_BOOL("glClearColor()");

#ifdef __HAVE_GL2__
	// specify the clear value for the depth buffer
	glClearDepth(1.0);
	ERROR_CHECK_BOOL("glClearDepth(1.0)");
#endif

#ifdef __HAVE_GLES2__
	// specify the clear value for the depth buffer
	glClearDepthf(1.0);
	ERROR_CHECK_BOOL("glClearDepthf(1.0)");
#endif

	// clear color and depth buffer if existent
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	ERROR_CHECK_BOOL("glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)");

	return true;
}


bool MMSFBGL::setColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {

	INITCHECK;

	// change the current color
	this->current_color_r = r;
	this->current_color_g = g;
	this->current_color_b = b;
	this->current_color_a = a;

#ifdef __HAVE_GL2__
	glColor4ub(r, g, b, a);
	ERROR_CHECK_BOOL("glColor4ub()");
	return true;
#endif

#ifdef __HAVE_GLES2__
	if (!this->FSColorLoc_initialized) {
		// get the location of color uniform variable within the fragment shader
		this->FSColorLoc = -1;
		if (this->po_current) {
			this->FSColorLoc = glGetUniformLocation(this->po_current, "FSColor");
			ERROR_CHECK_BOOL("glGetUniformLocation(this->po_current, \"FSColor\")");

			this->FSColorLoc_initialized = true;
		}
	}

	if (this->FSColorLoc >= 0) {
		// store the new color for the fragment shader
		glUniform4f(this->FSColorLoc,
					(!r)?0:(r==0xff)?1:(float)r/255,
					(!g)?0:(g==0xff)?1:(float)g/255,
					(!b)?0:(b==0xff)?1:(float)b/255,
					(!a)?0:(a==0xff)?1:(float)a/255);
		ERROR_CHECK_BOOL("glUniform4f(this->FSColorLoc,...)");
	}

	return true;
#endif
}



bool MMSFBGL::drawRectangle2D(float x1, float y1, float x2, float y2) {

	INITCHECK;

#ifdef __HAVE_GL2__

	glBegin(GL_LINE_STRIP);
		glVertex2f(x1, y1);
		glVertex2f(x2, y1);
		glVertex2f(x2, y2);
		glVertex2f(x1, y2);
		glVertex2f(x1, y1);
	glEnd();
	ERROR_CHECK_BOOL("glBegin(GL_LINE_STRIP)");

#endif

#ifdef __HAVE_GLES2__

	// configure generic vertex attribute array
	GLfloat vertices[] = {x1,y1,x2,y1,x2,y2,x1,y2,x1,y1};
	glEnableVertexAttribArray(MMSFBGL_VSV_LOC);
	ERROR_CHECK_BOOL("glEnableVertexAttribArray(MMSFBGL_VSV_LOC)");

	glVertexAttribPointer(MMSFBGL_VSV_LOC, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), vertices);
	ERROR_CHECK_BOOL("glVertexAttribPointer(MMSFBGL_VSV_LOC,...)");

	// draw it
	glDrawArrays(GL_LINE_STRIP, 0, 5);
	ERROR_CHECK_BOOL("glDrawArrays(GL_LINE_STRIP,...)");

#endif

	return true;
}


bool MMSFBGL::drawRectangle2Di(int x1, int y1, int x2, int y2) {
	// change pixel based values to float values and draw it
	return drawRectangle2D(OGL_CALC_COORD_MIDDLE(x1, x2), OGL_CALC_COORD_MIDDLE(y1, y2),
							OGL_CALC_COORD_MIDDLE(x2, x1), OGL_CALC_COORD_MIDDLE(y2, y1));
}






bool MMSFBGL::fillTriangle(float x1, float y1, float z1,
							   float x2, float y2, float z2,
							   float x3, float y3, float z3) {

	INITCHECK;

	// configure generic vertex attribute array
	GLfloat vertices[] = {x1,y1,z1,x2,y2,z2,x3,y3,z3};
	glEnableVertexAttribArray(MMSFBGL_VSV_LOC);
	ERROR_CHECK_BOOL("glEnableVertexAttribArray(MMSFBGL_VSV_LOC)");

	glVertexAttribPointer(MMSFBGL_VSV_LOC, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), vertices);
	ERROR_CHECK_BOOL("glVertexAttribPointer(MMSFBGL_VSV_LOC,...)");

	// draw it
	glDrawArrays(GL_TRIANGLES, 0, 3);
	ERROR_CHECK_BOOL("glDrawArrays(GL_TRIANGLES,...)");

	return true;
}

bool MMSFBGL::fillTriangle2D(float x1, float y1, float x2, float y2, float x3, float y3) {

	INITCHECK;

	// configure generic vertex attribute array
	GLfloat vertices[] = {x1,y1,x2,y2,x3,y3};
	glEnableVertexAttribArray(MMSFBGL_VSV_LOC);
	ERROR_CHECK_BOOL("glEnableVertexAttribArray(MMSFBGL_VSV_LOC)");

	glVertexAttribPointer(MMSFBGL_VSV_LOC, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), vertices);
	ERROR_CHECK_BOOL("glVertexAttribPointer(MMSFBGL_VSV_LOC,...)");

	// draw it
	glDrawArrays(GL_TRIANGLES, 0, 3);
	ERROR_CHECK_BOOL("glDrawArrays(GL_TRIANGLES,...)");

	return true;
}



bool MMSFBGL::fillRectangle2D(float x1, float y1, float x2, float y2) {

	INITCHECK;

#ifdef __HAVE_GL2__

	glRectf(x1, y1, x2, y2);
	ERROR_CHECK_BOOL("glRectf()");

#endif

#ifdef __HAVE_GLES2__

	// configure generic vertex attribute array
	GLfloat vertices[] = {x2,y1,x1,y1,x1,y2,x2,y2};
	glEnableVertexAttribArray(MMSFBGL_VSV_LOC);
	ERROR_CHECK_BOOL("glEnableVertexAttribArray(MMSFBGL_VSV_LOC)");

	glVertexAttribPointer(MMSFBGL_VSV_LOC, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), vertices);
	ERROR_CHECK_BOOL("glVertexAttribPointer(MMSFBGL_VSV_LOC,...)");

	// draw it
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	ERROR_CHECK_BOOL("glDrawArrays(GL_TRIANGLE_FAN,...)");

#endif

	return true;
}


bool MMSFBGL::fillRectangle2Di(int x1, int y1, int x2, int y2) {
	// change pixel based values to float values and draw it
	return fillRectangle2D(OGL_CALC_COORD(x1, x2), OGL_CALC_COORD(y1, y2),
							OGL_CALC_COORD(x2, x1), OGL_CALC_COORD(y2, y1));
}



bool MMSFBGL::stretchBlit3D(GLuint src_tex, float sx1, float sy1, float sx2, float sy2,
								  float dx1, float dy1, float dz1,
								  float dx2, float dy2, float dz2,
								  float dx3, float dy3, float dz3,
								  float dx4, float dy4, float dz4) {

	INITCHECK;

	// setup blitting
	enableTexture2D(src_tex);

	// setup vertex array and indices
	GLfloat vVertices[] = { dx1,  dy1, dz1,  // Position 0
							sx1,  sy1,        // TexCoord 0
						    dx2, dy2, dz2,  // Position 1
							sx2,  sy1,        // TexCoord 1
							dx3, dy3, dz3,  // Position 2
							sx2,  sy2,        // TexCoord 2
							dx4,  dy4, dz4,  // Position 3
							sx1,  sy2         // TexCoord 3
						 };

	GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

#ifdef __HAVE_GL2__

	// load the vertex data
	glEnableClientState(GL_VERTEX_ARRAY);
	ERROR_CHECK_BOOL("glEnableClientState(GL_VERTEX_ARRAY)");
	glVertexPointer(3, GL_FLOAT, 5 * sizeof(GLfloat), vVertices);
	ERROR_CHECK_BOOL("glVertexPointer()");

	// load the texture coordinates
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	ERROR_CHECK_BOOL("glEnableClientState(GL_TEXTURE_COORD_ARRAY)");
	glTexCoordPointer(2, GL_FLOAT, 5 * sizeof(GLfloat), &vVertices[3]);
	ERROR_CHECK_BOOL("glTexCoordPointer()");

#endif

#ifdef __HAVE_GLES2__

	// load the vertex data
	glVertexAttribPointer(MMSFBGL_VSV_LOC, 3, GL_FLOAT,
						   GL_FALSE, 5 * sizeof(GLfloat), vVertices );
	ERROR_CHECK_BOOL("glVertexAttribPointer(MMSFBGL_VSV_LOC,...)");

	glEnableVertexAttribArray(MMSFBGL_VSV_LOC);
	ERROR_CHECK_BOOL("glEnableVertexAttribArray(MMSFBGL_VSV_LOC)");

	// load the texture coordinates
	glVertexAttribPointer(VSTexCoordLoc, 2, GL_FLOAT,
						   GL_FALSE, 5 * sizeof(GLfloat), &vVertices[3] );
	ERROR_CHECK_BOOL("glVertexAttribPointer(VSTexCoordLoc,...)");

	glEnableVertexAttribArray(VSTexCoordLoc);
	ERROR_CHECK_BOOL("glEnableVertexAttribArray(VSTexCoordLoc)");

	// bind the texture unit0
	glActiveTexture(GL_TEXTURE0);
	ERROR_CHECK_BOOL("glActiveTexture(GL_TEXTURE0)");

	glUniform1i(FSTextureLoc, 0);
	ERROR_CHECK_BOOL("glUniform1i(FSTextureLoc, 0)");

#endif

	// finally draw the triangles...
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
	ERROR_CHECK_BOOL("glDrawElements(GL_TRIANGLES,...)");

	return true;
}




bool MMSFBGL::stretchBlit(GLuint src_tex, float sx1, float sy1, float sx2, float sy2,
										   float dx1, float dy1, float dx2, float dy2) {

	INITCHECK;

	// setup blitting
	enableTexture2D(src_tex);

#ifdef __HAVE_GL2__

	glBegin(GL_QUADS);
		glTexCoord2f(sx1, sy1);
			glVertex2f(dx1, dy1);
		glTexCoord2f(sx2, sy1);
			glVertex2f(dx2, dy1);
		glTexCoord2f(sx2, sy2);
			glVertex2f(dx2, dy2);
		glTexCoord2f(sx1, sy2);
			glVertex2f(dx1, dy2);
	glEnd();
	ERROR_CHECK_BOOL("glBegin(GL_QUADS)");

#endif

#ifdef __HAVE_GLES2__

	GLfloat vVertices[] = { dx1,  dy1, 0.0f,  // Position 0
							sx1,  sy1,        // TexCoord 0
						    dx2, dy1, 0.0f,  // Position 1
							sx2,  sy1,        // TexCoord 1
							dx2, dy2, 0.0f,  // Position 2
							sx2,  sy2,        // TexCoord 2
							dx1,  dy2, 0.0f,  // Position 3
							sx1,  sy2         // TexCoord 3
						 };

	GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

	// Load the vertex position
	glVertexAttribPointer (MMSFBGL_VSV_LOC, 3, GL_FLOAT,
						   GL_FALSE, 5 * sizeof(GLfloat), vVertices );
	ERROR_CHECK_BOOL("glVertexAttribPointer (MMSFBGL_VSV_LOC,...)");

	glEnableVertexAttribArray(MMSFBGL_VSV_LOC);
	ERROR_CHECK_BOOL("glEnableVertexAttribArray(MMSFBGL_VSV_LOC)");

	// Load the texture coordinate
	glVertexAttribPointer(VSTexCoordLoc, 2, GL_FLOAT,
						   GL_FALSE, 5 * sizeof(GLfloat), &vVertices[3]);
	ERROR_CHECK_BOOL("glVertexAttribPointer(VSTexCoordLoc,...");

	glEnableVertexAttribArray(VSTexCoordLoc);
	ERROR_CHECK_BOOL("glEnableVertexAttribArray(VSTexCoordLoc)");

	// bind the texture unit0
	glActiveTexture(GL_TEXTURE0);
	ERROR_CHECK_BOOL("glActiveTexture(GL_TEXTURE0)");

	glUniform1i(FSTextureLoc, 0);
	ERROR_CHECK_BOOL("glUniform1i(FSTextureLoc,...)");

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
	ERROR_CHECK_BOOL("glDrawElements(GL_TRIANGLES,...)");

#endif

	return true;
}


bool MMSFBGL::stretchBliti(GLuint src_tex, int sx1, int sy1, int sx2, int sy2, int sw, int sh,
							int dx1, int dy1, int dx2, int dy2) {

	if (dx1 != dx2 && dy1 != dy2) {
		return stretchBlit(src_tex,
							OGL_CALC_2X_N(sx1, sx2, sw),
							OGL_CALC_2Y_N(sy1, sy2, sh),
							OGL_CALC_2X_N(sx2, sx1, sw),
							OGL_CALC_2Y_N(sy2, sy1, sh),
							OGL_CALC_COORD(dx1, dx2),
							OGL_CALC_COORD(dy1, dy2),
							OGL_CALC_COORD(dx2, dx1),
							OGL_CALC_COORD(dy2, dy1));
	} else if (dy1 != dy2) {
		return stretchBlit(src_tex,
							OGL_CALC_2X_N(sx1, sx2, sw),
							OGL_CALC_2Y_N(sy1, sy2, sh),
							OGL_CALC_2X_N(sx2, sx1, sw),
							OGL_CALC_2Y_N(sy2, sy1, sh),
							(float)(dx1),
							OGL_CALC_COORD(dy1, dy2),
							(float)(dx1) + 0.99,
							OGL_CALC_COORD(dy2, dy1));
	} else if (dx1 != dx2) {
		return stretchBlit(src_tex,
							OGL_CALC_2X_N(sx1, sx2, sw),
							OGL_CALC_2Y_N(sy1, sy2, sh),
							OGL_CALC_2X_N(sx2, sx1, sw),
							OGL_CALC_2Y_N(sy2, sy1, sh),
							OGL_CALC_COORD(dx1, dx2),
							(float)(dy1),
							OGL_CALC_COORD(dx2, dx1),
							(float)(dy1) + 0.99);

	} else {
		return stretchBlit(src_tex,
							OGL_CALC_2X_N(sx1, sx2, sw),
							OGL_CALC_2Y_N(sy1, sy2, sh),
							OGL_CALC_2X_N(sx2, sx1, sw),
							OGL_CALC_2Y_N(sy2, sy1, sh),
							(float)(dx1),
							(float)(dy1),
							(float)(dx1) + 0.99,
							(float)(dy1) + 0.99);
	}
}



bool MMSFBGL::stretchBlitBuffer(void *buffer, float sx1, float sy1, float sx2, float sy2, int sw, int sh,
												float dx1, float dy1, float dx2, float dy2) {

	INITCHECK;

	// alloc and load texture from buffer
	GLuint tex;
	genTexture(&tex);
	initTexture2D(tex, GL_RGBA, buffer, GL_RGBA, sw, sh);

	// blit texture to active FBO
	stretchBlit(tex, sx1, sy1, sx2, sy2, dx1, dy1, dx2, dy2);

	// delete texture
	deleteTexture(tex);

	return true;
}


bool MMSFBGL::stretchBlitBufferi(void *buffer, int sx1, int sy1, int sx2, int sy2, int sw, int sh,
												int dx1, int dy1, int dx2, int dy2) {

	INITCHECK;

	// alloc and load texture from buffer
	GLuint tex;
	genTexture(&tex);
	initTexture2D(tex, GL_RGBA, buffer, GL_RGBA, sw, sh);

	// blit texture to active FBO
	stretchBliti(tex, sx1, sy1, sx2, sy2, sw, sh, dx1, dy1, dx2, dy2);

	// delete texture
	deleteTexture(tex);

	return true;
}


bool MMSFBGL::blitBuffer2Texture(GLuint dst_tex, bool realloc, void *buffer, int sw, int sh) {

	INITCHECK;

	// load texture from buffer
	if (realloc) {
		// (re-)allocate texture memory
		return initTexture2D(dst_tex, GL_RGBA, buffer, GL_RGBA, sw, sh);
	}
	else {
		// overwrite existing texture memory
		return initSubTexture2D(dst_tex, buffer, GL_RGBA, sw, sh, 0, 0);
	}
}


bool MMSFBGL::drawElements(MMS3D_VERTEX_ARRAY *vertices, MMS3D_VERTEX_ARRAY *normals, MMS3D_VERTEX_ARRAY *texcoords,
						   MMS3D_INDEX_ARRAY *indices) {

	INITCHECK;

	if (!vertices || !indices) {
		// minimum parameters are vertices and indices
		return false;
	}

#ifdef __HAVE_GL2__

	// load the vertices
	if (vertices) {
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(vertices->eSize, GL_FLOAT, 0, vertices->buf);
	}
	else {
		glDisableClientState(GL_VERTEX_ARRAY);
	}

	// load the normals
	if (normals) {
		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT, 0, normals->buf);
	}
	else {
		glDisableClientState(GL_NORMAL_ARRAY);
	}

	// load the texture coordinates
	if (texcoords) {
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(texcoords->eSize, GL_FLOAT, 0, texcoords->buf);
	}
	else {
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}

#endif

#ifdef __HAVE_GLES2__

	// load the vertices
	if (vertices) {
		glVertexAttribPointer(MMSFBGL_VSV_LOC, vertices->eSize, GL_FLOAT,
							   GL_FALSE, 0, vertices->buf);
		ERROR_CHECK_BOOL("glVertexAttribPointer(MMSFBGL_VSV_LOC,...)");

		glEnableVertexAttribArray(MMSFBGL_VSV_LOC);
		ERROR_CHECK_BOOL("glEnableVertexAttribArray(MMSFBGL_VSV_LOC)");
	}
	else {
		glDisableVertexAttribArray(MMSFBGL_VSV_LOC);
		ERROR_CHECK_BOOL("glDisableVertexAttribArray(MMSFBGL_VSV_LOC)");
	}

	// load the texture coordinates
	if (texcoords) {
		glVertexAttribPointer(VSTexCoordLoc, texcoords->eSize, GL_FLOAT,
							   GL_FALSE, 0, texcoords->buf);
		ERROR_CHECK_BOOL("glVertexAttribPointer(VSTexCoordLoc,...)");

		glEnableVertexAttribArray(VSTexCoordLoc);
		ERROR_CHECK_BOOL("glEnableVertexAttribArray(VSTexCoordLoc)");

		// bind the texture unit0
		glActiveTexture(GL_TEXTURE0);
		ERROR_CHECK_BOOL("glActiveTexture(GL_TEXTURE0)");

		glUniform1i(FSTextureLoc, 0);
		ERROR_CHECK_BOOL("glUniform1i(FSTextureLoc, 0)");
	}
	else {
		glDisableVertexAttribArray(VSTexCoordLoc);
		ERROR_CHECK_BOOL("glDisableVertexAttribArray(VSTexCoordLoc)");
	}

#endif

	// draw elements
	// note: MMS3D_INDEX_ARRAY uses indices with type unsigned int (GL_UNSIGNED_INT)
	GLenum mode = GL_TRIANGLES;
	switch (indices->type) {
	case MMS3D_INDEX_ARRAY_TYPE_TRIANGLES_STRIP:
		mode = GL_TRIANGLE_STRIP;
		break;
	case MMS3D_INDEX_ARRAY_TYPE_TRIANGLES_FAN:
		mode = GL_TRIANGLE_FAN;
		break;
	default:
		break;
	}
	glDrawElements(mode, indices->eNum, GL_UNSIGNED_INT, indices->buf);

	// print errors
	switch (indices->type) {
	case MMS3D_INDEX_ARRAY_TYPE_TRIANGLES:
		ERROR_CHECK_BOOL("glDrawElements(GL_TRIANGLES,...)");
		break;
	case MMS3D_INDEX_ARRAY_TYPE_TRIANGLES_STRIP:
		ERROR_CHECK_BOOL("glDrawElements(GL_TRIANGLE_STRIP,...)");
		break;
	case MMS3D_INDEX_ARRAY_TYPE_TRIANGLES_FAN:
		ERROR_CHECK_BOOL("glDrawElements(GL_TRIANGLE_FAN,...)");
		break;
	}

	return true;
}



#endif


