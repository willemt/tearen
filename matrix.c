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
#include <stdbool.h>
#include <assert.h>
#include "tea_vec.h"
#include <GL/gl.h>
#include "r_draw.h"
#include "r_local.h"
#include "linked_list_hashmap.h"
#include "fixed_arraylist.h"
#include <math.h>

void ren_mat4_identity(ren_mat4_t m)
{
    m[0] = m[5] = m[10] = m[15] = 1.0;
    m[1] = m[2] = m[3] = m[4] = 0.0;
    m[6] = m[7] = m[8] = m[9] = 0.0;
    m[11] = m[12] = m[13] = m[14] = 0.0;
}

void mat4_rotateZ(float radians, ren_mat4_t mat)
{
    ren_mat4_identity(mat);

    /* Rotate Z formula. */
    mat[0] = cosf(radians);
    mat[1] = sinf(radians);
    mat[4] = -sinf(radians);
    mat[5] = cosf(radians);
    mat[10] = 1;
}

void mat4_multiply_vec(ren_mat4_t m, float v[4])
{
    float out[2];

    out[0] = m[0] * v[0] + m[1] * v[1];
    out[1] = m[4] * v[0] + m[5] * v[1];
    v[0] = out[0];
    v[1] = out[1];
}

void ren_mat4_translate(float x, float y, float z, ren_mat4_t mat)
{
    ren_mat4_identity(mat);

    /* Translate slots. */
    mat[12] = x;
    mat[13] = y;
    mat[14] = z;
}

void ren_mat4_scale(float sx, float sy, float sz, ren_mat4_t mat)
{
    ren_mat4_identity(mat);

    /* Scale slots. */
    mat[0] = sx;
    mat[5] = sy;
    mat[10] = sz;
}

#if 0
void ren_mat4_rotateX(float degrees, ren_mat4_t mat)
{
    float radians = degreesToRadians(degrees);

    ren_mat4_identity(mat);

    /* Rotate X formula. */
    mat[5] = cosf(radians);
    mat[6] = -sinf(radians);
    mat[9] = -mat[6];
    mat[10] = mat[5];
}

void ren_mat4_rotateY(float degrees, ren_mat4_t mat)
{
    float radians = degreesToRadians(degrees);

    ren_mat4_identity(mat);

    /* Rotate Y formula. */
    mat[0] = cosf(radians);
    mat[2] = sinf(radians);
    mat[8] = -mat[2];
    mat[10] = mat[0];
}

#endif


void ren_mat4_multiply(ren_mat4_t m1, ren_mat4_t m2, ren_mat4_t result)
{
    /* Fisrt Column */
    result[0] =
	m1[0] * m2[0] + m1[4] * m2[1] + m1[8] * m2[2] + m1[12] * m2[3];
    result[1] =
	m1[1] * m2[0] + m1[5] * m2[1] + m1[9] * m2[2] + m1[13] * m2[3];
    result[2] =
	m1[2] * m2[0] + m1[6] * m2[1] + m1[10] * m2[2] + m1[14] * m2[3];
    result[3] =
	m1[3] * m2[0] + m1[7] * m2[1] + m1[11] * m2[2] + m1[15] * m2[3];

    /* Second Column */
    result[4] =
	m1[0] * m2[4] + m1[4] * m2[5] + m1[8] * m2[6] + m1[12] * m2[7];
    result[5] =
	m1[1] * m2[4] + m1[5] * m2[5] + m1[9] * m2[6] + m1[13] * m2[7];
    result[6] =
	m1[2] * m2[4] + m1[6] * m2[5] + m1[10] * m2[6] + m1[14] * m2[7];
    result[7] =
	m1[3] * m2[4] + m1[7] * m2[5] + m1[11] * m2[6] + m1[15] * m2[7];

    /* Third Column */
    result[8] =
	m1[0] * m2[8] + m1[4] * m2[9] + m1[8] * m2[10] + m1[12] * m2[11];
    result[9] =
	m1[1] * m2[8] + m1[5] * m2[9] + m1[9] * m2[10] + m1[13] * m2[11];
    result[10] =
	m1[2] * m2[8] + m1[6] * m2[9] + m1[10] * m2[10] + m1[14] * m2[11];
    result[11] =
	m1[3] * m2[8] + m1[7] * m2[9] + m1[11] * m2[10] + m1[15] * m2[11];

    /* Fourth Column */
    result[12] =
	m1[0] * m2[12] + m1[4] * m2[13] + m1[8] * m2[14] + m1[12] * m2[15];
    result[13] =
	m1[1] * m2[12] + m1[5] * m2[13] + m1[9] * m2[14] + m1[13] * m2[15];
    result[14] =
	m1[2] * m2[12] + m1[6] * m2[13] + m1[10] * m2[14] +
	m1[14] * m2[15];
    result[15] =
	m1[3] * m2[12] + m1[7] * m2[13] + m1[11] * m2[14] +
	m1[15] * m2[15];
}


void ren_mat4_projection(ren_mat4_t mat, float far, float near,
			 float right, float left, float top, float bottom)
{
    mat[0] = 2.0f / (right - left);
    mat[1] = 0;
    mat[2] = 0;
    mat[3] = 0;

    mat[4] = 0;
    mat[5] = 2.0f / (top - bottom);
    mat[6] = 0;
    mat[7] = 0;

    mat[8] = 0;
    mat[9] = 0;
    mat[10] = -2.0f / (far - near);
    mat[11] = 0;

    mat[12] = -(right + left) / (right - left);
    mat[13] = -(top + bottom) / (top - bottom);
    mat[14] = -(far + near) / (far - near);
    mat[15] = 1.0;
}

void ren_mat4_print(ren_mat4_t mat)
{
    int ii;

    for (ii = 0; ii < 4; ii++)
    {
	int jj;
	printf("{");

	for (jj = 0; jj < 4; jj++)
	{
	    printf("%0.2f ", mat[jj * 4 + ii]);
	}
	printf("},\n");
    }
}
