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

#ifndef MMSFBGL_H_
#define MMSFBGL_H_

#include "mmstools/mmstypes.h"
#include <stack>

#ifdef __HAVE_OPENGL__

#include <string.h>

#ifdef __HAVE_GL2__
#include <GL/glew.h>
#include <GL/gl.h>
#endif

#ifdef __HAVE_GLX__
#include <GL/glx.h>
#include <GL/glu.h>
#endif

#ifdef __HAVE_GLES2__
#include <GLES2/gl2.h>
#endif

#ifdef __HAVE_EGL__
#include <EGL/egl.h>
#endif

//! Wrapper class for all supported Open GL versions.
/*!
\author Jens Schneider
*/
class MMSFBGL {
    private:
    	// bind the "VSVertex" attribute to shader index 0
    	#define MMSFBGL_VSV_LOC	0

    	typedef enum {
    		MMSFBGL_SHADER_TYPE_FRAGMENT_SHADER = 0,
    		MMSFBGL_SHADER_TYPE_VERTEX_SHADER
    	} MMSFBGL_SHADER_TYPE;

#ifdef __HAVE_XLIB__
    	Display *x_display;
    	Window 	x_window;
#endif

#ifdef __HAVE_GLX__
    	XVisualInfo *xvi;
    	GLXContext	glx_context;
#endif

#ifdef __HAVE_EGL__
    	EGLDisplay eglDisplay;
    	EGLConfig  eglConfig[10];
    	EGLSurface eglSurface;
    	EGLContext eglContext;
#endif

    	bool initialized;

    	int screen_width;
    	int screen_height;

    	GLuint bound_fbo;

    	//! program handle to the fragment and vertex shader used for drawing primitives
    	GLuint po_draw;

    	//! program handle to the fragment and vertex shader used for blitting
    	GLuint po_blit;

    	//! program handle to the fragment and vertex shader used for blitting with source modulation beforehand
    	GLuint po_modulateblit;

    	//! program handle to the fragment and vertex shader used for blitting from alpha
    	GLuint po_blit_fromalpha;

    	//! program handle to the fragment and vertex shader used for blitting from alpha source and modulation beforehand
    	GLuint po_modulateblit_fromalpha;

    	//! currently active program
    	GLuint po_current;

    	//! matrix location in the vertex shader
    	GLint VSMatrixLoc;

    	//! matrix location in the vertex shader initialized
    	bool VSMatrixLoc_initialized;

    	//! color location in the fragment shader
    	GLint FSColorLoc;

    	//! color location in the fragment shader initialized
    	bool FSColorLoc_initialized;

    	//! texture location in the fragment shader
    	GLint FSTextureLoc;

    	//! texture location in the fragment shader initialized
    	bool FSTextureLoc_initialized;

    	//! texture coordinates location in the vertex shader
    	GLint VSTexCoordLoc;

    	//! texture coordinates location in the vertex shader initialized
    	bool VSTexCoordLoc_initialized;

    	//! current matrix
    	MMS3DMatrix	current_matrix;

    	//! current color
    	unsigned char	current_color_r;
    	unsigned char	current_color_g;
    	unsigned char	current_color_b;
    	unsigned char	current_color_a;

    	class MMSFBGLStackMatrix {
    	public:
    		MMS3DMatrix matrix;
    		MMSFBGLStackMatrix(MMS3DMatrix matrix) {
    			memcpy(this->matrix, matrix, sizeof(MMS3DMatrix));
    		}
    		void getMatrix(MMS3DMatrix matrix) {
    			memcpy(matrix, this->matrix, sizeof(MMS3DMatrix));
    		}
    	};

    	//! matrix stack
        std::stack<MMSFBGLStackMatrix> matrix_stack;


    	bool getError(const char* where, int line = __LINE__);

    	bool buildShader(MMSFBGL_SHADER_TYPE shader_type, const char *shader_code, GLuint *shader);
    	bool linkProgram(GLuint fragment_shader, GLuint vertex_shader, GLuint *program);
    	bool buildShaderProgram(const char *fragment_shader_code, const char *vertex_shader_code, GLuint *program);
    	bool buildShaderProgram4Drawing(GLuint *program);
    	bool buildShaderProgram4Blitting(GLuint *program);
    	bool buildShaderProgram4ModulateBlitting(GLuint *program);
    	bool buildShaderProgram4BlittingFromAlpha(GLuint *program);
    	bool buildShaderProgram4ModulateBlittingFromAlpha(GLuint *program);

    	void deleteShaders();
        bool initShaders();

    public:
        MMSFBGL();
        ~MMSFBGL();

#ifdef __HAVE_XLIB__
        bool init(Display *x_display, int x_screen, Window x_window, int w, int h);
#else
        bool init();
#endif
    	bool terminate();
    	bool getResolution(int *w, int *h);
    	bool swap();

        bool genTexture(GLuint *tex);
        bool deleteTexture(GLuint tex);
        bool bindTexture2D(GLuint tex);
        bool initTexture2D(GLuint tex, GLenum texture_format, void *buffer, GLenum buffer_format, int sw, int sh);
        bool initSubTexture2D(GLuint tex, void *buffer, GLenum buffer_format, int sw, int sh, int dx, int dy);
        bool enableTexture2D(GLuint tex);
        void disableTexture2D();

        bool genFrameBuffer(GLuint *fbo);
        bool deleteFrameBuffer(GLuint fbo);
        bool genRenderBuffer(GLuint *rbo);
        bool deleteRenderBuffer(GLuint rbo);
        bool attachTexture2FrameBuffer(GLuint fbo, GLuint tex);
        bool attachRenderBuffer2FrameBuffer(GLuint fbo, GLuint rbo, int width, int height);

        bool allocTexture(GLuint tex, int width, int height);
        bool allocFBO(GLuint fbo, GLuint tex, int width, int height);
        bool allocFBOandRBO(GLuint fbo, GLuint tex, GLuint rbo, int width, int height);
    	bool freeFBO(GLuint fbo, GLuint tex, GLuint rbo = 0);

    	bool bindFrameBuffer(GLuint ogl);
    	bool setScissor(GLint x, GLint y, GLsizei width, GLsizei height);
    	bool disableScissor();

        void enableBlend(GLenum srcRGB = GL_SRC_ALPHA, GLenum dstRGB = GL_ONE_MINUS_SRC_ALPHA,
						  GLenum srcAlpha = GL_ONE, GLenum dstAlpha = GL_ONE_MINUS_SRC_ALPHA);
        void disableBlend();
        void enableDepthTest(bool readonly = false);
        void disableDepthTest();
		void setDrawingMode();
		void setTexEnvReplace(GLenum format);
		void setTexEnvModulate(GLenum format);
		void disableArrays();

        bool useShaderProgram4Drawing();
        bool useShaderProgram4Blitting();
        bool useShaderProgram4ModulateBlitting();
        bool useShaderProgram4BlittingFromAlpha();
        bool useShaderProgram4ModulateBlittingFromAlpha();

        bool setCurrentMatrix(MMS3DMatrix matrix);
        bool getCurrentMatrix(MMS3DMatrix matrix);
        bool scaleCurrentMatrix(GLfloat sx, GLfloat sy, GLfloat sz);
        bool translateCurrentMatrix(GLfloat tx, GLfloat ty, GLfloat tz);
        bool rotateCurrentMatrix(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);

        bool getParallelProjectionMatrix(MMS3DMatrix result, float left, float right, float bottom, float top, float nearZ, float farZ);
        bool getCentralProjectionMatrix(MMS3DMatrix result, float left, float right, float bottom, float top, float nearZ, float farZ);
        bool getPerspectiveMatrix(MMS3DMatrix result, float fovy, float aspect, float nearZ, float farZ);

        bool setParallelProjection(float left, float right, float bottom, float top, float nearZ, float farZ);
        bool setCentralProjection(float left, float right, float bottom, float top, float nearZ, float farZ);
        bool setPerspective(float fovy, float aspect, float nearZ, float farZ);

        bool pushCurrentMatrix();
        bool popCurrentMatrix();

        bool clear(unsigned char r = 0x00, unsigned char g = 0x00, unsigned char b = 0x00, unsigned char a = 0x00);
        bool setColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a);

        bool drawRectangle2D(float x1, float y1, float x2, float y2);
        bool drawRectangle2Di(int x1, int y1, int x2, int y2);


        bool fillTriangle(float x1, float y1, float z1,
							 float x2, float y2, float z2,
							 float x3, float y3, float z3);
        bool fillTriangle2D(float x1, float y1, float x2, float y2, float x3, float y3);

        bool fillRectangle2D(float x1, float y1, float x2, float y2);
        bool fillRectangle2Di(int x1, int y1, int x2, int y2);


        bool stretchBlit3D(GLuint src_tex, float sx1, float sy1, float sx2, float sy2,
										  float dx1, float dy1, float dz1,
										  float dx2, float dy2, float dz2,
										  float dx3, float dy3, float dz3,
										  float dx4, float dy4, float dz4);

        bool stretchBlit(GLuint src_tex, float sx1, float sy1, float sx2, float sy2,
										  float dx1, float dy1, float dx2, float dy2);
        bool stretchBliti(GLuint src_tex, int sx1, int sy1, int sx2, int sy2, int sw, int sh,
										   int dx1, int dy1, int dx2, int dy2);



        bool stretchBlitBuffer(void *buffer, float sx1, float sy1, float sx2, float sy2, int sw, int sh,
											  float dx1, float dy1, float dx2, float dy2);
        bool stretchBlitBufferi(void *buffer, int sx1, int sy1, int sx2, int sy2, int sw, int sh,
										   int dx1, int dy1, int dx2, int dy2);

        bool blitBuffer2Texture(GLuint dst_tex, bool realloc, void *buffer, int sw, int sh);

        bool drawElements(MMS3D_VERTEX_ARRAY *vertices, MMS3D_VERTEX_ARRAY *normals, MMS3D_VERTEX_ARRAY *texcoords,
						  MMS3D_INDEX_ARRAY *indices);
};

#endif

#endif /* MMSFBGL_H_ */
