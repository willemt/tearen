/*
 * =====================================================================================
 *
 *       Filename:  r_vbo.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  04/03/11 20:37:02
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *        Company:  
 *
 * =====================================================================================
 */

/* 
 * managed VBO
 *
 * */

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "tea_vec.h"
#include "r_draw.h"
#include <GL/gl.h>


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



/*----------------------------------------------------------------------------*/
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
    vb->heap = heap_new(__freeslot_compare, NULL);      //teaObjUlong);
    vb->vbo = ren_vbo_init(n_elem);

    return arraylistf_add(__vbos, vb) + 1;
}


/* 
 * @return a new item slot on this VBO
 */
int ren_vbom_new_itemslot(
    const int id
)
{
    vbo_t *vb = __get_vbo_from_id(id);

    if (heap_isEmpty(vb->heap))
    {
        assert(vb->n_length_inuse < vb->n_elem);
        return vb->n_length_inuse++;
    }
    else
    {
        unsigned long slot = (unsigned long) heap_poll(vb->heap);

        return slot;
    }
}

/*----------------------------------------------------------------------------*/
void ren_vbom_item_set_vertices(
    const int id,
    const int item_idx,
    const ren_vertex_tc_t * verts,
    const int nverts
)
{
    vbo_t *vb = __get_vbo_from_id(id);

//    printf("VBO: %d set vertices of %d\n", id, item_idx);
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

    ren_vbom_draw(id, 0, vb->n_elem);
}
