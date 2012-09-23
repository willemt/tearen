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
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include "tea_vec.h"
#include <GL/gl.h>
#include "r_draw.h"
#include "r_local.h"
#include "linked_list_hashmap.h"
#include "fixed_arraylist.h"

static char *file_contents(const char *filename, int *len_out)
{
    char *buffer = 0;
    long length;
    FILE *f = fopen(filename, "rb");

    if (f)
    {
	fseek(f, 0, SEEK_END);
	length = ftell(f);
	fseek(f, 0, SEEK_SET);
	buffer = malloc(length);
	*len_out = length;
	if (buffer)
	{
	    if (0 == fread(buffer, 1, length, f))
	    {

	    }
	}
	fclose(f);
    }
    return buffer;
}

unsigned int ren_shader_program(unsigned int vertex_shader,
				unsigned int fragment_shader)
{
    GLint program_ok;

    GLuint program;

    if (0 == (program = glCreateProgram()))
    {
	assert(0);
    }

    glAttachShader(program, vertex_shader);
//    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    /* check success status of linking */
    glGetProgramiv(program, GL_LINK_STATUS, &program_ok);
    if (!program_ok)
    {
	fprintf(stderr, "Failed to link shader program:\n");
//      show_info_log(program, glGetProgramiv, glGetProgramInfoLog);
	glDeleteProgram(program);
	return 0;
    }

    glValidateProgram(program);
    GLint status;
    glGetProgramiv(program, GL_VALIDATE_STATUS, &status);

    if (status == GL_FALSE)
    {
	assert(0);
    }

    return program;
}

unsigned int ren_shader(const char *filename)
{
    GLint length;
    GLuint shader;
    GLint shader_ok;
    GLchar *source;
    GLenum type;

    if (strstr(filename, ".vert."))
    {
	type = GL_VERTEX_SHADER;
    } else if (strstr(filename, ".frag."))
    {
	type = GL_FRAGMENT_SHADER;
    } else
    {
	assert(0);
    }

    /* load shader source */
    source = file_contents(filename, &length);

    if (!source)
    {
	fprintf(stderr, "Failed to load %s:\n", filename);
	exit(0);
	return 0;
    }

    if (0 == (shader = glCreateShader(type)))
    {
	assert(0);
    }

    /* compile shader */
    glShaderSource(shader, 1, (const GLchar **) &source, &length);
    free(source);
    glCompileShader(shader);

    /* check sucess status of compilation */
    glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_ok);

    if (!shader_ok)
    {
	fprintf(stderr, "Failed to compile %s:\n", filename);
//      show_info_log(shader, glGetShaderiv, glGetShaderInfoLog);
	glDeleteShader(shader);
	exit(0);
	return 0;
    }

    return shader;
}

int ren_shader_new(const char *filename)
{
    return 0;
}
