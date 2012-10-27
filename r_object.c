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
 * 
 * multiple VBOs
 *  put verts in VBO according to GLTexture
 *  1 VBO for each GLTexture
 *
 *
 *
 */

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include "tea_vec.h"
#include <GL/gl.h>
#include "r_draw.h"
#include "r_local.h"
//#include "SDL/SDL_opengl.h"
//#include <opengl/OpenGL.h>
#include "linked_list_hashmap.h"
#include "fixed_arraylist.h"
#include "heap.h"
#include "matrix.h"

typedef void (*gl_value_func) (GLuint shader, GLenum pname,
			       GLint * params);
typedef void (*gl_info_log) (GLuint shader, GLsizei maxLength,
			     GLsizei * length, GLchar * infoLog);

static int __obj_get_vbo(ren_object_t * ro);

/*----------------------------------------------------------------------------*/
/**
 * generic type structure for render objects.
 * All objects sub-class this struct via typedata. */
typedef struct {
    vec2_t org;
    /*  subclass type */
    int type;
    int id;
    void *typedata;
} __obj_t;

/**
 * Square object */
typedef struct {
    int media;
    int w;
    vec2_t rot_org;
    float zrot;
    int vbo_slot;
} __square_t;

typedef struct {
    int media;
    int w, h;
    int vbo_slot;
} __rect_t;

typedef struct {
    int media;
    int w, h;
    /*  other part of bone */
    vec2_t endpos;
    int vbo_slot;
} __bone_t;

/**
 * Canvas object
 * This object's role is to draw other objects */
typedef struct {
    arraylistf_t *children;
    /* Use this heap for ordering the way we draw ros
       It isn't critical that the ordering of ros is perfect.  */
    heap_t *draw_heap;
    /* use this heap for offloading the ros that we draw */
    heap_t *offload_heap;

    /* dictionary of VBO IDs to squarevbo ros */
    hashmap_t *vbo_map;
} __canvas_t;

/**
 * Square vbo object
 * This object's role is to draw squares */
typedef struct {
    int media;
    int vbo;
} __square_vbo_t;

/*----------------------------------------------------------------------------*/

/**
 * Current state of the renderer */
typedef struct {

    // TODO

} r_object_render_state_t;

/*----------------------------------------------------------------------------*/

#define in(x) ((__obj_t*)x->in)
#define square(x) ((__square_t*)in(x)->typedata)
#define rect(x) ((__rect_t*)in(x)->typedata)
#define bone(x) ((__bone_t*)in(x)->typedata)
#define canvas(x) ((__canvas_t*)in(x)->typedata)
#define squarevbo(x) ((__square_vbo_t*)in(x)->typedata)

/** default size of each VBO */
#define VBO_ELEM_SIZE 1000

/** Internal list of objects */
static arraylistf_t *__roj_list = NULL;

/** Internal list of VBOs by texture
 * This is where we want to batch objects by their texture ID
 */
static hashmap_t *__vbosByTex = NULL;

typedef struct {
    GLuint vertex_shader, program;

    struct {
	GLint texcoord;
	GLint position;
	GLint texture;
	GLint pmatrix;
	GLint rotation;
    } attributes;

} __resources_t;

static __resources_t *resources = NULL;

/*----------------------------------------------------------------------------*/

static unsigned long __ulong_hash(const void *e1)
{
    const long i1 = (unsigned long) e1;

    assert(i1 >= 0);
    return i1;
}

static long __ulong_compare(const void *e1, const void *e2)
{
    const long i1 = (unsigned long) e1, i2 = (unsigned long) e2;

//      return !(*i1 == *i2); 
    return i1 - i2;
}

/**
 * Sort ros by the most effective drawing path */
static int __cmp_ros_to_draw(const void *o1,
			      const void *o2, const void *udata)
{
    const ren_object_t *r1 = o1, *r2 = o2;

    int t1, t2;

    t1 = ren_media_get_texture(ren_obj_get_media(r1));
    t2 = ren_media_get_texture(ren_obj_get_media(r2));

    /* compare textures */
    if (0 == t1 - t2)
    {
	/* equality testing:
	   compare against IDs */
	return in(r1)->id - in(r2)->id;
    }

    return t1 - t2;
}

/**
 * Set verts with the texture coordinates for this media_id */
static void
__media_texturecoords_2_gltexturecoords(const int media_id,
					ren_vertex_tc_t verts[4])
{
    vec2_t begin, end;

    ren_media_get_texturecoords(media_id, begin, end);
    vec2Set(verts[0].tex, end[0], begin[1]);
    vec2Set(verts[1].tex, begin[0], begin[1]);
    vec2Set(verts[2].tex, begin[0], end[1]);
    vec2Set(verts[3].tex, end[0], end[1]);
}

/*----------------------------------------------------------------------------*/

static void __obj_release_typedata(ren_object_t * ro)
{
    switch (in(ro)->type)
    {
    case RENT_SQUARE:
    case RENT_SQUARE_CENTER:
	{
	    ren_vbosquare_release_itemslot(__obj_get_vbo(ro),
					   square(ro)->vbo_slot);
	    free(in(ro)->typedata);
	}
	break;
    case RENT_RECT:
	{
	    ren_vbosquare_release_itemslot(__obj_get_vbo(ro),
					   rect(ro)->vbo_slot);
	    free(in(ro)->typedata);
	}
	break;
    default:
	assert(false);
	break;
    }

    in(ro)->typedata = NULL;
}

static void __obj_init_typedata(ren_object_t * ro)
{
    /*  sub-class initialisations */
    switch (in(ro)->type)
    {
    case RENT_SQUARE:
    case RENT_SQUARE_CENTER:
	in(ro)->typedata = calloc(1, sizeof(__square_t));
	break;
    case RENT_CANVAS:
	in(ro)->typedata = calloc(1, sizeof(__canvas_t));
	canvas(ro)->children = arraylistf_new();
	canvas(ro)->draw_heap = heap_new(__cmp_ros_to_draw, NULL);
	canvas(ro)->offload_heap = heap_new(__cmp_ros_to_draw, NULL);
	canvas(ro)->vbo_map = hashmap_new(__ulong_hash, __ulong_compare);
	break;
    case RENT_SQUARE_VBO:
	in(ro)->typedata = calloc(1, sizeof(__square_vbo_t));
	break;
    case RENT_BONE:
	in(ro)->typedata = calloc(1, sizeof(__bone_t));
	break;
    default:
	assert(false);
	break;
    }
}

/*----------------------------------------------------------------------------*/

/**
 * Get the VBO that corresponds to this texture.
 * Initialise a VBO if needed.
 * @return 0 = error; otherwise VBO ID
 */
static int __get_vbo_from_texture(unsigned long texture)
{
    unsigned long vbo;

    assert(0 < texture);

    /* look for the vbo */
    vbo =
	(unsigned long) (void *) hashmap_get(__vbosByTex,
					     (void *) texture);

    /* if there isn't a vbo for this texture, then create one */
    if (0 == vbo)
    {
	vbo = ren_vbosquare_init(VBO_ELEM_SIZE);
	hashmap_put(__vbosByTex, (void *) texture, (void *) vbo);
    }

    return vbo;
}

/*----------------------------------------------------------------------------*/

/**
 * @return VBO ID that object is on */
static int __obj_get_vbo(ren_object_t * ro)
{
    int tex, vbo;

    switch (in(ro)->type)
    {
    case RENT_SQUARE:
    case RENT_SQUARE_CENTER:
    case RENT_BONE:
	{
	    int tex;

	    tex = ren_media_get_texture(ren_obj_get_media(ro));

	    return __get_vbo_from_texture(tex);
	}
	break;
    default:
	assert(false);
	break;
    }

    return 0;
}

/*----------------------------------------------------------------------------*/


/**
 * roject destructor */
int ren_obj_release(ren_object_t * ro)
{
    void *removed;

    removed = arraylistf_remove(__roj_list, in(ro)->id);

    assert(ro == removed);

    __obj_release_typedata(ro);

    free(ro->in);
    free(ro);
    return 1;
}

/**
 * Release object and set ptr to NULL */
int ren_obj_release_uninitialise(ren_object_t ** ro)
{
    int ret;

    if (!*ro)
	return 0;

    ret = ren_obj_release(*ro);
    *ro = NULL;
    return ret;
}

/*----------------------------------------------------------------------------*/

static void __vboslot_with_new_texture(
					  /*  vbo item slot */
					  int *vslot,
					  /*  media new */
					  const int m_new,
					  /*  media old */
					  const int m_old)
{
    int tex, vbo;

    tex = ren_media_get_texture(m_new);
    vbo = __get_vbo_from_texture(tex);

    assert(vbo != 0);

    /*  if old texture is un-initialised, just create new itemslot from vbo */
    if (0 == m_old)
    {
	*vslot = ren_vbosquare_new_itemslot(vbo);
    }
    /*  otherwise, is the texture different? ie. do we need to change VBOs? */
    else
    {
	int old_tex;

	old_tex = ren_media_get_texture(m_old);

	/* yes there was a VBO change */
	if (tex != old_tex)
	{
	    int old_vbo;

	    old_vbo = __get_vbo_from_texture(old_tex);
	    ren_vbosquare_release_itemslot(old_vbo, *vslot);
	    *vslot = ren_vbosquare_new_itemslot(vbo);
	}
    }
}

/*----------------------------------------------------------------------------*/

/**
 * Set the current media ID of this object.
 * Used for texturing the object
 * @param m_id media id to use
 * */
int ren_obj_set_media(ren_object_t * ro, const int m_id)
{
//    void *vbo;

    assert(ro);
    assert(ro->in);

    switch (in(ro)->type)
    {
    case RENT_SQUARE:
    case RENT_SQUARE_CENTER:
	{
	    __vboslot_with_new_texture(&square(ro)->vbo_slot,
				       m_id, square(ro)->media);
	    square(ro)->media = m_id;
	}
	break;
    case RENT_RECT:
	__vboslot_with_new_texture(&rect(ro)->vbo_slot,
				   m_id, rect(ro)->media);
	rect(ro)->media = m_id;
	break;
    case RENT_BONE:
	__vboslot_with_new_texture(&bone(ro)->vbo_slot,
				   m_id, rect(ro)->media);
	bone(ro)->media = m_id;
	break;
    default:
	assert(false);
	break;
    }

    return 1;
}

/**
 * @return media id of object */
int ren_obj_get_media(ren_object_t * ro)
{
    assert(ro);
    assert(ro->in);
    switch (in(ro)->type)
    {
    case RENT_SQUARE:
    case RENT_SQUARE_CENTER:
	return square(ro)->media;
    case RENT_RECT:
	return rect(ro)->media;
    case RENT_SQUARE_VBO:
	return rect(ro)->media;
    case RENT_BONE:
	return bone(ro)->media;
    default:
	assert(false);
	return 0;
	break;
    }
}

/** Set rotational origin of object */
int ren_obj_set_rot_org(ren_object_t * ro, vec2_t rot_org)
{
    assert(ro);
    assert(ro->in);
    switch (in(ro)->type)
    {
    case RENT_SQUARE:
	vec2Copy(rot_org, square(ro)->rot_org);
#if 0
    case RENT_RECT:
	vec2Copy(rot_org, rect(ro)->rot_org);
	break;
#endif
    default:
	assert(false);
	break;
    }

    return 0;
}


/** Set Z rotation angle of object */
int ren_obj_set_zrot(ren_object_t * ro, float zrot)
{
    assert(ro);
    assert(ro->in);
    switch (in(ro)->type)
    {
    case RENT_SQUARE:
    case RENT_SQUARE_CENTER:
	{
	    square(ro)->zrot = zrot;
	    float w = (float) square(ro)->w;

	    float r[2];

	    ren_mat4_t mat;
	    ren_vertex_tc_t verts[4];

	    mat4_rotateZ(zrot, mat);

	    vec2Set(r, -w / 2, -w / 2);
	    mat4_multiply_vec(mat, r);
	    vec2Add(in(ro)->org, r, verts[0].pos);

	    vec2Set(r, -w / 2, w / 2);
	    mat4_multiply_vec(mat, r);
	    vec2Add(in(ro)->org, r, verts[1].pos);

	    vec2Set(r, w / 2, w / 2);
	    mat4_multiply_vec(mat, r);
	    vec2Add(in(ro)->org, r, verts[2].pos);

	    vec2Set(r, w / 2, -w / 2);
	    mat4_multiply_vec(mat, r);
	    vec2Add(in(ro)->org, r, verts[3].pos);

	    /* set texture coordinates */
	    __media_texturecoords_2_gltexturecoords(square(ro)->media,
						    verts);
	    /* commit changes to vbo */
	    ren_vbosquare_item_set_vertices(__obj_get_vbo(ro),
					    square(ro)->vbo_slot, verts,
					    4);
	}
	break;
    case RENT_BONE:
	break;
    default:
	assert(false);
	break;
    }

    return 1;
}

/**
 * @return width of object */
int ren_obj_get_w(ren_object_t * ro)
{
    assert(ro);
    assert(ro->in);
    switch (in(ro)->type)
    {
    case RENT_SQUARE:
    case RENT_SQUARE_CENTER:
	return square(ro)->w;
    case RENT_RECT:
	return rect(ro)->w;
    default:
	assert(false);
	break;
    }

    return 0;
}

/**
 * @return height of object */
int ren_obj_get_h(ren_object_t * ro)
{
    assert(ro);
    assert(ro->in);
    switch (in(ro)->type)
    {
    case RENT_SQUARE:
    case RENT_SQUARE_CENTER:
	return square(ro)->w;
    case RENT_RECT:
	return rect(ro)->h;
    default:
	assert(false);
	break;
    }

    return 0;
}

static void __boneify_verts(const vec2_t a,
			    const vec2_t b, ren_vertex_tc_t * o, float w)
{
    vec2_t normal;

    vec2Subtract(a, b, normal);
    vec2Normalize(normal);
    vec2Perp(normal);
    vec2MA(a, -w, normal, o[0].pos);
    vec2MA(a, w, normal, o[1].pos);
    vec2MA(b, w, normal, o[2].pos);
    vec2MA(b, -w, normal, o[3].pos);
}

/**
 * set origin of object
 *
 * */
int ren_obj_set_org(ren_object_t * ro, vec2_t org)
{
    assert(ro);
    assert(ro->in);
    switch (in(ro)->type)
    {
    case RENT_SQUARE:
    case RENT_SQUARE_CENTER:
	{
	    int vbo, vbo_slot, media_id;

	    float w = (float) square(ro)->w;

	    vec2Copy(org, in(ro)->org);

#if 0
	    printf("%f,%f %d %d vbo:%d vslot:%d w:%d\n", org[0], org[1],
		   media_id, ren_media_get_texture(media_id), vbo,
		   vbo_slot, w);
#endif
	    ren_vertex_tc_t verts[4];

	    vec2Set(verts[0].pos, org[0], org[1]);
	    vec2Set(verts[1].pos, org[0] + w, org[1]);
	    vec2Set(verts[2].pos, org[0] + w, org[1] + w);
	    vec2Set(verts[3].pos, org[0], org[1] + w);

	    /* set texture coordinates */
	    __media_texturecoords_2_gltexturecoords(square(ro)->media,
						    verts);
	    /* commit changes to vbo */
	    ren_vbosquare_item_set_vertices(__obj_get_vbo(ro),
					    square(ro)->vbo_slot, verts,
					    4);
	}
	break;
    case RENT_BONE:
	{
	    ren_vertex_tc_t verts[4];

	    vec2Copy(org, in(ro)->org);

	    /* make it like a bone */
	    __boneify_verts(in(ro)->org, bone(ro)->endpos, verts,
			    bone(ro)->w);

	    /* set texture coordinates */
	    __media_texturecoords_2_gltexturecoords(bone(ro)->media,
						    verts);
	    /* commit changes to vbo */
	    ren_vbosquare_item_set_vertices(__obj_get_vbo(ro),
					    bone(ro)->vbo_slot, verts, 4);
	}
	break;
    default:
	assert(false);
	break;
    }

    return 0;
}

/**
 * set end point of object
 *
 * */
int ren_obj_set_endpos(ren_object_t * ro, vec2_t endpos)
{
    assert(ro);
    assert(ro->in);
    switch (in(ro)->type)
    {
    case RENT_SQUARE:
    case RENT_SQUARE_CENTER:
	{
	    int vbo, vbo_slot, media_id;
	    vec2_t org;

	    float w;

	    /* archive data */
	    w = (float) square(ro)->w;
	    media_id = square(ro)->media;
	    vec2Copy(in(ro)->org, org);

	    /* convert into bone object */
	    /* free typedata - this is because we are re-allocating */
	    __obj_release_typedata(ro);
	    __obj_init_typedata(ro);
	    in(ro)->type = RENT_BONE;

	    /* use archive data */
	    ren_obj_set_w(ro, w);
	    ren_obj_set_media(ro, media_id);

	    /* re-position */
	    ren_obj_set_org(ro, org);
	    ren_obj_set_endpos(ro, endpos);
	}
	break;

    case RENT_BONE:
	{
	    ren_vertex_tc_t verts[4];

	    vec2Copy(endpos, bone(ro)->endpos);

	    /* make it like a bone */
	    __boneify_verts(in(ro)->org, bone(ro)->endpos, verts,
			    bone(ro)->w);

	    /* set texture coordinates */
	    __media_texturecoords_2_gltexturecoords(bone(ro)->media,
						    verts);
	    /* commit changes to vbo */
	    ren_vbosquare_item_set_vertices(__obj_get_vbo(ro),
					    bone(ro)->vbo_slot, verts, 4);
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
int
ren_obj_set_org_from_xy(ren_object_t * ro, const float x, const float y)
{
    assert(ro);
    assert(ro->in);
    switch (in(ro)->type)
    {
    case RENT_SQUARE:
    case RENT_SQUARE_CENTER:
    case RENT_RECT:
	{
	    vec2_t org;

	    vec2Set(org, x, y);
	    ren_obj_set_org(ro, org);
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
int ren_obj_set_w(ren_object_t * ro, const int w)
{
    assert(ro);
    assert(ro->in);
    switch (in(ro)->type)
    {
    case RENT_SQUARE_CENTER:
	vec2Set(square(ro)->rot_org, (float) w / 2, (float) w / 2);
    case RENT_SQUARE:
	square(ro)->w = w;
	break;
    case RENT_RECT:
	rect(ro)->w = w;
	break;
    case RENT_BONE:
	bone(ro)->w = w;
	break;
    default:
	assert(false);
	break;
    }

    return 1;
}

/**
 * Set height of object */
int ren_obj_set_h(ren_object_t * ro, const int h)
{
    assert(ro);
    assert(ro->in);
    switch (in(ro)->type)
    {
    case RENT_SQUARE_CENTER:
	vec2Set(square(ro)->rot_org, (float) h / 2, (float) h / 2);
    case RENT_SQUARE:
	square(ro)->w = h;
	break;
    case RENT_RECT:
	rect(ro)->h = h;
	break;
    default:
	assert(false);
	break;
    }

    return 1;
}

/**
 * Set current object as parent */
void ren_obj_set_parent(ren_object_t * ro, ren_object_t * par)
{
    ren_obj_add_child(par, ro);
}

/*----------------------------------------------------------------------------*/

/**
 * Set destination org of end of bone */
void ren_bone_set_dst(ren_object_t * ro, vec2_t dst)
{

}

/*----------------------------------------------------------------------------*/
#if 0
static void
show_info_log(GLuint object,
	      gl_value_func glGet__iv,
	      PFNGLGETSHADERINFOLOGPROC glGet__InfoLog)
{
    GLint log_length;
    char *log;

    glGet__iv(object, GL_INFO_LOG_LENGTH, &log_length);
    log = malloc(log_length);
    glGetProgramInfoLog(object, log_length, NULL, log);
    fprintf(stderr, "%s", log);
    free(log);
}
#endif



/**
 * Object constructor
 *
 * @return new renderable object
 * */
ren_object_t *ren_obj_init(const int type)
{
    ren_object_t *ro;

    /*  roject initialisations */
    ro = calloc(1, sizeof(ren_object_t));
    ro->in = calloc(1, sizeof(__obj_t));
    /* add to internal registry */
    in(ro)->id = arraylistf_add(__roj_list, ro);
    /* set type and load typedata */
    in(ro)->type = type;
    __obj_init_typedata(ro);

    return ro;
}

/**
 *
 * Note: For convenience mainly
 *
 */
int ren_obj_init_ptr(ren_object_t ** ro, const int type)
{
    if (NULL == *ro)
    {
	*ro = ren_obj_init(type);
	return 1;
    }

    return 0;
}

/*----------------------------------------------------------------------------*/
void ren_obj_add_child(ren_object_t * ro, ren_object_t * child)
{
    switch (in(ro)->type)
    {
    case RENT_CANVAS:
	arraylistf_add(canvas(ro)->children, child);

	if (in(child)->type == RENT_SQUARE)
	{
	    long child_vbo;
	    ren_object_t *new;

	    /* get child ro's vbo */
	    child_vbo =
		__get_vbo_from_texture(ren_media_get_texture
				       (square(child)->media));

	    /* check if we already have a ro for this vbo */
	    if (hashmap_get(canvas(ro)->vbo_map, (void *) child_vbo))
	    {
		return;
	    }

	    /* create a ro for this vbo */
	    new = ren_obj_init(RENT_SQUARE_VBO);
	    squarevbo(new)->media = ren_obj_get_media(child);
	    squarevbo(new)->vbo = child_vbo;
	    heap_offer(canvas(ro)->draw_heap, new);
	    hashmap_put(canvas(ro)->vbo_map, (void *) child_vbo, new);
	}
	break;
    default:
	assert(false);
    }

}

void ren_obj_remove_child(ren_object_t * ro, ren_object_t * child)
{
    assert(false);
}

/*----------------------------------------------------------------------------*/

/**
 * Draw object
 *
 * */
int ren_obj_draw(ren_object_t * ro)
{
    assert(ro);

    switch (in(ro)->type)
    {
    case RENT_SQUARE:
	{
	}
	break;
    case RENT_SQUARE_VBO:
	{
	    ren_mat4_t mat;
	    int tex;

	    /* start using standard shader */
	    glUseProgram(resources->program);

	    /* get the texture to use */
	    tex = ren_media_get_texture(squarevbo(ro)->media);

	    /* bind texture */
	    glActiveTexture(GL_TEXTURE0);
	    glBindTexture(GL_TEXTURE_2D, tex);
	    glUniform1i(resources->attributes.texture, 0);

	    /* set up the screen view */
	    ren_mat4_projection(mat, 100.0, -1, 960.0, 0.0, 0.0, 640.0);
	    glUniformMatrix4fv(resources->attributes.pmatrix, 1, GL_FALSE,
			       mat);

	    /* draw the object */
	    ren_vbosquare_draw_all(__get_vbo_from_texture(tex),
				   resources->attributes.position,
				   resources->attributes.texcoord);

	    /* clean up */
	    glBindTexture(GL_TEXTURE_2D, 0);
	    glUseProgram(0);
	}
	break;
    case RENT_CANVAS:

	{
#if 0
	    int ii;

	    /* draw all children */
	    for (ii = 0; ii < arraylistf_count(canvas(ro)->children);
		 ii++)
	    {
		child = arraylistf_get(canvas(ro)->children, ii);
	    }
#endif

	    do
	    {
		ren_object_t *child;

		/* get the next best object to draw */
		child = heap_poll(canvas(ro)->draw_heap);

		if (!child)
		    break;

		/* offload to secondary heap */
		heap_offer(canvas(ro)->offload_heap, child);

		/* draw the object */
		ren_obj_draw(child);
	    }
	    while (1);

	    void *heap_swapped;

	    /* swap heaps so that we don't have to copy memory */
	    heap_swapped = canvas(ro)->offload_heap;
	    canvas(ro)->offload_heap = canvas(ro)->draw_heap;
	    canvas(ro)->draw_heap = heap_swapped;
	}

	break;
    default:
	assert(0);
    }

    return 1;
}

void ren_objs_init()
{
    GLuint vertex_shader, fragment_shader;

    /*  initialise vbo -> texture hashmap */
    if (!__vbosByTex)
    {
	__vbosByTex = hashmap_new(__ulong_hash, __ulong_compare);
    }

    /*  initialise roject list */
    if (!__roj_list)
    {
	__roj_list = arraylistf_new();
    }

    if (!resources)
    {
	resources = calloc(1, sizeof(__resources_t));
    }

    /* load standard resources */

    vertex_shader = ren_shader("default.vert.glsl");
    fragment_shader = ren_shader("default.frag.glsl");
    resources->program =
	ren_shader_program(vertex_shader, fragment_shader);

    resources->attributes.position =
	glGetAttribLocation(resources->program, "position");

    if (resources->attributes.position == -1)
    {
	assert(0);
    }

    resources->attributes.texcoord =
	glGetAttribLocation(resources->program, "texcoord");

    if (resources->attributes.texcoord == -1)
    {
	assert(0);
    }

    resources->attributes.texture =
	glGetUniformLocation(resources->program, "texture");

    if (resources->attributes.texture == -1)
    {
	assert(0);
    }

    resources->attributes.pmatrix =
	glGetUniformLocation(resources->program, "pmatrix");

    if (resources->attributes.pmatrix == -1)
    {
	assert(0);
    }
}
