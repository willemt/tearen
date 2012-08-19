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


#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "r_media.h"
#include "tea_vec.h"
#include "r_draw.h"

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_opengl.h>

#include "linked_list_queue.h"
#include "fixed_arraylist.h"
#include "linked_list_hashmap.h"

/* use PHYSFS for image loading? */
#if defined(HAVE_PHYSFS_H)
#define PHYSFS 1
#if PHYSFS
#include "physfs.h"
#include "tea_fsrwops.h"
#endif
#endif

#define ERR_S "error"

static int __atlas = -1;


/* Only allow this thread to do graphics.
 * Others will force images onto the initialisation queue */
static int __allowed_thread = -1;

/* This media wants its image data to be initialized.
 * We put it on a queue so that the proper thread can init it. */
static linked_list_queue_t *__media_to_init_queue;

/* Mapping of file name to media */
static hashmap_t *__mediaHashmap = NULL;

/* Mapping of ID to media */
static arraylistf_t *__mediaList = NULL;

/*----------------------------------------------------- OBJECT CLASSIFICATION */
static void __media_image_init_enque(
    media_t * media
)
{
    llqueue_offer(__media_to_init_queue, media);
}

static unsigned long __media_hash(
    const void *e1
)
{
    const media_t *m1 = e1;

    return r_hash_string(m1->fname);
}

static int __media_compare(
    const void *e1,
    const void *e2
)
{
    const media_t *m1 = e1, *m2 = e2;

    return strcmp(m1->fname, m2->fname);
}

/*----------------------------------------------------------------------------*/

/**
 * Get this media from this ID */
void *ren_medias_get(
    const int idx
)
{
    media_t *media;


    /* initialise new media list if necessary */
    if (NULL == __mediaList)
    {
        __mediaList = arraylistf_new();
    }

    media = arraylistf_get(__mediaList, idx - 1);

    /* get the emergency fall back image */
    if (NULL == media)
    {
        return arraylistf_get(__mediaList, 0);
    }
    else
    {
        assert(NULL != media);
        return media;
    }
}

/*----------------------------------------------------------------------------*/

static void __media_release(
    media_t * media
)
{
    arraylistf_remove(__mediaList, media->id);
    SDL_FreeSurface(media->surface);
    glDeleteTextures(1, &media->glImage);
    free(media->fname);
    free(media);
}

/**
 * release the media's used memory, and remove it from the media DB */
void ren_media_release(
    int idx
)
{
    media_t *media;

    if (NULL == __mediaList)
    {
        printf(ERR_S "mediaList un-initialized\n");
        assert(false);
    }

    media = ren_medias_get(idx);

    assert(media);
    media->refCount--;

    /* make sure reference count makes sense */
    if (media->refCount < 0)
    {
        printf(ERR_S "refcount below zero! (media:%d refcount:%d)\n",
               idx, media->refCount);
    }
    /* only release when refcount is 0 */
    else if (0 == media->refCount)
    {
        __media_release(media);
    }
}

/*----------------------------------------------------------------------------*/

/**
 * Get the texture that this media uses
 * @return 0 on error
 */
int ren_media_get_texture(
    const int idx
)
{
    media_t *media = ren_medias_get(idx);

    assert(media);

    return ren_texture_atlas_get_texture(__atlas);
}

/**
 * Get the texture coordinates of this texture as it may be sitting on a texture
 * atlas */
void ren_media_get_texturecoords(
    int idx,
    vec2_t begin,
    vec2_t end
)
{
    media_t *media = ren_medias_get(idx);

    assert(media);

    ren_texture_atlas_get_coords_from_texid(__atlas, media->glImage, begin,
                                            end);
}

const char *ren_media_get_filename(
    const void *media
)
{
    return ((media_t *) media)->fname;
}

void ren_media_resize_w_by_realw(
    const int idx,
    const int w,
    float *w_out
)
{
    media_t *media = ren_medias_get(idx);

    *w_out = ((float) w / (float) media->h) * (float) media->hReal;
}

void ren_media_resize_h_by_realh(
    const int idx,
    const int h,
    float *h_out
)
{
    media_t *media = ren_medias_get(idx);

    *h_out = ((float) h / (float) media->h) * (float) media->hReal;
}

/*----------------------------------------------------------------------------*/

static bool __texture_size_is_possible(
    int w,
    int h
)
{
    GLint width;

    glTexImage2D(GL_PROXY_TEXTURE_2D, 0, GL_RGBA, w, h, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
//    printf("%d,%d %d, \n", w, h, width);

    return (width != 0);
}

#define MIN_TEXTURE_SIZE 32

/**
 * Find out what the maximum texture size is
 * */
static void __get_max_texture_size(
    int *w,
    int *h
)
{
    int ii;

    for (ii = 0; ii < 10; ii++)
    {
        *w = *h = MIN_TEXTURE_SIZE << ii;

        if (!__texture_size_is_possible(*w, *h))
        {
            *w = *h = MIN_TEXTURE_SIZE << (ii - 1);
            return;
        }
    }
}

static int __create_blank_texture(
    const int w,
    const int h
)
{
    GLuint image = 0;

    unsigned char data[w * h * 4];

    int bpp;

    glGenTextures(1, &image);
    glBindTexture(GL_TEXTURE_2D, image);
    bpp = GL_RGBA;

    memset(data, 0, sizeof(unsigned char) * w * h);
#if 0
    int ii, jj;

    for (ii = 0; ii < w; ii++)
        for (jj = 0; jj < h; jj++)
            data[ii * w + jj] = 127;
#endif

    /* error checking for maximum size of textures? */
    glTexImage2D
        (GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, bpp, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    return image;
}

static SDL_Surface *__load_image(
    const char *fname,
    int *w,
    int *h
)
{
    SDL_Surface *surf;

    if (!(surf = (SDL_Surface *) r_load_image(fname)))
    {
        printf(ERR_S "bad file: '(%s) ' \n ", fname);
        return NULL;
    }

    *w = surf->w;
    *h = surf->h;
    return surf;
}

static void __write_pixels_to_texture(
    const unsigned char pixels[],
    const rect_t * rect,
    const unsigned int texture
)
{
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0,
                    rect->x, rect->y, rect->w, rect->h,
                    GL_RGBA, GL_UNSIGNED_BYTE, pixels);
}

/*----------------------------------------------------------------------------*/
static void __media_init_image(
    media_t * media
)
{
    if (__atlas == -1)
    {
        int w, h;

        __get_max_texture_size(&w, &h);
//        printf("MAXIMUM texture size: %d %d\n", w, h);

        __atlas =
            ren_texture_atlas_init(512, __write_pixels_to_texture,
                                   __create_blank_texture);
    }

    if (media->fname)
    {
        SDL_Surface *surf;

        int w, h;

        if (!(surf = __load_image(media->fname, &w, &h)))
        {
            printf(ERR_S "couldn't load image: %s\n", media->fname);
            return;
        }

        media->glImage =
            ren_texture_atlas_push_pixels(__atlas, surf->pixels, w, h);
//        media->glImage = ren_texture_atlas_push_file(__atlas, media->fname);
        SDL_FreeSurface(surf);
    }
}

static media_t *__register_media(
    const char *fname
)
{
    media_t *media;

    media = calloc(1, sizeof(media_t));
    media->fname = strdup(fname);
    return media;
}

/**
 * get the media associate with this file name
 * automatically load the media into graphics memory
 * enqueue the media loading if we are in another thread
 *
 * @return 0 on error
 */
int ren_media_get(
    char *fname
)
{
    media_t *media;

    int idx = 0;

//      printf("image %s\n", fname);

    if (!__mediaList)
    {
        /* new media */
        __mediaList = arraylistf_new();
    }
    else
    {
        if ((media = hashmap_get(__mediaHashmap, fname)))
        {
            idx = media->id;
        }
    }

    if (0 < idx)
    {
        media = ren_medias_get(idx);
    }
    else
    {
        media = __register_media(fname);
        idx = 1 + arraylistf_add(__mediaList, media);
        media->id = idx;
    }

    if (0 == media->refCount)
    {
#if 0
        if (SDL_ThreadID() != __allowed_thread)
        {
            __media_image_init_enque(media);
        }
        else
        {
            __media_init_image(media);
        }
#else
        __media_init_image(media);
#endif
    }

    media->refCount++;
    return idx;
}

/*----------------------------------------------------------------------------*/
/**
 * Step through and ensure that all polled media is processed. */
void ren_medias_step(
)
{
    while (0 < llqueue_count(__media_to_init_queue))
    {
        media_t *media;

        media = llqueue_poll(__media_to_init_queue);
        __media_init_image(media);
    }
}

void ren_medias_allow_thread(
    int threadid
)
{
    __allowed_thread = threadid;
}

void ren_medias_init(
)
{
    __media_to_init_queue = llqueue_new();
    __mediaHashmap = hashmap_new(r_hash_string, r_cmp_string);
}

/*----------------------------------------------------------------------------*/

/**
 * Loads an image and returns it as an SDL_Surface* casted as void*
 * Can uses a physfs filesystem
 * @return a SDL_Surface */
void *r_load_image(
    const char *fname
)
{
    SDL_Surface *surf = NULL;

#if defined(HAVE_PHYSFS_H)
    void *file = NULL;
    SDL_RWops *sdl_fd;

    sdl_fd = fsRWOPS_openRead(fname, &file);
#endif
    /* if a *.bmp, use the bmp loader */
    if (strstr(fname, ".bmp"))
    {
        if ((surf = (SDL_Surface *) SDL_LoadBMP(fname)) == NULL)
        {
            printf(ERR_S "couldn't find shader: %s\n", fname);
        }
    }
    else if (strstr(fname, ".png"))
    {
//              if ((surf = (SDL_Surface*)IMG_Load(fname)) == NULL) {
#if defined(HAVE_PHYSFS_H)
        if (NULL == (surf = (SDL_Surface *) IMG_Load_RW(sdl_fd, 0)))
        {
#else
        if ((surf = (SDL_Surface *) IMG_Load(fname)) == NULL)
        {
#endif
            printf(ERR_S "couldn't find shader: %s\n", fname);
        }
        else
        {
        }
    }

#if defined(HAVE_PHYSFS_H)
    if (sdl_fd)
    {
        sdl_fd->close(sdl_fd);
        if (file)
        {
//                      printf("closed\n");
            fs_file_close(file);
        }
    }
#endif

    return surf;
}
