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
//#include <GL/gl.h>
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
    int size;

    size = sizeof(ren_vertex_tc_t) * nverts;

    /*  set context to this vbo's buffer */
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo);

    /*  memcpy to subdata of buffer */
    glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,     /*  buffer type */
                       item_idx * size, /*  memory location */
                       size,    /*  size of data */
                       verts);  /*  data */

    /*  reset current context */
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
}

void ren_vbo_draw(
    const int vbo,
    const int start,
    const int n_elems
)
{
    /* bind VBOs for ren_vertex_tc_t array and index array */
    /*  set context to this vbo's buffer */
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo);  // for vertex coordinates
//    glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo_colours);
//    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, vboId2); // for indices

    /* do same as vertex array except pointer */
    /*  activate vertex coords array */
    glEnableClientState(GL_VERTEX_ARRAY);
    /*  activate texture coords array */
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
//    glEnableClientState(GL_COLOR_ARRAY);

    /*  set pointer */
    glTexCoordPointer(2, GL_FLOAT, sizeof(ren_vertex_tc_t), BUFFER_OFFSET(12));
    glVertexPointer(3, GL_FLOAT, sizeof(ren_vertex_tc_t), 0);
//  glTexCoordPointer(2, GL_FLOAT, sizeof(ren_vertex_tc_t), BUFFER_OFFSET(12));

//    glColorPointer(4, GL_UNSIGNED_BYTE, 0, BUFFER_OFFSET(16));

#if 0
    glVertexAttribPointer(
        g_resources.attributes.position,  /* attribute */
        2,                                /* size */
        GL_FLOAT,                         /* type */
        GL_FALSE,                         /* normalized? */
        sizeof(GLfloat)*2,                /* stride */
        (void*)0                          /* array buffer offset */
    );
    glEnableVertexAttribArray(g_resources.attributes.position);
#endif

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
