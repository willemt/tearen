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

//#include "r_local.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "tea_vec.h"
#include "r_draw.h"
#include <GL/gl.h>
//#include "r_opengl.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

unsigned int ren_vbo_init(
    int n_elem
)
{
    ren_vertex_tc_t verts[n_elem];

    unsigned int size;

    GLuint vbo;

//    glBindBufferARB(GLenum target, GLuint id)
//    glBufferDataARB(GLenum target, GLsizei size, const void* data, GLenum usage)

    size = sizeof(ren_vertex_tc_t) * n_elem;
    memset(verts, 0, size);

    glGenBuffersARB(1, &vbo);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo);
    glBufferDataARB(GL_ARRAY_BUFFER_ARB, size, verts, GL_DYNAMIC_DRAW_ARB);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

    return vbo;
}

void ren_vbo_item_set_vertices(
    const int vbo,
    const int item_idx,
    const ren_vertex_tc_t * verts,
    const int nverts
)
{
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo);
    glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,
                       item_idx * sizeof(ren_vertex_tc_t) * nverts,
                       sizeof(ren_vertex_tc_t) * nverts, verts);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
}

void ren_vbo_draw(
    const int vbo,
    const int start,
    const int n_elems
)
{
    /* bind VBOs for ren_vertex_tc_t array and index array */
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo);  // for vertex coordinates
//    glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo_colours);
//    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, vboId2); // for indices

    /* do same as vertex array except pointer */
    glEnableClientState(GL_VERTEX_ARRAY);       // activate vertex coords array
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
//    glEnableClientState(GL_COLOR_ARRAY);

    glTexCoordPointer(2, GL_FLOAT, sizeof(ren_vertex_tc_t), BUFFER_OFFSET(12));
    glVertexPointer(3, GL_FLOAT, sizeof(ren_vertex_tc_t), 0);
//  glTexCoordPointer(2, GL_FLOAT, sizeof(ren_vertex_tc_t), BUFFER_OFFSET(12));

//    glColorPointer(4, GL_UNSIGNED_BYTE, 0, BUFFER_OFFSET(16));

    glDrawArrays(GL_QUADS, 4 * start, 4 * n_elems);

    /* draw 6 quads using offset of index array */
//    glDrawElements(GL_QUADS, 24, GL_UNSIGNED_BYTE, 0);

    glDisableClientState(GL_VERTEX_ARRAY);      // deactivate vertex array
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);       // deactivate vertex array
//    glDisableClientState(GL_COLOR_ARRAY);

    /* bind with 0, so, switch back to normal pointer operation */
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
//  glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
}

#if 0
void ren_vbo_item_set_vertices_and_texcoords(
    const int vbo,
    const int item_idx,
    vec2_t begin,
    vec2_t end,
    const int nverts
)
{
    vec3Set(verts[0].pos, rent->org[0], rent->org[1], 0);
    vec3Set(verts[1].pos, rent->org[0] + w, rent->org[1], 0);
    vec3Set(verts[2].pos, rent->org[0] + w, rent->org[1] + w, 0);
    vec3Set(verts[3].pos, rent->org[0], rent->org[1] + w, 0);

    __media_texturecoords_2_gltexturecoords(media_id, verts);

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo_verts);
    glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, elem, sizeof(ren_vertex_tc_t) * 4,
                       verts);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
}
#endif
