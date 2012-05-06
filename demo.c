

#include "r_draw.h"


int main(
    int argc,
    char **argv
)
{
    ren_object_t *obs[1000];
    ren_object_t *canvas;
    int ii;

    ren_draw_init(argv[0]);
    canvas = ren_obj_init(RENT_CANVAS);

    for (ii = 0; ii < 100; ii++)
    {
        vec2_t org;

        org[0] = (float) random() / RAND_MAX - 0.5;     // * 400;
        org[1] = (float) random() / RAND_MAX - 0.5;
        obs[ii] = ren_obj_init(RENT_SQUARE);
        ren_obj_set_media(obs[ii], ren_media_get("default.png"));
        ren_obj_set_w(obs[ii], 1);
        ren_obj_set_h(obs[ii], 1);
        ren_obj_set_org(obs[ii], org);
        ren_obj_set_parent(obs[ii], canvas);
    }

    while (1)
    {
        ren_frame_begin();

        ren_obj_draw(canvas);

        ren_frame_end();
    }

    return 0;
}
