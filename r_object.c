/*
 * ============================================================================
 *
 *       Filename:  r_object.c
 *
 *    Description:  renderable object - state based rendering
 *
 *        Version:  1.0
 *        Created:  12/12/10 19:19:39
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *        Company:  
 *
 * ============================================================================
 */

/*
 * 
 * multiple VBOs
 *  put verts in VBO according to GLTexture
 *  1 VBO for each GLTexture
 *
 *
 *
 */

//#include "r_local.h"
#include <stdbool.h>
#include "tea_vec.h"
#include "r_draw.h"
//#include "r_opengl.h"
#include <GL/gl.h>
//#include <opengl/OpenGL.h>
#include "linked_list_hashmap.h"
#include "fixed_arraylist.h"
//#include "linked_list_hashmap.h"
//#include "fixed_arraylist.h"

typedef struct
{
    vec2_t org;
    int type;
    int id;
    void *typedata;
} __obj_t;

typedef struct
{
    int media;
    int w;
    vec2_t rot_org;
    float zrot;
    int vbo_slot;
} __square_t;

typedef struct
{
    int media;
    int w, h;
    int vbo_slot;
} __rect_t;

#define in(x) ((__obj_t*)x->in)
#define square(x) ((__square_t*)in(x)->typedata)
#define rect(x) ((__rect_t*)in(x)->typedata)

/** default size of each VBO */
#define VBO_ELEM_SIZE 1000


/** Internal list of objects */
static arraylistf_t *__robj_list = NULL;

/** Internal list of VBOs by texture */
static hashmap_t *__vbosByTexture = NULL;

/*----------------------------------------------------------------------------*/
static void __media_texturecoords_2_gltexturecoords(
    const int media_id,
    ren_vertex_tc_t verts[4]
)
{
    vec2_t begin, end;

#if 1
    ren_media_get_texturecoords(media_id, begin, end);
#if 0
    vec2Set(verts[0].tex, begin[0], begin[1]);
    vec2Set(verts[1].tex, begin[0], end[1]);
    vec2Set(verts[2].tex, end[0], end[1]);
    vec2Set(verts[3].tex, end[0], begin[1]);
#endif
    vec2Set(verts[0].tex, end[0], begin[1]);
    vec2Set(verts[1].tex, begin[0], begin[1]);
    vec2Set(verts[2].tex, begin[0], end[1]);
    vec2Set(verts[3].tex, end[0], end[1]);
#else
    vec2Set(verts[0].tex, 0, 0);
    vec2Set(verts[1].tex, 0, 1);
    vec2Set(verts[2].tex, 1, 1);
    vec2Set(verts[3].tex, 1, 0);
#endif
}

/*----------------------------------------------------------------------------*/
/**
 * @return 0 = error; otherwise VBO ID
 */
static int __get_vbo_from_texture(
    unsigned long texture
)
{
    unsigned long vbo;

    vbo =
        (unsigned long) (void *) hashmap_get(__vbosByTexture, (void *) texture);

    if (0 == vbo)
    {
        vbo = ren_vbom_init(VBO_ELEM_SIZE);
        hashmap_put(__vbosByTexture, (void *) texture, (void *) vbo);
    }

    return vbo;
}

/**
 * @return VBO ID that object is on */
static int __ren_obj_get_vbo(
    ren_object_t * rob
)
{
    int tex, vbo;

    switch (in(rob)->type)
    {
    case RENT_SQUARE:
    case RENT_SQUARE_CENTER:
        {
            int tex;

            tex = ren_media_get_texture(square(rob)->media);
            return __get_vbo_from_texture(tex);
        }
        break;
    default:
        assert(false);
        break;
    }

    return 0;
}

#if 0

/*----------------------------------------------------------------------------*/
static int __ren_obj_compare(
    const void *e1,
    const void *e2
)
{
    const ren_entity_t *r1, *r2;

    r1 = e1;
    r2 = e2;

    int diff;

    diff = r1->z - r2->z;

    if (0 == diff)
    {
        diff = r2->enqueue_order - r1->enqueue_order;
        assert(0 != diff);
        return diff;
    }

    return diff;
}

static tea_object_t __objRentZ = {
    __ren_entity_compare_z,
    NULL,
    NULL,
};
#endif

/**
 * Robject destructor */
int ren_obj_release(
    ren_object_t * rob
)
{
    void *removed;

    removed = arraylistf_remove(__robj_list, in(rob)->id);

    assert(rob == removed);

    switch (in(rob)->type)
    {
    case RENT_SQUARE:
    case RENT_SQUARE_CENTER:
        {
//            printf("releasing square rom vbo: %d\n", in(rob)->id);
            ren_vbom_release_itemslot(__ren_obj_get_vbo(rob),
                                      square(rob)->vbo_slot);
            free(in(rob)->typedata);
        }
        break;
    case RENT_RECT:
        {
//            printf("releasing rect from vbo: %d\n", in(rob)->id);
            ren_vbom_release_itemslot(__ren_obj_get_vbo(rob),
                                      rect(rob)->vbo_slot);
            free(in(rob)->typedata);
        }
        break;
    default:
        assert(false);
        break;
    }

    free(rob->in);
    free(rob);
    return 1;
}

/**
 * Release object and set ptr to NULL */
int ren_obj_release_unitialise(
    ren_object_t ** rob
)
{
    int ret;

    if (!*rob)
        return 0;

    ret = ren_obj_release(*rob);
    *rob = NULL;
    return ret;
}

static void __manage_vboslot_with_new_textures(
    /*  vbo item slot */
    int *vslot,
    /*  media new */
    const int m_new,
    /*  media old */
    const int m_old
)
{
    int tex, vbo;

    tex = ren_media_get_texture(m_new);
    vbo = __get_vbo_from_texture(tex);

    if (!vbo)
        return;

    /*  if un-initialised, just create new itemslot from vbo */
    if (m_old == 0)
    {
//        printf("new vslot\n\n\n\n");

        *vslot = ren_vbom_new_itemslot(vbo);
    }
    /*  otherwise check if we need to change VBOs */
    else
    {
        int old_tex;

        old_tex = ren_media_get_texture(m_old);

        /* yes there was a VBO change */
        if (tex != old_tex)
        {
            int old_vbo;

            old_vbo = __get_vbo_from_texture(old_tex);
            ren_vbom_release_itemslot(old_vbo, *vslot);
            *vslot = ren_vbom_new_itemslot(vbo);
        }
    }
}

/**
 * Set the current media ID of this object.
 * Used for texturing the object
 * */
int ren_obj_set_media(
    ren_object_t * rob,
    const int media_id
)
{
//    void *vbo;

    tea_precond(rob);
    tea_precond(rob->in);

    switch (in(rob)->type)
    {
    case RENT_SQUARE:
    case RENT_SQUARE_CENTER:
        {
            __manage_vboslot_with_new_textures(&square(rob)->vbo_slot,
                                               media_id, square(rob)->media);
            square(rob)->media = media_id;
        }
        break;
    case RENT_RECT:
        {
            __manage_vboslot_with_new_textures(&rect(rob)->vbo_slot,
                                               media_id, rect(rob)->media);
            rect(rob)->media = media_id;
        }
        break;
    default:
        assert(false);
        break;
    }

    return 1;
}

/**
 * @return media id of object */
int ren_obj_get_media(
    ren_object_t * rob
)
{
    tea_precond(rob);
    tea_precond(rob->in);
    switch (in(rob)->type)
    {
    case RENT_SQUARE:
    case RENT_SQUARE_CENTER:
        return square(rob)->media;
        break;
    case RENT_RECT:
        return rect(rob)->media;
        break;
    default:
        assert(false);
        return 0;
        break;
    }
}

/** Set rotational origin of object */
int ren_obj_set_rot_org(
    ren_object_t * rob,
    vec2_t rot_org
)
{
    tea_precond(rob);
    tea_precond(rob->in);
    switch (in(rob)->type)
    {
    case RENT_SQUARE:
        vec2Copy(rot_org, square(rob)->rot_org);
#if 0
    case RENT_RECT:
        vec2Copy(rot_org, rect(rob)->rot_org);
        break;
#endif
    default:
        assert(false);
        break;
    }

    return 0;
}

/** Set Z rotation angle of object */
int ren_obj_set_zrot(
    ren_object_t * rob,
    float zrot
)
{
    tea_precond(rob);
    tea_precond(rob->in);
    switch (in(rob)->type)
    {
    case RENT_SQUARE:
    case RENT_SQUARE_CENTER:
        square(rob)->zrot = zrot;
        break;
    default:
        assert(false);
        break;
    }

    return 1;
}

/**
 * @return width of object */
int ren_obj_get_w(
    ren_object_t * rob
)
{
    tea_precond(rob);
    tea_precond(rob->in);
    switch (in(rob)->type)
    {
    case RENT_SQUARE:
    case RENT_SQUARE_CENTER:
        return square(rob)->w;
    case RENT_RECT:
        return rect(rob)->w;
    default:
        assert(false);
        break;
    }

    return 0;
}

/**
 * @return height of object */
int ren_obj_get_h(
    ren_object_t * rob
)
{
    tea_precond(rob);
    tea_precond(rob->in);
    switch (in(rob)->type)
    {
    case RENT_SQUARE:
    case RENT_SQUARE_CENTER:
        return square(rob)->w;
    case RENT_RECT:
        return rect(rob)->h;
    default:
        assert(false);
        break;
    }

    return 0;
}

/**
 * set origin of object
 *
 * */
int ren_obj_set_org(
    ren_object_t * rob,
    vec2_t org
)
{
    tea_precond(rob);
    tea_precond(rob->in);
    switch (in(rob)->type)
    {
    case RENT_SQUARE:
    case RENT_SQUARE_CENTER:
        {
            vec2Copy(org, in(rob)->org);

            ren_vertex_tc_t verts[4];

            int vbo, vbo_slot;

            int media_id;

            int w = square(rob)->w;

            media_id = square(rob)->media;
//            vbo = __get_vbo_from_texture(ren_media_get_texture(media_id));
            vbo = __ren_obj_get_vbo(rob);
            vbo_slot = square(rob)->vbo_slot;

#if 0
            printf("%f,%f %d %d vbo:%d vslot:%d %lx\n", org[0], org[1],
                   media_id, ren_media_get_texture(media_id), vbo, vbo_slot,
                   rob);
#endif

            vec3Set(verts[0].pos, org[0], org[1], 0);
            vec3Set(verts[1].pos, org[0] + w, org[1], 0);
            vec3Set(verts[2].pos, org[0] + w, org[1] + w, 0);
            vec3Set(verts[3].pos, org[0], org[1] + w, 0);
            __media_texturecoords_2_gltexturecoords(media_id, verts);
            ren_vbom_item_set_vertices(vbo, vbo_slot, verts, 4);
        }
        break;
    default:
        assert(false);
        break;
    }

    return 0;
}

/**
 * Set origin using X and Y variables
 *
 * */
int ren_obj_set_org_from_xy(
    ren_object_t * rob,
    const float x,
    const float y
)
{
    tea_precond(rob);
    tea_precond(rob->in);
    switch (in(rob)->type)
    {
    case RENT_SQUARE:
    case RENT_SQUARE_CENTER:
    case RENT_RECT:
        {
            vec2_t org;

            vec2Set(org, x, y);
            ren_obj_set_org(rob, org);
        }
        break;
    default:
        assert(false);
        break;
    }

    return 1;
}

/**
 * Set width of object */
int ren_obj_set_w(
    ren_object_t * rob,
    int w
)
{
    tea_precond(rob);
    tea_precond(rob->in);
    switch (in(rob)->type)
    {
    case RENT_SQUARE_CENTER:
        vec2Set(square(rob)->rot_org, (float) w / 2, (float) w / 2);
    case RENT_SQUARE:
        square(rob)->w = w;
        break;
    case RENT_RECT:
        rect(rob)->w = w;
        break;
    default:
        assert(false);
        break;
    }

    return 1;
}

/**
 * Set height of object */
int ren_obj_set_h(
    ren_object_t * rob,
    int h
)
{
    tea_precond(rob);
    tea_precond(rob->in);
    switch (in(rob)->type)
    {
    case RENT_SQUARE_CENTER:
        vec2Set(square(rob)->rot_org, (float) h / 2, (float) h / 2);
    case RENT_SQUARE:
        square(rob)->w = h;
        break;
    case RENT_RECT:
        rect(rob)->h = h;
        break;
    default:
        assert(false);
        break;
    }

    return 1;
}

#if 0
static void __mod(
    const int vbo_verts,
    int elem,
    ren_entity_t * rent,
    int media_id
)
{
    ren_vertex_tc_t verts[4];

    int w = 768;

    vec3Set(verts[0].pos, rent->org[0], rent->org[1], 0);
    vec3Set(verts[1].pos, rent->org[0] + w, rent->org[1], 0);
    vec3Set(verts[2].pos, rent->org[0] + w, rent->org[1] + w, 0);
    vec3Set(verts[3].pos, rent->org[0], rent->org[1] + w, 0);
    __media_texturecoords_2_gltexturecoords(media_id, verts);
    ren_vbom_item_set_vertices(vbo_verts, elem, verts, 4);
}

static void __draw(
    int vbo_verts,
    int n_elems,
    int media_id
)
{
    int image;

//    char *str = "found %d %d %d %d %s \n";
    image = ren_media_get_texture(media_id);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, image);
    ren_vbom_draw(vbo_verts, 0, n_elems);
    glBindTexture(GL_TEXTURE_2D, 0);
}
#endif

static unsigned long __ulong_hash(
    const void *e1
)
{
    const long i1 = (unsigned long) e1;

    assert(i1 >= 0);
    return i1;
}

static long __ulong_compare(
    const void *e1,
    const void *e2
)
{
    const long i1 = (unsigned long) e1, i2 = (unsigned long) e2;

//      return !(*i1 == *i2); 
    return i1 - i2;
}

/**
 * Object constructor
 *
 * @return new renderable object
 * */
ren_object_t *ren_obj_init(
    const int type
)
{
    ren_object_t *rob;

    if (!__vbosByTexture)
    {
        __vbosByTexture = hashmap_new(__ulong_hash, __ulong_compare);
    }

    if (!__robj_list)
    {
        __robj_list = arraylistf_new();
    }


    rob = calloc(1, sizeof(ren_object_t));
    rob->in = calloc(1, sizeof(__obj_t));
    in(rob)->id = arraylistf_add(__robj_list, rob);
    in(rob)->type = type;
//    printf("CREATING ROB: %d\n", in(rob)->id);

    switch (type)
    {
    case RENT_SQUARE:
    case RENT_SQUARE_CENTER:
        in(rob)->typedata = calloc(1, sizeof(__square_t));
        break;
#if 0
    case RENT_RECT:
        in(rob)->typedata = calloc(1, sizeof(__rect_t));
        break;
    case RENT_POSTPIXELS:
        break;
    case RENT_TEXT:
        break;
    case RENT_TEXT_SHADOWED:
        break;
    case RENT_SKELETON:
        break;
    case RENT_LIGHT:
        break;
    case RENT_CALLBACK:
        break;
    case RENT_QUAD:
        break;
    case RENT_BEAM:
        break;
    case RENT_ARCHIVE:
        break;
#endif
    default:
        assert(false);
        break;
    }

    return rob;
}

/**
 *
 * Note: For convienence mainly
 *
 */
int ren_obj_init_ptr(
    ren_object_t ** rob,
    const int type
)
{
    if (NULL == *rob)
    {
        *rob = ren_obj_init(type);
        return 1;
    }

    return 0;
}

/*----------------------------------------------------------------------------*/
static void __rentity_from_obj(
    ren_entity_t * rent,
    ren_object_t * rob
)
{
    ren_rent_clean(rent);
    rent->type = in(rob)->type;
    switch (in(rob)->type)
    {
    case RENT_SQUARE_CENTER:
        rent->type = RENT_SQUARE;
    case RENT_SQUARE:
        {
//            rent->type = RENT_SQUARE;    //in(rob)->type;
            rent->rect.media = square(rob)->media;
            rent->rect.w = square(rob)->w;
            vec2Copy(in(rob)->org, rent->org);
//            vec2Set(rent->org, 500, 500);
            rent->rotAngle[0] = square(rob)->zrot;
            vec2Copy(square(rob)->rot_org, rent->rotCentre);
            //img_asset_get("items.gold");
//            ren_draw_push(&rent);
        }
        break;
    case RENT_RECT:
        {
            rent->rect.media = rect(rob)->media;
            rent->rect.w = rect(rob)->w;
            rent->rect.h = rect(rob)->h;
            vec2Copy(in(rob)->org, rent->org);
        }
        break;
    default:
        assert(false);
        break;
    }

}

/**
 * Draw object
 *
 * */
int ren_obj_draw(
    ren_object_t * rob
)
{
    assert(false);

    ren_entity_t rent;

    __rentity_from_obj(&rent, rob);
//    ren_rent_draw(&rent);
    return 1;
}

#if 0
int ren_obj_push(
    ren_object_t * rob
)
{
    assert(false);

    ren_entity_t rent;

    int media;

    int vbo;

    media = ren_obj_get_media(rob);
    vbo = __get_vbo_from_texture(ren_media_get_texture(media));

    __rentity_from_obj(&rent, rob);
    __mod(vbo, 0, &rent, media);
//    ren_draw_push(&rent);
    return 1;
}
#endif

static void __draw_vbo_from_texture(
    int texture,
    void *udata
)
{
    int vbo;

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, texture);

    vbo = __get_vbo_from_texture(texture);

    ren_vbom_draw_all(vbo);

    glBindTexture(GL_TEXTURE_2D, 0);

//    printf("drawing vbo:%d\n", vbo);
}

/**
 * Draw all objects in registry
 *
 * */
void ren_objs_draw(
)
{
#if 0
    for (iter = arraylistf_iter(__robj_list); tea_iter_hasNextDone(iter);)
    {
        ren_object_t *rob;

        rob = tea_iter_next(iter);
//        ren_obj_push(rob);
//        ren_obj_draw(rob);
    }
#endif

//    tea_iter_t *iter;

    if (__vbosByTexture)
    {
#if 0
        tea_iter_forall_udata(hashmap_iterKeys
                              (__vbosByTexture),
                              (void *) __draw_vbo_from_texture, NULL);
#endif
    }
}
