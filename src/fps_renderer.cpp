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
#include "GLES2/gl2.h"

#include "stdlib.h"
#include "stdio.h"
#include "memory.h"


#include "surroundview_globals.h"

//TODO - configurable input - depends on bitmap, num of chars in bitmap
#define FONT_BITMAP_WIDTH 1024
#define FONT_BITMAP_HEIGHT 32
#define FONT_BITMAP_NUM_CHARS 36
#define FONT_PER_CHARACTER_WIDTH (float)((float)FONT_BITMAP_WIDTH/(float)FONT_BITMAP_NUM_CHARS)
#define FONT_PER_CHARACTER_HEIGHT 32

static const float vertices[] = {
	-1.0, 1.0, 0.0,
	-1.0, -1.0, 0.0,
	1.0, -1.0, 0.0,
	1.0, 1.0, 0.0
};
static const unsigned short indices[] = {
	0,1,2,
	0,2,3
};

static const float texCoords[] = {
	0,1.0,
	0,0,
	1.0,0,
	1.0,1.0
};

static const char* vshader = "\
	precision mediump float; \
    attribute vec4 aVertexPosition;\
	attribute vec2 aTextureCoord; \
	varying vec2 vTextureCoord; \
	uniform float yInvert; \
    void main(void) {\
		gl_Position = vec4(aVertexPosition.x, yInvert*aVertexPosition.y, 0.0, 1.0);\
		vTextureCoord = aTextureCoord; \
    }";

static const char* fshader = "\
	precision mediump float; \
	varying vec2 vTextureCoord; \
	uniform sampler2D uSampler; \
	uniform float textureScaler; \
	uniform float offset; \
    void main(void) {\
	gl_FragColor = texture2D(uSampler, vec2((0.01+offset + vTextureCoord.x/textureScaler), vTextureCoord.y));\
	}";
	//texture2D(uSampler, vTextureCoord);\
	

int fps_shader_init(gl_state *fps_gl)
{
	/* Create program and link */
	GLuint uiFragShader, uiVertShader;		// Used to hold the fragment and vertex shader handles

	// Create the fragment shader object
	uiFragShader = glCreateShader(GL_FRAGMENT_SHADER);

	// Load the source code into it
	glShaderSource(uiFragShader, 1, (const char**)&fshader, NULL);
	// Compile the source code
	glCompileShader(uiFragShader);

	// Check if compilation succeeded
	GLint bShaderCompiled;
    glGetShaderiv(uiFragShader, GL_COMPILE_STATUS, &bShaderCompiled);

	if (!bShaderCompiled)
	{
		printf("Error in frag shader!\n");
	}
	// Loads the vertex shader in the same way
	uiVertShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(uiVertShader, 1, (const char**)&vshader, NULL);

	glCompileShader(uiVertShader);
    glGetShaderiv(uiVertShader, GL_COMPILE_STATUS, &bShaderCompiled);

	if (!bShaderCompiled)
	{
		printf("Error: compiling vert shader\n");
	}
	// Create the shader program
	fps_gl->program = glCreateProgram();

	// Attach the fragment and vertex shaders to it
    glAttachShader(fps_gl->program, uiFragShader);
    glAttachShader(fps_gl->program, uiVertShader);

	// Link the program
    glLinkProgram(fps_gl->program);

	// Check if linking succeeded in the same way we checked for compilation success
    GLint bLinked;
    glGetProgramiv(fps_gl->program, GL_LINK_STATUS, &bLinked);

	//set the program
	glUseProgram(fps_gl->program);

	if (!bLinked)
	{
		printf("Error: linking prog\n");
	}
	return 0;
}

int __pow2(int num)
{
	int pow2=1;
	while(pow2 < num)
	{
		pow2 = 2*pow2;
	}
	return pow2;
}

void offscreen_state_restore(gl_state *fps_gl, int fps)
{
	//set the program we need
	glUseProgram(fps_gl->program);
	if(fps_gl->yInvertLoc != -1)
		glUniform1f(fps_gl->yInvertLoc, -1);

	//font texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fps_gl->fontTextureID);
	//framebuffer target
	glBindFramebuffer(GL_FRAMEBUFFER, fps_gl->fboID);

	//restore the vbo
	glBindBuffer(GL_ARRAY_BUFFER, fps_gl->vboID[0]);
	glEnableVertexAttribArray(fps_gl->attribIndices[0]);
	glVertexAttribPointer(fps_gl->attribIndices[0], 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, fps_gl->vboID[1]);
	glEnableVertexAttribArray(fps_gl->attribIndices[1]);
	glVertexAttribPointer(fps_gl->attribIndices[1], 2, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fps_gl->vboID[2]);

	glViewport(0,0,__pow2(fps_gl->digitCount*FONT_PER_CHARACTER_WIDTH), __pow2(FONT_PER_CHARACTER_HEIGHT));
	if(fps_gl->uniformTextureScalerLoc != -1)
		glUniform1f(fps_gl->uniformTextureScalerLoc, FONT_BITMAP_NUM_CHARS);
}

int __count_digits(int digit)
{
	int count = 1;
	while(digit=digit/10) {		count ++;}
	return count;
}

int __font_to_pixels(int digit)
{
	return (digit * FONT_PER_CHARACTER_WIDTH);
}

int draw_gl(int fps, gl_state *fps_gl)
{
	int currdigit;
	float curroffset;
	int digits = __count_digits(fps);
	//calculate the number of digits

	//clear before!!
	glClearColor(0.0, 1.0, 0.0, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	do{
		//Set the offset in viewport correctly
		glViewport(0+(digits-1)*FONT_PER_CHARACTER_WIDTH,0, (FONT_PER_CHARACTER_WIDTH), __pow2(FONT_PER_CHARACTER_HEIGHT));
		//break the number into individual digits, start from least digit
		currdigit = fps-10*(fps/10);
		curroffset = __font_to_pixels(currdigit);
		// set the texture coordinate offset
		glUniform1f(fps_gl->uniformOffsetLoc, (float)curroffset/(float)FONT_BITMAP_WIDTH);
		/* draw the triangles to fbo */
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
		GL_CHECK(glDrawElements);
		//now move the next drawloc
		digits --;
	}while(fps = fps/10);
	
	//test with clear - this works!!
	//glClearColor(0.0, 1.0, 0.0, 0.5);
	//glClear(GL_COLOR_BUFFER_BIT);

	glFlush();
	return 0;
}

int fps_draw_offscreen(int fps, gl_state *fps_gl)
{
	/* set FBO render program, and set render target to FBO */
	offscreen_state_restore(fps_gl, fps);
	/* convert fps string to font triangles */
	draw_gl(fps, fps_gl);
	return 0;
}

int fps_font_init(gl_state *fps_gl, GLuint textureID, int max_chars)
{
	fps_gl->digitCount = max_chars;
	/* texture attachment for the FBO */
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, __pow2(fps_gl->digitCount*FONT_PER_CHARACTER_WIDTH), __pow2(FONT_PER_CHARACTER_HEIGHT), 
					0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	fps_gl->fbotextureID = textureID;

	/* Create FBO target */
	glGenFramebuffers(1, &fps_gl->fboID);
	glBindFramebuffer(GL_FRAMEBUFFER, fps_gl->fboID);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fps_gl->fbotextureID, 0);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) 
	{
		printf("Error in FB completeness check\n");
		while(1);
	};

	// Create and Bind font texture
	glGenTextures(1, &fps_gl->fontTextureID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fps_gl->fontTextureID);
	GL_CHECK(glBindTexture);
	load_texture_from_raw_file(FONT_BITMAP_WIDTH, FONT_BITMAP_HEIGHT, GL_RGB, "anonymous_pro_new2.bmp");
	
	//Setup VBO
	glGenBuffers(3, fps_gl->vboID);
	glBindBuffer(GL_ARRAY_BUFFER, fps_gl->vboID[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, fps_gl->vboID[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fps_gl->vboID[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	GL_CHECK(glBufferData);

	fps_gl->attribIndices[0] = glGetAttribLocation(fps_gl->program, "aVertexPosition");
	fps_gl->attribIndices[1] = glGetAttribLocation(fps_gl->program, "aTextureCoord");
	glEnableVertexAttribArray(fps_gl->attribIndices[0]);
	glVertexAttribPointer(fps_gl->attribIndices[0], 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(fps_gl->attribIndices[1]);
	glVertexAttribPointer(fps_gl->attribIndices[1], 2, GL_FLOAT, GL_FALSE, 0, 0);

	GL_CHECK(glVertexAttribPointer);

	//since a single shader is used for both offscreen and onscreen, scale from 1.0 (fullscreen) to single character
	fps_gl->uniformTextureScalerLoc= glGetUniformLocation(fps_gl->program, "textureScaler");
	GL_CHECK(glGetUniformLocation);
	//offset of text string in offscreen run
	fps_gl->uniformOffsetLoc= glGetUniformLocation(fps_gl->program, "offset");
	GL_CHECK(glGetUniformLocation);

	//whether to invert Y or not
	fps_gl->yInvertLoc= glGetUniformLocation(fps_gl->program, "yInvert");
	GL_CHECK(glGetUniformLocation);

	return 0;
}

int fps_init(gl_state *fps_gl, GLuint textureID, int fps, int width, int height)
{
	//Initialise the shaders
	fps_shader_init(fps_gl);
	GL_CHECK(fps_shader_init);
	/* Initialise Font engine */
	fps_font_init(fps_gl, textureID, fps);
	GL_CHECK(fps_font_init);

	fps_gl->screenWidthPix = width;
	fps_gl->screenHeightPix = height;
	return 0;
}

void onscreen_state_restore(gl_state *fps_gl, fps_loc loc)
{
	//set the program we need
	glUseProgram(fps_gl->program);
	if(loc == FPS_LOC_BOTTOM_LEFT)
		glViewport ( 0, 0, __pow2(fps_gl->digitCount*FONT_PER_CHARACTER_WIDTH), __pow2(FONT_PER_CHARACTER_HEIGHT));
	else if(loc == FPS_LOC_TOP_RIGHT)
		glViewport ( fps_gl->screenWidthPix-__pow2(fps_gl->digitCount*FONT_PER_CHARACTER_WIDTH), 
			fps_gl->screenHeightPix-__pow2(FONT_PER_CHARACTER_HEIGHT), 
				__pow2(fps_gl->digitCount*FONT_PER_CHARACTER_WIDTH), __pow2(FONT_PER_CHARACTER_HEIGHT));

	//font texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fps_gl->fbotextureID);
	//do not need invert here
	if(fps_gl->yInvertLoc != -1)
		glUniform1f(fps_gl->yInvertLoc, -1);
	//framebuffer target
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("Error in FB completeness check\n");
		while(1);
	};

	//restore the vbo
	glBindBuffer(GL_ARRAY_BUFFER, fps_gl->vboID[0]);
	glEnableVertexAttribArray(fps_gl->attribIndices[0]);
	glVertexAttribPointer(fps_gl->attribIndices[0], 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, fps_gl->vboID[1]);
	glEnableVertexAttribArray(fps_gl->attribIndices[1]);
	glVertexAttribPointer(fps_gl->attribIndices[1], 2, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fps_gl->vboID[2]);
	if(fps_gl->uniformTextureScalerLoc != -1)
		glUniform1f(fps_gl->uniformTextureScalerLoc, 1.0);

	glUniform1f(fps_gl->uniformOffsetLoc, 0.0);
}

void fps_display_onscreen(gl_state *fps_gl, fps_loc loc)
{
	onscreen_state_restore(fps_gl, loc);

	//draw and flush
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
	glFlush();
}

int fps_deinit(void *fps_gl)
{
	/* Delete program */
	/* UnInitialise Font engine */
	/* Delete FBO target */
	return 0;
}
