
/* 
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*  Tactical Elemental Arenas                                                
*  C Implementation: render draw (main)                                     
*                                                                           
*                                                                           
*---------------------------------------------------------------------------
*	Copyright (C) 2005 by Willem-Hendrik Thiart                         
*	beandaddy@gmail.com                                                 
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*/

/**
 * TODO
 * use tea adts and caching for pixel shaders
 * tea arraylist_fixed_t* instead of rMedia[]
 *
 * use listmodes to find the highest resolution to use, then use it
    SDL_Rect **SDL_ListModes(SDL_PixelFormat *format, Uint32 flags);
    http://www.libsdl.org/cgi/docwiki.cgi/SDL_5fListModes
 */

#include <stdlib.h>
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
#include <assert.h>
//#include "r_local.h"
#include "tea_vec.h"
#include <stdbool.h>
#include "r_draw.h"
#include <SDL/SDL.h>
//#include "r_opengl.h"
//#include "r_tiles.h"
//#include "tea_datatype.h"

#define ACCUM_BUFFER 0
#define R_SDL_SCREENFLAGS SDL_OPENGL|SDL_GL_DOUBLEBUFFER

#define ERR_S "error"

ren_renderer_t *rSys = NULL;

typedef struct
{
    bool testgrid;
    SDL_Surface *screen;
    int w;
    int h;

} __ren_renderer_in_t;

#define in(x) ((__ren_renderer_in_t*)x->in)

/**
 * set the renderer's camera */
void ren_draw_set_camera(
    vec2_t camera_orig,
    vec2_t camera_targ
)
{
    vec2Copy(camera_orig, rSys->cameraOrg);
    vec2Copy(camera_targ, rSys->cameraTarg);
}

/** for multi-render scenes */

void ren_draw_set_context(
    ren_renderer_t * context
)
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
static int __initOpenGL(
)
{
//      GL_ARB_fragment_shader 
//      GL_ARB_vertex_shader

#ifdef HAVE_GLEW
    glewInit();

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

//      __initShaders();

#if 0
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glClearColor(0.4f, 0.4f, 0.4f, 0.0f);
#endif
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

#if ACCUM_BUFFER
    glClear(GL_ACCUM_BUFFER_BIT);
#endif

//      glShadeModel(GL_SMOOTH);
//      glDisable(GL_DEPTH_TEST);

//              glPolygonMode(GL_FRONT, GL_FILL);
//              glPolygonMode(GL_BACK, GL_LINE);
//              glDisable(GL_CULL_FACE);
//      glDisable(GL_ARB_texture_non_power_of_two);

    return 1;
}

static int __initSDL_end(
)
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

static void __vid_info(
)
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

static int __initSDL_start(
    const bool opengl,
    const int w,
    const int h,
    const char *name
)
{
    int video_flags = 0;

//      SDL_VideoInfo   const *vinfo;

    video_flags = SDL_HWSURFACE;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
        exit(2);
    }

    printf("OpenGL version: %s\n", glGetString(GL_VERSION));
    printf("OpenGL vendor: %s\n", glGetString(GL_VENDOR));
    printf("OpenGL renderer: %s\n", glGetString(GL_RENDERER));
    printf("setting size: %d %d\n", w, h);

//      SDL_WM_SetIcon( IMG_Load("tea.png"), NULL );
    SDL_WM_SetCaption(name, name);
    __vid_info();

    if (opengl)
    {
        // TODO: add fullscreen
        /* Enable double buffering */
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        video_flags |= R_SDL_SCREENFLAGS;
        //video_flags |= SDL_FULLSCREEN;
        //video_flags |= SDL_RESIZABLE
    }

    in(rSys)->screen = SDL_SetVideoMode(w, h, 32, video_flags);

    return 0;
}

/*----------------------------------------------------------------SET METHODS */

/** Set Renderer options/flags */
void ren_draw_enable(
    int flag
)
{
    switch (flag)
    {
    case R_FULLSCREEN:
        {       /* set to full screen */
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

void ren_draw_disable(
    int flag
)
{
    switch (flag)
    {
    case R_FULLSCREEN:
        {       /* set to full screen */
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

#if 0
/**
 * This somehow fixes a bug. */
static void __fakefix(
)
{
#if 0
    ren_entity_t rent;

    memset(&rent, 0, sizeof(ren_entity_t));
    rent.rotAngle[2] = 10;
    ren_rent_draw(&rent);
#endif
}

/** draw everything to the screen */
void ren_draw_step(
    vec2_t camera_org
)
{
    int ii;
    vec2_t camera = { 0, 0 };

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
//      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    __fakefix();

//    rOGLResetColor();

    ren_objs_draw();
}
#endif

const float vertexPositions[] = {
    -1.0f, 1.0f, 0.0f, 1.0f,
    0.75f, 0.75f, 0.0f, 1.0f,
    0.75f, -0.75f, 0.0f, 1.0f,
    -0.75f, -0.75f, 0.0f, 1.0f,
};

int positionBufferObject;

void InitializeVertexBuffer(
)
{
    glGenBuffers(1, &positionBufferObject);

    glBindBuffer(GL_ARRAY_BUFFER, positionBufferObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions,
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

/** zero the marker */
void ren_frame_begin(
    void
)
{
/*
	glViewport (0,0,in(rSys)->w,in(rSys)->h);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();	
	glOrtho(0.0f, in(rSys)->w,in(rSys)->h, 0.0f, -100, 100);

	glMatrixMode (GL_MODELVIEW);	
	glLoadIdentity ();					
*/

#if 0
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
#endif

#if 0
    /* Set The Viewport To The Top Left.
     * It Will Take Up Half The Screen in(rSys)->w And in(rSys)->h */
//    glViewport(rSys->x, rSys->y, rSys->w, rSys->h);
    glViewport(0, 0, in(rSys)->w, in(rSys)->h);
//    glViewport(rSys->x, rSys->y, 100, 100);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    /* Set Up Ortho Mode To Fit 1/4 The Screen (Size Of A Viewport) */
    //gluOrtho2D(0, window_in(rSys)->w/2, window_in(rSys)->h/2, 0);
//    glOrtho(0.0f, rSys->w, rSys->h, 0.0f, -100, 100);
    glOrtho(0.0f, in(rSys)->w, in(rSys)->h, 0.0f, -100, 100);
//    glScalef(rSys->scale, rSys->scale, 1);
//    glScalef(1.0f, 1.0f, 1.0f);
#endif

//    glMatrixMode(GL_MODELVIEW);
//    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

//    glUseProgram(theProgram);

#if 0
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, 1);



    glBindBuffer(GL_ARRAY_BUFFER, positionBufferObject);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

    glDrawArrays(GL_QUADS, 0, 4);

    glDisableVertexAttribArray(0);
#endif

//    glUseProgram(0);
}

void ren_frame_end(
    void
)
{
//    glFlush();
    SDL_GL_SwapBuffers();
//    glutSwapBuffers();
}

void ren_set_screensize(
    int w,
    int h
)
{
    in(rSys)->w = w;
    in(rSys)->h = h;
}

/** init the renderer sub system */
void ren_draw_init(
    char *name
)
{
    bool opengl = true;

    rSys = calloc(1, sizeof(ren_renderer_t));
    rSys->in = calloc(1, sizeof(__ren_renderer_in_t));
    in(rSys)->w = 600;
    in(rSys)->h = 600;

    __initSDL_start(opengl, in(rSys)->w, in(rSys)->h, name);

    if (opengl)
    {
        __initOpenGL();
    }

    __initSDL_end();

    ren_medias_init();
//    ren_rents_init();


#if 0
    InitializeVertexBuffer();
    glBindBuffer(GL_ARRAY_BUFFER, positionBufferObject);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
#endif
}

/*--------------------------------------------------------------79-characters-*/
/* vim: set expandtab! : */
