
#ifndef _R_DRAW_H
#define _R_DRAW_H

#include <stdlib.h>
#include "tea_vec.h"

#define R_MAX_MEDIA_NAME_LENGTH 255
#define R_MAX_SHADER_PROGRAMS 2
#define R_MAX_RENTITIES 1024
#define R_FULLSCREEN (1<<0)
#define R_TESTGRID (1<<1)
#define R_MAX_TEXT_LEN 64

#define R_OFFSET_NORMAL 0       // drawn with screen
#define R_OFFSET_NONE 1

// ================--|AA|RR|GG|BB
#define ANIM_TAG_COLOUR_ALPHA 0xffff00ff
#define ANIM_TAG_COLOUR_BETA 0xffffff00
#define ANIM_TAG_COLOUR_GAMMA 0xff00ffff

#define RENTF_COLORED (1<<1)

typedef enum
{
    RENT_SQUARE,
    RENT_SQUARE_ROTATE,
    RENT_SQUARE_CENTER,         // center point is middle of rect
//      RENT_SQUARE_COLORED,
    RENT_RECT,
//      RENT_RECT_CENTER, // center point is middle of rect
//      RENT_RECT_COLORED,
//      RENT_POLYS_COLORED,
    RENT_POSTPIXELS,
    RENT_TEXT,
//      RENT_TEXT_COLORED,
    RENT_TEXT_SHADOWED,
//      RENT_TEXT_SHADOWED_COLORED,
    /* draw a skeleton, using the ik */
    RENT_SKELETON,
    RENT_LIGHT,
    RENT_CALLBACK,
    RENT_QUAD,
    RENT_BEAM,
    RENT_ARCHIVE,               // display list 
    RENT_CANVAS,
    RENT_SQUARE_VBO,
} r_enttype_e;

typedef struct
{

    void *in;
} ren_object_t;

typedef struct
{
//      int font;
    char string[R_MAX_TEXT_LEN];
} ren_text_t;

typedef struct
{
    int media;                  // cached texture memory in renderer
    int w, h;
} ren_entity_rect_t;

typedef struct
{
    char *string;               // string data
    int useColor;
    int font;
    void *tdata;
} ren_entity_text_t;

typedef struct
{
    int media;
    vec2_t end;
} ren_entity_quad_t;

typedef struct
{
    int media;
    vec2_t end;
    int w;
} ren_entity_beam_t;



typedef struct
{

} ren_entity_light_t;

typedef struct
{

} ren_entity_skeleton_t;

typedef struct
{

} ren_entity_postpixels_t;

typedef struct
{
    void *udata;
    int (
    *callb
    )   (
    void *rent,
    void *udata
    );
} ren_entity_cb_t;

#if 0
// this is what we send to the renderer from cgame
typedef struct ren_entity_s
{
    /* content */
    vec4_t color;
    /* position */
    vec2_t org;
    vec2_t rotCentre;
    vec3_t rotAngle;            // rotate into 3 dimensions! thanks opengl!
    /* make-up */
    int z;                      // for depth testing // and fog effects?

    float scale;
//      ren_entity_info_t info;

    /* way of ordering based on enqueue_num as well */
    unsigned char enqueue_order;
    unsigned char offset;       // the way this is drawn from the camera
    unsigned char alpha;
    unsigned char type;

    /*  bool */
    int mirror_yaxis;

    int flags;

    union
    {
        ren_entity_rect_t rect;
        ren_entity_postpixels_t postpixels;
        ren_entity_text_t text;
        ren_entity_cb_t cb;
        ren_entity_quad_t quad;
        ren_entity_beam_t beam;
    };
} ren_entity_t;
#endif

typedef struct
{
    int flags;

    /* orientation */
    float scale;
    vec2_t scrAcc;              // screen acel
    vec2_t cameraOrg, cameraTarg;
    float zoom;

    void *in;
} ren_renderer_t;

extern ren_renderer_t *rSys;

 //
// draw.c
//
void ren_draw_init(
);

void ren_beginFrame(
    void
);

void ren_endFrame(
    void
);

void ren_draw_setContext(
    ren_renderer_t * context
);


void ren_draw_begin(
);

void ren_draw_step(
    vec2_t camera_org
);

void ren_draw_end(
    void
);


void ren_draw_setCamera(
    vec2_t camera_orig,
    vec2_t camera_targ
);

void ren_draw_enable(
    int flag
);

void ren_draw_disable(
    int flag
);

void ren_draw_feature(
    int flag
);
/*----------------------------------------------------------------------------*/

void *ren_medias_get(
    int idx
);

int ren_media_get(
    const char *fname
);

void ren_media_release(
    int idx
);

/*----------------------------------------------------------------------------*/

int ren_font_init(
    char *fname,
    unsigned int h
);

int ren_font_width(
    int font_idx
);

int ren_font_height(
    int font_idx
);

/*----------------------------------------------------------------------------*/
int ren_printf(
    int font_idx,
    float x,
    float y,
    float scale,
    char *format,
    ...
);

const char *ren_media_get_filename(
    const void *media
);

void ren_medias_step(
);
void ren_take_screenshot(
    const char *fname
);

int ren_obj_release(
    ren_object_t * rob
);


int ren_obj_set_media(
    ren_object_t * rob,
    int media_id
);


int ren_obj_set_rot_org(
    ren_object_t * rob,
    vec2_t rot_org
);

int ren_obj_set_zrot(
    ren_object_t * rob,
    float zrot
);

int ren_obj_set_org_from_xy(
    ren_object_t * rob,
    const float x,
    const float y
);

int ren_obj_get_w(
    ren_object_t * rob
);


int ren_obj_set_org_from_xy(
    ren_object_t * rob,
    float x,
    float y
);


int ren_obj_set_w(
    ren_object_t * rob,
    int w
);

void ren_obj_add_child(
    ren_object_t * rob,
    ren_object_t * child
);

ren_object_t *ren_obj_init(
    const int type
);

int ren_obj_draw(
    ren_object_t * rob
);


void ren_objs_draw(
);

/*----------------------------------------------------------------------------*/

void *r_load_image(
    const char *fname
);

unsigned long r_hash_string(
    const void *e1
);

long r_cmp_string(
    const void *e1,
    const void *e2
);


#endif /* _R_DRAW_H */

/*--------------------------------------------------------------79-characters-*/
/* vim: set expandtab! : */
