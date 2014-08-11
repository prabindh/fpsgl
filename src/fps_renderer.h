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


int fps_init(gl_state *fps_gl, GLuint textureID, int max_chars);
void fps_display_onscreen(gl_state *fps_gl);
int fps_draw_offscreen(int fps, gl_state *fps_gl);

#endif //#ifndef __FPS_RENDERER_H
