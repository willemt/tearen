/* 
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
* Tactical Elemental Arenas
*  C Implementation: r_media 
*
*
*
* Author: Willem-Hendrik Thiart <beandaddy@gmail.com>, (C) 2006
* Copyright: See COPYING file that comes with this distribution
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*/

#include <stdlib.h>
#include "r_local.h"
#include "r_draw.h"
#include "tea_datatype.h"
#include "tea_xml.h"
#include "tea_utils.h"

//#include "arrayqueue.h"
//#include "fixed_arraylist.h"
//#include "linked_list_hashmap.h"

/* use PHYSFS for image loading? */
#if defined(HAVE_PHYSFS_H)
#define PHYSFS 1
#if PHYSFS
#include "physfs.h"
#include "tea_fsrwops.h"
#endif
#endif

static int __atlas = -1;

/*----------------------------------------------------- OBJECT CLASSIFICATION */

/* only allow this thread to do init graphics.
 * others will force images onto the init queue */
static int __allowed_thread = -1;

/* this media wants its image data to be init'd.
 * We should put it on a queue so that the proper thread can init it. */
static tea_arrayqueue_t *__media_to_init_queue;

//tea_hashmap_t*                _hash = NULL;
static tea_alistf_t *__mediaList = NULL;

static void __media_image_init_enque(
    media_t * media
)
{
    tea_arrayqueue_offer(__media_to_init_queue, media);
}

static ulong __media_hash(
    const void *e1
)
{
    const media_t *m1 = e1;

    return teaString_hash(m1->fname);
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

static tea_object_t mediaObj = {
    __media_compare,
    __media_hash,
    NULL,
};

void *ren_medias_get(
    const int idx
)
{
    media_t *media;

//      return &ren_media[idx];

    if (NULL == __mediaList)
    {
        /* new media */
        __mediaList = tea_alistf_initalloc(&mediaObj);
    }
    //if (NULL == __mediaList) {
    //      tea_msg(mREN, 0, "ERROR: __mediaList un-initialized\n");
    //      assert(FALSE);
    //}

    media = tea_alistf_get(__mediaList, idx - 1);

    /* get the emergency fall back image, prettttttty ugly */
    if (NULL == media)
    {
        return tea_alistf_get(__mediaList, 0);
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
    tea_alistf_removeItem(__mediaList, media);
//      free(media->surface);
//      rGraphic_realse(media->fname, media, NULL, R_INITSHADER_BILINEAR)) {
    SDL_FreeSurface(media->surface);
    glDeleteTextures(1, &media->glImage);
    free(media->fname);
    free(media);
//    tea_pool_free(media);
}

void ren_media_release(
    int idx
)
{
    if (NULL == __mediaList)
    {
        tea_msg(mREN, 0, ERR_S "mediaList un-initialized\n");
        assert(FALSE);
    }

    media_t *media;

    media = ren_medias_get(idx);

    assert(media);
    media->refCount--;
//      assert(media->refCount

    if (media->refCount < 0)
    {
        tea_msg(mREN, 0, "ERROR: refcount below zero! (media:%d refcount:%d)\n",
                idx, media->refCount);
    }
    else if (0 == media->refCount)
    {
        __media_release(media);
    }
}

/*----------------------------------------------------------------------------*/

/*
 * 
 * @return 0 on error
 */
int ren_media_get_texture(
    int idx
)
{
    media_t *media = ren_medias_get(idx);

    assert(media);
//    return media->glImage;

    return ren_texture_atlas_get_glimage(__atlas);
}

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
    int idx,
    int w,
    float *w_out
)
{
    media_t *media = ren_medias_get(idx);

    *w_out = ((float) w / (float) media->h) * (float) media->hReal;
}

void ren_media_resize_h_by_realh(
    int idx,
    int h,
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

/*
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
    GLuint image = 0;           //This is our texture

    int bpp;

    glGenTextures(1, &image);
    glBindTexture(GL_TEXTURE_2D, image);
    bpp = GL_RGBA;
    unsigned char data[w * h * 4];

    memset(data, 0, sizeof(unsigned char) * w * h);
#if 0
    int ii, jj;

    for (ii = 0; ii < w; ii++)
        for (jj = 0; jj < h; jj++)
            data[ii * w + jj] = 127;
#endif

    // error checking for maximum size of textures?
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

    if (!(surf = (SDL_Surface *) rLoadImageFile(fname)))
    {
        tea_msg(mREN, 0, ERR_S "bad file: '(%s) ' \n ", fname);
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
#if 0
    if (strlen(media->fname) == 0)
    {
#if 1
        media->inuse = true;
        media->surface =
            SDL_CreateRGBSurface(SDL_SWSURFACE, 32, 32, 32, 0, 0, 0, 0);
        if (media->surface == NULL)
        {
            fprintf(stderr, "rmedia: couldn't create lcont surface: %s\n",
                    SDL_GetError());
            exit(1);
        }

        media->glImage = rOpenglTexture(media->surface, R_INITSHADER_BILINEAR);

//        if (src_rect)
//            media->surface = crop(media->surface, src_rect);

        media->w = media->surface->w;
        media->h = media->surface->h;
//      media->surface = surf2TaglessSurf(media->surface);
//      media->surface = rPowerOf2erizer(media->surface);
        media->wReal = media->surface->w;
        media->hReal = media->surface->h;
        media->glImage = rOpenglTexture(media->surface, R_INITSHADER_BILINEAR);
        SDL_FreeSurface(media->surface);

        return;
#endif
    }
    else if (0 <= initShader(media->fname, media, NULL, R_INITSHADER_BILINEAR))
    {

    }
#endif


#if 1
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
            tea_msg(mREN, 0, ERR_S "couldn't load image: %s\n", media->fname);
            return;
        }
        media->glImage =
            ren_texture_atlas_push_pixels(__atlas, surf->pixels, w, h);
//        media->glImage = ren_texture_atlas_push_file(__atlas, media->fname);
        SDL_FreeSurface(surf);
    }
#endif
}

static media_t *__register_media(
    const char *fname
)
{
    media_t *media;

//    media = tea_pool_malloc_zero(sizeof(media_t));
    media = calloc(1, sizeof(media_t));
    media->fname = strdup(fname);
    tea_msg(mREN, 0, "reg'd media: %s\n", media->fname);
    return media;
}

/*
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
        __mediaList = tea_alistf_initalloc(&mediaObj);
    }
    else
    {
        /* check if we already have this file loaded */
        idx = 1 + tea_alistf_itemIndex(__mediaList, &(media_t)
                                       {
                                       .fname = fname}
        );
    }

    if (0 < idx)
    {
        media = ren_medias_get(idx);
    }
    else
    {
        media = __register_media(fname);
        idx = 1 + tea_alistf_add(__mediaList, media);
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
void ren_medias_step(
)
{
    while (!tea_arrayqueue_isEmpty(__media_to_init_queue))
    {
        media_t *media;

        media = tea_arrayqueue_poll(__media_to_init_queue);
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
    __media_to_init_queue = tea_arrayqueue_initalloc(NULL);
}
