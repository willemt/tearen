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
 * managed VBO
 *
 * */

#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include "tea_vec.h"
#include "SDL/SDL_opengl.h"
#include "r_draw.h"
#include "r_local.h"
//#include <GL/gl.h>


#include "heap.h"
#include "fixed_arraylist.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

typedef struct
{
    GLuint vbo;
    int n_elem;
    heap_t *heap;
    /*  how far we have gone down the array */
    /*  how many we have slots created */
    /*  creation is different from recycling */
    int n_length_inuse;
} vbo_t;


/*----------------------------------------------------------------------------*/

arraylistf_t *__vbos = NULL;

static vbo_t *__get_vbo_from_id(
    const int id
)
{
    return arraylistf_get(__vbos, id - 1);
}

static int __freeslot_compare(
    const void *e1,
    const void *e2,
    const void *udata
)
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
unsigned int ren_vbom_init(
    const int n_elem
)
{
    if (!__vbos)
    {
        __vbos = arraylistf_new();
    }

    vbo_t *vb;

    vb = calloc(1, sizeof(vbo_t));
    vb->n_elem = n_elem;
    vb->heap = heap_new(__freeslot_compare, NULL);
    /*  we are wrapping a VBO */
    vb->vbo = ren_vbo_init(n_elem);
    return arraylistf_add(__vbos, vb) + 1;
}

/** 
 * Find a slot that is free on this VBO
 * @return a new item slot on this VBO
 */
int ren_vbom_new_itemslot(
    const int id
)
{
    vbo_t *vb = __get_vbo_from_id(id);

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
void ren_vbom_item_set_vertices(
    const int id,
    const int item_idx,
    const ren_vertex_tc_t * verts,
    const int nverts
)
{
    vbo_t *vb = __get_vbo_from_id(id);

#if 0
    printf("VBO: %d set #%d vertices of %d\n", id, nverts, item_idx);
    int i;

    for (i = 0; i < nverts; i++)
        printf("%f,%f\n", verts[i].pos[0], verts[i].pos[1]);
#endif
    ren_vbo_item_set_vertices(vb->vbo, item_idx, verts, nverts);
}

static void __zero_item(
    const int id,
    const int slot_id
)
{
    ren_vertex_tc_t verts[4];

    memset(verts, 0, sizeof(ren_vertex_tc_t) * 4);
    ren_vbom_item_set_vertices(id, slot_id, verts, 4);
}

void ren_vbom_release_itemslot(
    const int id,
    const int slot_id
)
{
    vbo_t *vb = __get_vbo_from_id(id);

    __zero_item(id, slot_id);

    if (slot_id == vb->n_length_inuse)
    {
        vb->n_length_inuse--;
    }
    else
    {
        heap_offer(vb->heap, (void *) (unsigned long) slot_id);
    }
}

void ren_vbom_draw(
    const int id,
    const int start,
    const int n_elems
)
{
    vbo_t *vb = __get_vbo_from_id(id);

//    printf("drawing vbo: %d %d, %d %d\n", id, start, n_elems, sizeof(float));

    ren_vbo_draw(vb->vbo, start, n_elems);
}

void ren_vbom_draw_all(
    const int id
)
{
    vbo_t *vb = __get_vbo_from_id(id);

//    printf("drawing %d vbo objects\n", vb->n_elem);
//    printf("drawing: %d\n", id);

    ren_vbom_draw(id, 0, vb->n_elem);
}
