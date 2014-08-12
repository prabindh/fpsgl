/************************************************************************

Module to display FPS text using OpenGL ES2

Usage:

fps_init()
fps_draw_offscreen()
fps_display_onscreen()

The texture generated in the offscreen stage can be used as a 
regular texture in other use-cases, or fps_display_onscreen() can be 
used to display onscreen. The module uses an internal bitmap to 
translate numbers to OpenGL texture.

prabindh@yahoo.com, 2014

************************************************************************/


#ifndef __FPS_RENDERER_H
#define __FPS_RENDERER_H

#define MAX_TEXT_CHARS 2
typedef struct _gl_state
{
	int program;
	GLuint fboID;
	GLuint fbotextureID;
	GLuint fontTextureID;
	GLuint vboID[3];
	GLuint attribIndices[2];
	GLint uniformTextureScalerLoc;
	GLint yInvertLoc;
	int digitCount;
	GLint uniformOffsetLoc;
	int screenWidthPix;
	int screenHeightPix;
}gl_state;

//debug
#ifdef _DEBUG
#define D_PRINTF(x)  printf(x)
#define GL_CHECK(x) \
{ \
x; \
int err = glGetError(); \
printf("GL Error = %x for %s\n", err, (char*)(#x)); \
}
#else //DEBUG
#define D_PRINTF(x)
#define GL_CHECK(x)
#endif //DEBUG


typedef enum __fps_loc
{
	FPS_LOC_TOP_LEFT = 0,
	FPS_LOC_TOP_RIGHT,
	FPS_LOC_BOTTOM_LEFT,
	FPS_LOC_BOTTOM_RIGHT
}fps_loc;

 
int fps_init(gl_state *fps_gl, GLuint textureID, int max_chars, int width, int height);
int fps_draw_offscreen(int fps, gl_state *fps_gl);
void fps_display_onscreen(gl_state *fps_gl, fps_loc loc);

#endif //#ifndef __FPS_RENDERER_H

