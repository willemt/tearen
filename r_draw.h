/* 
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
* Tactical Elemental Arenas
*  C Header: renderer draw 
*
*
*
* Author: Willem-Hendrik Thiart <beandaddy@gmail.com>, (C) 2006
* Copyright: See COPYING file that comes with this distribution
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*/

#ifndef _R_DRAW_H
#define _R_DRAW_H

#include <stdlib.h>
#include "tea_vec.h"

typedef int indexGFX;

#define R_MAX_MEDIA_NAME_LENGTH 255
#define R_MAX_SHADER_PROGRAMS 2
#define R_MAX_RENTITIES 1024
#define R_FULLSCREEN (1<<0)
#define R_TESTGRID (1<<1)
#define R_MAX_TEXT_LEN 64

#define MAX_EFFECTS 8

#define R_INITSHADER_BILINEAR (1<<0)
#define R_INITSHADER_NEAREST (1<<1)


#define R_OFFSET_NORMAL 0       // drawn with screen
#define R_OFFSET_NONE 1

#define R_FONT_W 12
#define R_FONT_H 26

#define PARTICLES_FRONTBUFF 0
#define PARTICLES_BACKBUFF 1

// ================--|AA|RR|GG|BB
#define ANIM_TAG_COLOUR_ALPHA 0xffff00ff        // little-endian needed on i386? wtf?
#define ANIM_TAG_COLOUR_BETA 0xffffff00
#define ANIM_TAG_COLOUR_GAMMA 0xff00ffff

#define RENTF_COLORED (1<<1)


typedef struct
{
    float pos[3];
    float tex[2];
} ren_vertex_tc_t;

enum
{
    REN_EFFECT_BLUIFY25,
    REN_EFFECT_REDIFY25,
    REN_EFFECT_HURT,
    REN_EFFECT_FROZEN,
    REN_EFFECT_BURNING,
    /* as seen in odin sphere.
     * when someone gets hurt */
    REN_EFFECT_MICRO_SHAKE,
    REN_EFFECT_BLUR,
    REN_EFFECT_LARGEN,
};

typedef struct
{
    int param[2];
    int type;

} ren_effect_ins_t;

typedef struct
{
    ren_effect_ins_t ins[MAX_EFFECTS];
    int size;
//    int params[MAX_EFFECTS * 2];
    int id;
} ren_effect_t;

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
} r_enttype_e;

typedef struct
{

    void *in;
} ren_object_t;

#if 0
typedef enum
{
    RENT_IMG,
    RENT_TEXT,
    RENT_TEXT_SHADOW,
    RENT_CALLB_ONLY,
} r_enttype_e;
#endif

#if 1
typedef struct
{
//      int font;
    char string[R_MAX_TEXT_LEN];
} ren_text_t;
#endif

/*
typedef struct {
	int font;
} ren_entity_text_info_t;

typedef union {
	ren_entity_text_info_t txt;
} ren_entity_info_t;
*/

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

#if 0
    /* type/options */
    int pstrainDepth;
    void *pstrain;              // pixel shader train
    unsigned char pShader;
    int pixelShader;            // pixel shader index
#endif

    int flags;

    ren_effect_t *effect;

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

//#define LCLIENTF_REDSKY (1<<1)
#define RF_REDSKY (1<<1)        // draw in red sky man
typedef struct
{
    int flags;

    /*int           pstrainsNum; // number of unique pstrains
     * void*                pstrainHeader; // array grows and dies
     * void*                pstrainCurr; // current pixel shader train
     * void*                pstrainLatest; // latest added pixel shader
     */
    /* orientation */
    float scale;
//    int w, h;
//    int x, y;
    vec2_t scrAcc;              // screen acel
    vec2_t cameraOrg, cameraTarg;
    float zoom;

    void *in;
} ren_renderer_t;

extern int /*GLhandleARB */ rProgram[R_MAX_SHADER_PROGRAMS];

extern ren_entity_t renderable_entities[R_MAX_RENTITIES];

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

#define ren_drawContextSet ren_draw_setContext

void ren_draw_begin(
);

void ren_draw_step(
    vec2_t camera_org
);

void ren_draw_end(
    void
);

void ren_draw_push(
    ren_entity_t * rent
);

//void ren_rent_enque(ren_entity_t *rent);
//void ren_drawEnque(ren_entity_t *rent);

#define ren_drawEnque ren_draw_push

//void ren_draw_raw(ren_entity_t* rent);
void ren_draw_setCamera(
    vec2_t camera_orig,
    vec2_t camera_targ
);

//void ren_drawSet(int flag);
void ren_draw_enable(
    int flag
);

void ren_draw_disable(
    int flag
);

void ren_draw_feature(
    int flag
);

#define ren_drawSet ren_draw_enable

//
// r_rentity.c
//
ren_entity_t *ren_rent_clean(
    ren_entity_t * rent
);

ren_entity_t *ren_rent_enqueue(
    ren_entity_t * rent
);

ren_entity_t *ren_rent_prep(
    ren_entity_t * rent
);

void ren_rent_set_alpha(
    ren_entity_t * rent,
    unsigned char a
);

/*-------------------------------SQUARE---------------------------------------*/

#define ren_rentrect_set_w_and_h(rent, _w, _h)\
	(rent)->rect.w = (_w);\
	(rent)->rect.h = (_h)

#define ren_rentsquare_set_w(rent, _w)\
	(rent)->rect.w = (_w)

#define ren_rentsquare_get_w(rent)\
	(rent)->rect.w

#define ren_rentsquare_get_h(rent)\
	(rent)->rect.w

#define ren_rentsquare_center_on_org(rent)\
{\
    int w = ren_rentsquare_get_w((rent));\
	vec2Set((rent)->rotCentre, w/2, w/2);\
    (rent)->org[X] -= w/2;\
    (rent)->org[Y] -= w/2;\
}

/*-------------------------------RECT-----------------------------------------*/

#define ren_rentrect_set_w(rent, _w)\
	(rent)->rect.w = (_w)

#define ren_rentrect_set_h(rent, _h)\
	(rent)->rect.h = (_h)

#define ren_rentrect_get_w(rent)\
	(rent)->rect.w

#define ren_rentrect_get_h(rent)\
	(rent)->rect.h

#define ren_rentrect_center_on_org(rent)\
{\
    int w = ren_rentrect_get_w((rent));\
    int h = ren_rentrect_get_h((rent));\
	vec2Set((rent)->rotCentre, w/2, h/2);\
    (rent)->org[X] -= w/2;\
    (rent)->org[Y] -= h/2;\
}

int rRegisterPShader(
    char *handle
);

void rLoadGfx(
    void
);

 //
// draw_utils.c
//
indexGFX rRegisterImage(
    char *handle
);


void *ren_medias_get(
    int idx
);

int ren_media_getOGLTex(
    int idx
);

int ren_media_get(
    char *fname
);

void ren_media_release(
    int idx
);

#define ren_media_release rMedia_release

#define rMedia_getOGLTex ren_media_getOGLTex

#define rMedias_get ren_medias_get

#define ren_media_get rMedia_get
#define rMedia_idx rMedia_get
#define rMedia_register rMedia_idx

int rShaderGet_height(
    indexGFX index
);

int rShaderGet_width(
    indexGFX index
);

void rShaderGet_wANDh(
    indexGFX index,
    int *w,
    int *h
);

#define rGetShaderHeight(x) rShaderGet_height(x)        // is deprecated naming convention
#define rGetShaderWidth(x) rShaderGet_width(x)

int rShaderActivate(
    indexGFX index
);

int rShaderDeactivate(
    indexGFX index
);

void *rLoadImageFile(
    char *handle
);

int rSurfaceColour2Vec(
    char *handle,
    int colour,
    vec2_t pixel_org
);

int rShaderHeight(
    char *handle
);

int rShaderWidth(
    char *handle
);

//
// cl_hud.c
//
void rHudMain(
);

int rConFont_load(
    char *handle
);

#define rLoadFont(x) rConFont_load(x)
void rConSetFont(
);

float rConFont_width(
    float w
);

float rConFont_height(
    float h
);

int rPrint(
    char *string,
    int x,
    int y,
    float scale
);

int rPrintBox(
    char *string,
    rect_t rect,
    float scale
);

void rConPrint(
    char *str
);


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

int rFPrint(
    int font_idx,
    float x,
    float y,
    float scale,
    int max_w,
    char *format,
    ...
);

int rFPrint_shadow(
    int font_idx,
    float x,
    float y,
    float scale,
    int max_w,
    vec4_t color,
    char *format,
    ...
);

int ren_printf(
    int font_idx,
    float x,
    float y,
    float scale,
    char *format,
    ...
);


void ren_rent_draw(
    ren_entity_t * rent
);

const char *ren_media_get_filename(
    const void *media
);

void ren_medias_step(
);

int ren_term_main(
    void *screen
);

int ren_term_init(
    void *screen
);

void ren_term_checkEvents(
    void *event
);

int ren_term_printf(
    char *string,
    ...
);

void ren_term_initDone(
);

void ren_term_toggle(
    int num
);

void ren_term_background(
    int num
);

void ren_term_clear(
);

void ren_term_setLine(
    int num,
    char *line
);

void ren_term_setCallbackTab(
    int num,
    void (*tab_event_func) (char *line)
);

int ren_term_isEnabled(
    int id
);

int ren_term_charPress(
    int id,
    const void *event
);

int ren_term_create(
);

int ren_term_draw(
    int id
);

void ren_take_screenshot(
    const char *fname
);

/* private */
void ren_effect_apply_on_rentity(
    ren_effect_t * prev,
    ren_effect_t * eff,
    ren_entity_t * rent
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
