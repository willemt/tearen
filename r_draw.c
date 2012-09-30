/*
 
Copyright (c) 2011, Willem-Hendrik Thiart
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * The names of its contributors may not be used to endorse or promote
      products derived from this software without specific prior written
      permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL WILLEM-HENDRIK THIART BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/


/**
 * TODO
 * use tea adts and caching for pixel shaders
 *
 * use listmodes to find the highest resolution to use, then use it
    SDL_Rect **SDL_ListModes(SDL_PixelFormat *format, Uint32 flags);
    http://www.libsdl.org/cgi/docwiki.cgi/SDL_5fListModes
 */

#include <stdlib.h>

#if 0
#ifdef __APPLE__

#else
#define HAVE_GLEW 1
#include <GL/glew.h>
#endif

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#endif



#include <assert.h>
#include "tea_vec.h"
#include <stdbool.h>
#include "r_draw.h"
#include <SDL/SDL.h>
#include "SDL/SDL_opengl.h"
//#include <OpenGL/gl.h>
//#include <OpenGL/glext.h>
#include <gl.h>
#include <glext.h>

#define R_SDL_SCREENFLAGS SDL_OPENGL|SDL_GL_DOUBLEBUFFER

#define ERR_S "error"

ren_renderer_t *rSys = NULL;

typedef struct {
    bool testgrid;
    SDL_Surface *screen;
    int w;
    int h;

} __ren_renderer_in_t;

#define in(x) ((__ren_renderer_in_t*)x->in)

/**
 * set the renderer's camera */
void ren_draw_set_camera(vec2_t camera_orig, vec2_t camera_targ)
{
    vec2Copy(camera_orig, rSys->cameraOrg);
    vec2Copy(camera_targ, rSys->cameraTarg);
}

/** for multi-render scenes */
void ren_draw_set_context(ren_renderer_t * context)
{
    rSys = context;
    if (NULL == rSys->in)
    {
	rSys->in = calloc(1, sizeof(__ren_renderer_in_t));
    }
}

/*-------------------------------------------------------------- INIT METHODS */

/*  
 *  @return 0 on error; 1 otherwise*/
static int __initOpenGL()
{
//      GL_ARB_fragment_shader 
//      GL_ARB_vertex_shader

#ifdef HAVE_GLEW
    glewInit();

    if (!GLEW_VERSION_2_0)
    {
	fprintf(stderr, "OpenGL 2.0 not available\n");
	return 1;
    }

    /* check if we have shader support */
    if (GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader)
    {

    }
    else
    {

    }

    /*  check if we can do point sprites */
    if (glewGetExtension("GL_ARB_point_sprite"))
    {
	float maxSize = 0.0f;

	/*  find out what the max point sprite size is */
	glGetFloatv(GL_POINT_SIZE_MAX_ARB, &maxSize);

	if (maxSize < 32)
	{

	}
	else
	{
//              glPointSize(maxSize);
//              rSysi->have_point_sprite = TRUE;
	}
    }

#else


#endif

#if 0
#if 1
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
#else
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#endif
#endif

//    glEnable(GL_TEXTURE_2D);
//    glDisable(GL_DEPTH_TEST);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
//    glClearDepth(1.0f);
//    glShadeModel(GL_SMOOTH);
//    glEnable(GL_DEPTH_TEST);
//    glDepthFunc(GL_LEQUAL);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
//      glDisable(GL_ARB_texture_non_power_of_two);

    printf("Using: %s\n", glGetString(GL_VERSION));

    return 1;
}

static int __initSDL_end()
{
    /* If we fail, return error. */
    if (in(rSys)->screen == NULL)
    {
	//fprintf(stderr, ERR_S"Unable to set up video: %s\n", SDL_GetError());
	printf(ERR_S "Unable to set up video: %s\n", SDL_GetError());
	exit(1);
    }

    /*  check if the main surface is a hardware surface */
    if (1 == (in(rSys)->screen->flags & SDL_HWSURFACE))
    {
	printf(ERR_S "Can't get hardware surface\n");
	exit(1);
    }

    /*  check if we have a double buffer */
    if (!(in(rSys)->screen->flags & SDL_DOUBLEBUF))
    {

    }
    else
    {

    }

    return 0;
}

static void __vid_info()
{
/*
	vinfo = SDL_GetVideoInfo() ;

	tea_msg(mREN,0,"hw_available:  %d\n", vinfo->hw_available) ;
	tea_msg(mREN,0,"wm_available:  %d\n", vinfo->wm_available) ;
	tea_msg(mREN,0,"UnusedBits1:   %d\n", vinfo->UnusedBits1) ;
	tea_msg(mREN,0,"UnusedBits2:   %d\n", vinfo->UnusedBits2) ;
	tea_msg(mREN,0,"blit_hw:       %d\n", vinfo->blit_hw) ;
	tea_msg(mREN,0,"blit_hw_CC:    %d\n", vinfo->blit_hw_CC) ;
	tea_msg(mREN,0,"blit_hw_A:     %d\n", vinfo->blit_hw_A) ;
	tea_msg(mREN,0,"blit_sw:       %d\n", vinfo->blit_sw) ;
	tea_msg(mREN,0,"blit_sw_CC:    %d\n", vinfo->blit_sw_CC) ;
	tea_msg(mREN,0,"blit_sw_A:     %d\n", vinfo->blit_sw_A) ;
	tea_msg(mREN,0,"blit_fill:     %d\n", vinfo->blit_fill) ;
	tea_msg(mREN,0,"UnusedBits3:   %d\n", vinfo->UnusedBits3) ;
	tea_msg(mREN,0,"video_mem:     %dK\n", vinfo->video_mem) ;
	tea_msg(mREN,0,"BitsPerPixel:  %d\n\n", vinfo->vfmt->BitsPerPixel) ;
*/
}

static int __initSDL_start(const bool opengl,
			   const int w, const int h, const char *name)
{
    int video_flags = 0;

    video_flags = SDL_HWSURFACE | SDL_OPENGL;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
	fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
	exit(2);
    }

#if 0
    printf("OpenGL version: %s\n", glGetString(GL_VERSION));
    printf("OpenGL vendor: %s\n", glGetString(GL_VENDOR));
    printf("OpenGL renderer: %s\n", glGetString(GL_RENDERER));
    printf("setting size: %d %d\n", w, h);
#endif

//      SDL_WM_SetIcon( IMG_Load("tea.png"), NULL );
//    SDL_WM_SetCaption(name, name);
    __vid_info();

#if 0
    if (opengl)
    {
	// TODO: add fullscreen
	/* Enable double buffering */
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	video_flags |= R_SDL_SCREENFLAGS;
	//video_flags |= SDL_FULLSCREEN;
	//video_flags |= SDL_RESIZABLE
    }
#endif

    in(rSys)->screen =
	SDL_SetVideoMode(in(rSys)->w, in(rSys)->h, 32, video_flags);

    return 0;
}

/*----------------------------------------------------------------SET METHODS */

/** Set Renderer options/flags */
void ren_draw_enable(const int flag)
{
    switch (flag)
    {
    case R_FULLSCREEN:
	{			/* set to full screen */
	    int video_flags = R_SDL_SCREENFLAGS | SDL_FULLSCREEN;

//            in(rSys)->screen = SDL_SetVideoMode(w, in(rSys)->h, 32, video_flags);

#if defined(__TEA_WIN32__)
	    __initOpenGL();
#endif
	}
	break;
    case R_TESTGRID:
	in(rSys)->testgrid = true;
	break;
    }
}

void ren_draw_disable(const int flag)
{
    switch (flag)
    {
    case R_FULLSCREEN:
	{			/* set to full screen */
	    int video_flags;

	    video_flags = R_SDL_SCREENFLAGS;

//            in(rSys)->screen = SDL_SetVideoMode(in(rSys)->w, in(rSys)->h, 32, video_flags);
#if defined(__TEA_WIN32__)
	    __initOpenGL();
#endif
	}
	break;
    case R_TESTGRID:
	in(rSys)->testgrid = false;
	break;
    }
}

/*------------------------------------------------------- ACTUAL DRAW METHODS */

/** zero the marker */
void ren_frame_begin(void
    )
{
    glClear(GL_COLOR_BUFFER_BIT);
}

void ren_frame_end(void
    )
{
    SDL_GL_SwapBuffers();
//    glFlush();
//    glutSwapBuffers();
}

void ren_set_screensize(const int w, const int h)
{
    in(rSys)->w = w;
    in(rSys)->h = h;
}

/**
 * init the renderer sub system */
void ren_draw_init(char *name)
{
    bool opengl = true;

    rSys = calloc(1, sizeof(ren_renderer_t));
    rSys->in = calloc(1, sizeof(__ren_renderer_in_t));
    ren_set_screensize(960, 640);

    __initSDL_start(opengl, in(rSys)->w, in(rSys)->h, name);

    if (opengl)
    {
	__initOpenGL();
    }

    __initSDL_end();

    ren_medias_init();
    ren_objs_init();
}

/*--------------------------------------------------------------79-characters-*/
/* vim: set expandtab! : */
