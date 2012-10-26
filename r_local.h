/*
    tc: texcoord
    r: rotation
*/

typedef struct
{
    GLfloat pos[2];
    GLfloat tex[2];
} ren_vertex_tc_t;

typedef struct
{
/* the 3rd position is used as the rotation */
    GLfloat pos[3];
    GLfloat tex[2];
} ren_vertex_tcr_t;

typedef struct {
    GLfloat position[2];
} ren_vertex_position_t;

typedef GLfloat ren_vec4_t[4];
//typedef ren_vec4_t ren_mat4_t[4];
typedef float ren_mat4_t[16];


#define vec4Clear(x) ((x)[0] = (x)[1] = (x)[2] = (x)[3] = 0)

void ren_mat4_projection(ren_mat4_t mat, float far, float near, float right,
		      float left, float top, float bottom);



unsigned int ren_shader_program(unsigned int vertex_shader,
				unsigned int fragment_shader);

unsigned int ren_shader(const char *filename);
