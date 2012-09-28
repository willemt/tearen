
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

#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include "tea_vec.h"
#include "SDL/SDL_opengl.h"
#include "r_draw.h"
#include "r_local.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

#include "heap.h"
#include "fixed_arraylist.h"


static arraylistf_t *__vbos = NULL;

typedef struct {
    int n_elem;
    heap_t *heap;
    /*  how far we have gone down the array */
    /*  how many we have slots created */
    /*  creation is different from recycling */
    int n_length_inuse;

    GLuint vtxcoord, vtxpos;
} vbo_t;

/*----------------------------------------------------------------------------*/

static vbo_t *__get_vbo_from_id(const int id)
{
    return arraylistf_get(__vbos, id - 1);
}

static int __freeslot_compare(const void *e1,
			      const void *e2, const void *udata)
{
    const unsigned long t1 = (unsigned long) e1, t2 = (unsigned long) e2;

    unsigned long ret = t1 - t2;

    return ret;
}

/*----------------------------------------------------------------------------*/
/**
 * Create a managed VBO.
 * @param n_elem number of elements in new VBO
 * @return VBO ID*/
unsigned int ren_vbosquare_init(const int n_elem)
{
    if (!__vbos)
    {
	__vbos = arraylistf_new();
    }

    vbo_t *vb;

    /* create vbo */
    vb = calloc(1, sizeof(vbo_t));
    vb->n_elem = n_elem;
    vb->heap = heap_new(__freeslot_compare, NULL);

    ren_vertex_tc_t verts[n_elem];
    ren_vertex_position_t verts_pos[n_elem];

    unsigned int size;

    /* vertex coords */
    size = sizeof(ren_vertex_tc_t) * n_elem;
    memset(verts, 0, size);
    glGenBuffers(1, &vb->vtxcoord);
    glBindBuffer(GL_ARRAY_BUFFER_ARB, vb->vtxcoord);
    glBufferData(GL_ARRAY_BUFFER_ARB, size, verts, GL_DYNAMIC_DRAW);	//GL_STATIC_DRAW);

#if 0
    /* position vertex attribute */
    size = sizeof(ren_vertex_position_t) * n_elem;
    memset(verts_pos, 0, size);
    glGenBuffers(1, &vb->vtxpos);
    glBindBuffer(GL_ARRAY_BUFFER, vb->vtxpos);
    glBufferData(GL_ARRAY_BUFFER, size, verts_pos, GL_DYNAMIC_DRAW);
    #endif

    /* clean up */
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

    return arraylistf_add(__vbos, vb) + 1;
}

/** 
 * Find a slot that is free on this VBO
 * @return a new item slot on this VBO
 */
int ren_vbosquare_new_itemslot(const int id)
{
    vbo_t *vb = __get_vbo_from_id(id);

    /* if the heap is empty just use the most latest slot */
    if (0 == heap_count(vb->heap))
    {
	assert(vb->n_length_inuse < vb->n_elem);
	return vb->n_length_inuse++;
    }
    else
    {
	unsigned long slot;

	slot = (unsigned long) heap_poll(vb->heap);

	return slot;
    }
}

/*----------------------------------------------------------------------------*/
/**
 * Set data elements of an item.
 */
void ren_vbosquare_item_set_vertices(const int id,
				     const int item_idx,
				     const ren_vertex_tc_t * verts,
				     const int nverts)
{
    vbo_t *vb = __get_vbo_from_id(id);

    int size;

    size = sizeof(ren_vertex_tc_t) * nverts;

    /*  set context to this vbo's buffer */
    glBindBufferARB(GL_ARRAY_BUFFER, vb->vtxcoord);

    /*  memcpy to subdata of buffer */
    glBufferSubDataARB(GL_ARRAY_BUFFER,	/*  buffer type */
		       item_idx * size,	/*  memory location */
		       size,	/*  size of data */
		       verts);	/*  data */
/* clean up */
    glBindBufferARB(GL_ARRAY_BUFFER, 0);
}

void ren_vbosquare_item_set_vertex_position(const int id,
					    const int item_idx,
					    const ren_vertex_position_t *
					    data_in, const int nverts)
{
    int size;
    vbo_t *vb;

    vb = __get_vbo_from_id(id);
    size = sizeof(ren_vertex_position_t) * nverts;

    /*  set context to this vbo's buffer */
    glBindBufferARB(GL_ARRAY_BUFFER, vb->vtxpos);

    /*  memcpy to subdata of buffer */
    glBufferSubDataARB(GL_ARRAY_BUFFER,	/*  buffer type */
		       item_idx * size,	/*  memory location */
		       size,	/*  size of data */
		       data_in);	/*  data */

    /* clean up */
    glBindBufferARB(GL_ARRAY_BUFFER, 0);
}

static void __zero_item(const int id, const int slot_id)
{
    ren_vertex_tc_t verts[4];

    memset(verts, 0, sizeof(ren_vertex_tc_t) * 4);
    ren_vbosquare_item_set_vertices(id, slot_id, verts, 4);
}

void ren_vbosquare_release_itemslot(const int id, const int slot_id)
{
    vbo_t *vb = __get_vbo_from_id(id);

    __zero_item(id, slot_id);

    /* decrease size of array */
    if (slot_id == vb->n_length_inuse)
    {
	vb->n_length_inuse--;
    }
    else
    {
	heap_offer(vb->heap, (void *) (unsigned long) slot_id);
    }
}

void ren_vbosquare_draw(const int id, const int start, const int n_elems,
			int attr_position, int attr_texcoord)
{
    vbo_t *vb = __get_vbo_from_id(id);


#if 0
    glBindBufferARB(GL_ARRAY_BUFFER, vb->vtxpos);
    glVertexAttribPointer(attr_position,	/* attribute */
			  2,	/* size */
			  GL_FLOAT,	/* type */
			  GL_FALSE,	/* normalized? */
			  sizeof(GLfloat) * 2,	/* stride */
			  (void *) 0	/* array buffer offset */
	);
    glEnableVertexAttribArray(attr_position);
#endif

    glBindBufferARB(GL_ARRAY_BUFFER, vb->vtxcoord);

#if 0
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    /*  set pointer */
    glTexCoordPointer(2, GL_FLOAT, sizeof(ren_vertex_tc_t),
		      BUFFER_OFFSET(12));
    glVertexPointer(3, GL_FLOAT, sizeof(ren_vertex_tc_t), 0);
#endif


    /* vertex */
    glVertexAttribPointer(attr_position, 2, GL_FLOAT, GL_FALSE,
			  sizeof(ren_vertex_tc_t),
			  (void *) offsetof(ren_vertex_tc_t, pos));
    glEnableVertexAttribArray(attr_position);

    /* texcoord */
    glVertexAttribPointer(attr_texcoord, 2, GL_FLOAT, GL_FALSE,
			  sizeof(ren_vertex_tc_t),
			  (void *) offsetof(ren_vertex_tc_t, tex));
    glEnableVertexAttribArray(attr_texcoord);

    glDrawArrays(GL_QUADS, 4 * start, 4 * n_elems);

    /* clean up */
    glDisableVertexAttribArray(attr_position);
    glDisableVertexAttribArray(attr_texcoord);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
}

void ren_vbosquare_draw_all(const int id, int attr_position,
			    int attr_texcoord)
{
    vbo_t *vb = __get_vbo_from_id(id);

    ren_vbosquare_draw(id, 0, vb->n_elem, attr_position, attr_texcoord);
}

void ren_vbosquares_init()
{
    /* create attribute vertex shader */

}
