
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

#include <GL/glew.h>
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

//      __initShaders();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glClearColor(0.4f, 0.4f, 0.4f, 0.0f);

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
    const int h
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

//      SDL_WM_SetIcon( IMG_Load("tea.png"), NULL );
//    SDL_WM_SetCaption("TEAREN", "TEAREN");
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
//      in(rSys)->screen = SDL_SetVideoMode(rSys->w, rSys->h, 32, video_flags);

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

/**
 * This somehow fixes a bug. */
static void __fakefix(
)
{
    ren_entity_t rent;

    memset(&rent, 0, sizeof(ren_entity_t));
    rent.rotAngle[2] = 10;
    ren_rent_draw(&rent);
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

    rOGLResetColor();

    ren_rents_draw();
}


#if 0
static void __grid_draw(
)
{
    int i;

#define SEGMENTS 20

    glColor4f(1, 1, 1, 1);

    rOGLDrawLine(rSys->x + 1, rSys->y + 1, rSys->x + rSys->w - 1, rSys->y + 1,
                 0, 0);
    rOGLDrawLine(rSys->x + rSys->w - 1, rSys->y + 1, rSys->x + rSys->w - 1,
                 rSys->y + rSys->h - 1, 0, 0);
    rOGLDrawLine(rSys->x + rSys->w - 1, rSys->y + rSys->h - 1, rSys->x + 1,
                 rSys->y + rSys->h - 1, 0, 0);
    rOGLDrawLine(rSys->x + 1, rSys->y + rSys->h - 1, rSys->x + 1, rSys->y + 1,
                 0, 0);

    vec2_t end = { rSys->x + rSys->w, rSys->y + rSys->h };
    vec2_t org = { rSys->x, rSys->y };

    vec2_t sub;

    vec2Subtract(end, org, sub);

    float dist = vec2Normalize(sub);

    for (i = 0; i < SEGMENTS - 1; i++)
    {
        vec2_t v1, v2;

        if (i % 2)
            continue;
        vec2MA(org, (float) i / SEGMENTS * dist, sub, v1);
        vec2MA(org, (float) (i + 1) / SEGMENTS * dist, sub, v2);
        rOGLDrawLine2(v1, v2, 0, 0);    // left down
    }


    org[X] += end[X];
    end[X] -= org[X];
//      vec2Set(org, rSys->x+rSys->w, rSys->y);
//      vec2Set(end, rSys->x, rSys->y+rSys->h);

    vec2Subtract(end, org, sub);
    dist = vec2Normalize(sub);

    for (i = 0; i < SEGMENTS - 1; i++)
    {
        vec2_t v1, v2;

        if (i % 2)
            continue;
        vec2MA(org, (float) i / SEGMENTS * dist, sub, v1);
        vec2MA(org, (float) (i + 1) / SEGMENTS * dist, sub, v2);
        rOGLDrawLine2(v1, v2, 0, 0);    // left down
    }
}
#endif

void ren_draw_begin(
)
{
//      static int list = -1;

#if 1
/*	if (-1 == list) {
		list = glGenLists(1);

		glNewList(list,GL_COMPILE);
*/
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
    glScalef(1.0f, 1.0f, 1.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

#if 0
    if (rSys->zoom != 1)
    {
        glScalef(rSys->zoom, rSys->zoom, 1);
    }
#endif

#endif
/*		glEndList();
	}

	glCallList(list);
*/

//      glEnable(GL_SCISSOR_TEST);
//      glScissor(rSys->x, rSys->y, rSys->w, rSys->h);

/*
	glPushMatrix ();
	glEnable(GL_SCISSOR_TEST);
	glScissor(rSys->x, in(rSys)->h-rSys->h-rSys->y, rSys->w, rSys->h);
	glTranslatef(rSys->x,rSys->h-in(rSys)->h+rSys->y, 0);  // Position 
// 	glTranslatef(rSys->x-(rSys->scale-1)*512,rSys->y-(rSys->scale-1)*rSys->h, -50);  // Position 
	glScalef(rSys->scale,rSys->scale,1);
//	glTranslatef(128,128, -50);  // Position 
*/

//      rSky();
//      if (localclient.cameraTarget) vec2Copy(localclient.cameraTarget->org_terp, camera);
//      if (session.maploaded) rTilesDrawMain( localclient.scrOrg[0], localclient.scrOrg[1], camera, false );
}

/** zero the marker */
void ren_beginFrame(
    void
)
{
    ren_rents_beginFrame();
/*
	glViewport (0,0,in(rSys)->w,in(rSys)->h);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();	
	glOrtho(0.0f, in(rSys)->w,in(rSys)->h, 0.0f, -100, 100);

	glMatrixMode (GL_MODELVIEW);	
	glLoadIdentity ();					
*/

#if ACCUM_BUFFER
    glClearAccum(0, 0, 0, 1);
#endif
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT); //|GL_ACCUM_BUFFER_BIT);

#if 0
    if (in(rSys)->testgrid)
    {
        __grid_draw();
    }
#endif
}

void ren_endFrame(
    void
)
{
#if ACCUM_BUFFER
    float val = 0.95;

    glAccum(GL_MULT, val);
    glAccum(GL_ACCUM, 1.0 - val);
    glAccum(GL_RETURN, 1.0);
#endif

    //glFlush();
    SDL_GL_SwapBuffers();
}

/** init the renderer sub system */
void ren_draw_init(
)
{
    bool opengl = true;

    __initSDL_start(opengl, in(rSys)->w, in(rSys)->h);

    if (opengl)
    {
        __initOpenGL();
    }

    __initSDL_end();

    ren_medias_init();
    ren_rents_init();
    ren_effects_init();
}

/*--------------------------------------------------------------79-characters-*/
/* vim: set expandtab! : */
