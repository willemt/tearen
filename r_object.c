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

typedef void (*gl_value_func) (GLuint shader, GLenum pname,
			       GLint * params);
typedef void (*gl_info_log) (GLuint shader, GLsizei maxLength,
			     GLsizei * length, GLchar * infoLog);


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
    vec2_t dst;
    int vbo_slot;
} __bone_t;

/**
 * Canvas object
 * This object's role is to draw other objects */
typedef struct {
    arraylistf_t *children;
} __canvas_t;

/*----------------------------------------------------------------------------*/

#define in(x) ((__obj_t*)x->in)
#define square(x) ((__square_t*)in(x)->typedata)
#define rect(x) ((__rect_t*)in(x)->typedata)
#define bone(x) ((__bone_t*)in(x)->typedata)
#define canvas(x) ((__canvas_t*)in(x)->typedata)

/** default size of each VBO */
#define VBO_ELEM_SIZE 1000


/** Internal list of objects */
static arraylistf_t *__robj_list = NULL;

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
    } attributes;

} __resources_t;

static __resources_t *resources = NULL;

/*----------------------------------------------------------------------------*/
/**
 * Set verts with the texture coordinates for this media_id */
static void
__media_texturecoords_2_gltexturecoords(const int media_id,
					ren_vertex_tc_t verts[4])
{
    vec2_t begin, end;

#if 1
    ren_media_get_texturecoords(media_id, begin, end);
    vec2Set(verts[0].tex, end[0], begin[1]);
    vec2Set(verts[1].tex, begin[0], begin[1]);
    vec2Set(verts[2].tex, begin[0], end[1]);
    vec2Set(verts[3].tex, end[0], end[1]);
#else
    /*
       vec2Set(verts[0].tex, 0, 0.1);
       vec2Set(verts[1].tex, 0.1, 0);
       vec2Set(verts[2].tex, 0.1, 0.1);
       vec2Set(verts[3].tex, 0, 0.1);
     */
    vec2Set(verts[0].tex, 0, 1);
    vec2Set(verts[1].tex, 1, 1);
    vec2Set(verts[2].tex, 1, 0);
    vec2Set(verts[3].tex, 0, 0);

    vec2Set(verts[0].tex, 0, 0);
    vec2Set(verts[1].tex, 0, 1);
    vec2Set(verts[2].tex, 1, 1);
    vec2Set(verts[3].tex, 1, 0);
#endif
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
//      vbo = ren_vbom_init(VBO_ELEM_SIZE);
	vbo = ren_vbosquare_init(VBO_ELEM_SIZE);
	hashmap_put(__vbosByTex, (void *) texture, (void *) vbo);
    }

    return vbo;
}

/*----------------------------------------------------------------------------*/

/**
 * @return VBO ID that object is on */
static int __ren_obj_get_vbo(ren_object_t * rob)
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

/*----------------------------------------------------------------------------*/

/**
 * Robject destructor */
int ren_obj_release(ren_object_t * rob)
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
	    ren_vbosquare_release_itemslot(__ren_obj_get_vbo(rob),
					   square(rob)->vbo_slot);
	    free(in(rob)->typedata);
	}
	break;
    case RENT_RECT:
	{
//            printf("releasing rect from vbo: %d\n", in(rob)->id);
	    ren_vbosquare_release_itemslot(__ren_obj_get_vbo(rob),
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
int ren_obj_release_uninitialise(ren_object_t ** rob)
{
    int ret;

    if (!*rob)
	return 0;

    ret = ren_obj_release(*rob);
    *rob = NULL;
    return ret;
}

/*----------------------------------------------------------------------------*/

static void __manage_vboslot_with_new_textures(
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
 * */
int ren_obj_set_media(ren_object_t * rob, const int media_id)
{
//    void *vbo;

    assert(rob);
    assert(rob->in);

    switch (in(rob)->type)
    {
    case RENT_SQUARE:
    case RENT_SQUARE_CENTER:
	{
	    __manage_vboslot_with_new_textures(&square(rob)->vbo_slot,
					       media_id,
					       square(rob)->media);
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
int ren_obj_get_media(ren_object_t * rob)
{
    assert(rob);
    assert(rob->in);
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
int ren_obj_set_rot_org(ren_object_t * rob, vec2_t rot_org)
{
    assert(rob);
    assert(rob->in);
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
int ren_obj_set_zrot(ren_object_t * rob, float zrot)
{
    assert(rob);
    assert(rob->in);
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
int ren_obj_get_w(ren_object_t * rob)
{
    assert(rob);
    assert(rob->in);
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
int ren_obj_get_h(ren_object_t * rob)
{
    assert(rob);
    assert(rob->in);
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
int ren_obj_set_org(ren_object_t * rob, vec2_t org)
{
    assert(rob);
    assert(rob->in);
    switch (in(rob)->type)
    {
    case RENT_SQUARE:
    case RENT_SQUARE_CENTER:
	{
	    int vbo, vbo_slot;

	    int media_id;

	    float w = (float) square(rob)->w;

	    vec2Copy(org, in(rob)->org);

	    media_id = square(rob)->media;
	    vbo = __ren_obj_get_vbo(rob);
	    vbo_slot = square(rob)->vbo_slot;

#if 0
	    printf("%f,%f %d %d vbo:%d vslot:%d w:%d\n", org[0], org[1],
		   media_id, ren_media_get_texture(media_id), vbo,
		   vbo_slot, w);
#endif
	    {
		ren_vertex_tc_t verts[4];

		vec2Set(verts[0].pos, org[0], org[1]);
		vec2Set(verts[1].pos, org[0] + w, org[1]);
		vec2Set(verts[2].pos, org[0] + w, org[1] + w);
		vec2Set(verts[3].pos, org[0], org[1] + w);
#if 0
		vec2Set(verts[0].tex, 1, 1);
		vec2Set(verts[1].tex, 0, 1);
		vec2Set(verts[2].tex, 0, 0);
		vec2Set(verts[3].tex, 1, 0);
#endif

		__media_texturecoords_2_gltexturecoords(media_id, verts);
		ren_vbosquare_item_set_vertices(vbo, vbo_slot, verts, 4);
	    }
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
ren_obj_set_org_from_xy(ren_object_t * rob, const float x, const float y)
{
    assert(rob);
    assert(rob->in);
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
int ren_obj_set_w(ren_object_t * rob, const int w)
{
    assert(rob);
    assert(rob->in);
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
int ren_obj_set_h(ren_object_t * rob, const int h)
{
    assert(rob);
    assert(rob->in);
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

/**
 * Set current object as parent */
void ren_obj_set_parent(ren_object_t * rob, ren_object_t * par)
{
    ren_obj_add_child(par, rob);
}

/*----------------------------------------------------------------------------*/

/**
 * Set destination org of end of bone */
void ren_bone_set_dst(ren_object_t * rob, vec2_t dst)
{

}

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
    ren_object_t *rob;

    /*  robject initialisations */
    rob = calloc(1, sizeof(ren_object_t));
    rob->in = calloc(1, sizeof(__obj_t));
    in(rob)->id = arraylistf_add(__robj_list, rob);
    in(rob)->type = type;

    /*  sub-class initialisations */
    switch (type)
    {
    case RENT_SQUARE:
    case RENT_SQUARE_CENTER:
	in(rob)->typedata = calloc(1, sizeof(__square_t));
	break;
    case RENT_CANVAS:
	in(rob)->typedata = calloc(1, sizeof(__canvas_t));
	canvas(rob)->children = arraylistf_new();
	break;
    default:
	assert(false);
	break;
    }

    return rob;
}

/**
 *
 * Note: For convenience mainly
 *
 */
int ren_obj_init_ptr(ren_object_t ** rob, const int type)
{
    if (NULL == *rob)
    {
	*rob = ren_obj_init(type);
	return 1;
    }

    return 0;
}

/*----------------------------------------------------------------------------*/
void ren_obj_add_child(ren_object_t * rob, ren_object_t * child)
{
    arraylistf_add(canvas(rob)->children, child);
}

void ren_obj_remove_child(ren_object_t * rob, ren_object_t * child)
{
    assert(false);
}

/*----------------------------------------------------------------------------*/

/**
 * Draw object
 *
 * */
int ren_obj_draw(ren_object_t * rob)
{
    assert(rob);

    switch (in(rob)->type)
    {
    case RENT_SQUARE:
	{
	    ren_mat4_t mat;
	    int tex;

            /* start using standard shader */
	    glUseProgram(resources->program);

            /* get the texture to use */
	    tex = ren_media_get_texture(square(rob)->media);

	    /* bind texture */
	    glActiveTexture(GL_TEXTURE0);
	    glBindTexture(GL_TEXTURE_2D, tex);
	    glUniform1i(resources->attributes.texture, 0);

            /* set up the screen view */
	    ren_mat4_projection(mat, 100.0, -1, 640.0, 0.0, 0.0, 480.0);
	    glUniformMatrix4fv(resources->attributes.pmatrix, 1, GL_FALSE,
			       mat);

            /* draw the object */
	    ren_vbosquare_draw_all(__ren_obj_get_vbo(rob),
				   resources->attributes.position,
				   resources->attributes.texcoord);

	    /* clean up */
	    glBindTexture(GL_TEXTURE_2D, 0);
	    glUseProgram(0);
	}
	break;
    case RENT_CANVAS:

	{
	    int ii;

            /* draw all children */
	    for (ii = 0; ii < arraylistf_count(canvas(rob)->children);
		 ii++)
	    {
		ren_object_t *child;

		child = arraylistf_get(canvas(rob)->children, ii);

		ren_obj_draw(child);
	    }

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

    /*  initialise robject list */
    if (!__robj_list)
    {
	__robj_list = arraylistf_new();
    }

    if (!resources)
    {
	resources = calloc(1, sizeof(__resources_t));
    }

    /* load standard resources */

    vertex_shader = ren_shader("defaultt.vert.glsl");
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
