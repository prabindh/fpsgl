/************************************************************************

Test Module to display FPS text using OpenGL ES2

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


static gl_state myfps_gl;
static GLuint fps_texture;


//EGL initialisation should have been done before this
void test_fps()
{
	//TEST FPS
	glGenTextures(1, &fps_texture);
	fps_init(&myfps_gl, fps_texture, MAX_TEXT_CHARS);
	GL_CHECK(fps_init);
	
	//Draw fps and test
	static int fps = 0;
	if(fps > 99) fps = 0;
	fps_draw_offscreen(fps++, &myfps_gl);
	//draw fps onscreen now
	fps_display_onscreen(&myfps_gl);
}
