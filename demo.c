
#include "r_draw.h"

#include "SDL/SDL.h"
#include "SDL/SDL_main.h"


/* for details around SDL_main see:
http://sdl.beuc.net/sdl.wiki/FAQ_MacOS_X_SDLMain.m
*/

#if defined(DARWIN)
int SDL_main(int argc, char **argv)
#else
int main(int argc, char **argv)
#endif
{
    ren_object_t *obs[1000];
    ren_object_t *canvas;
    int ii;
    int media;

    ren_draw_init(argv[0]);
    canvas = ren_obj_init(RENT_CANVAS);
    media = ren_media_get("default.png");
    media = ren_media_get("rss.png");

    for (ii = 0; ii < 10; ii++)
    {
	vec2_t org;

	org[0] = (float) ii *40;	// / 100;
	org[1] = (float) ii *40;	// / 100;
	obs[ii] = ren_obj_init(RENT_SQUARE);
	ren_obj_set_media(obs[ii], media);
	ren_obj_set_w(obs[ii], 64);
	ren_obj_set_h(obs[ii], 64);
	ren_obj_set_org(obs[ii], org);
	ren_obj_set_parent(obs[ii], canvas);
    }


#if 0
    for (ii = 100; ii < 200; ii++)
    {
	vec2_t org;

	org[0] = (float) random() / RAND_MAX - 0.5;	// * 400;
	org[1] = (float) random() / RAND_MAX - 0.5;
	obs[ii] = ren_obj_init(RENT_SQUARE);
	ren_obj_set_media(obs[ii], ren_media_get("default.png"));
	ren_obj_set_w(obs[ii], 1);
	ren_obj_set_h(obs[ii], 1);
	ren_obj_set_org(obs[ii], org);
	ren_obj_set_parent(obs[ii], canvas);
    }
#endif

    int degree = 0;

    while (1)
    {
	vec2_t org;
	int mx, my;

	ren_frame_begin();

	ren_obj_draw(canvas);

	ren_frame_end();

	SDL_PumpEvents();

        vec2Set(org, 300, 100);
	ren_obj_set_org(obs[0], org);
	ren_obj_set_w(obs[0], 32);

	SDL_GetMouseState(&mx, &my);
        vec2Set(org, mx, my);
	ren_obj_set_endpos(obs[0], org);

        for (ii=0; ii<10; ii++)
        {
            ren_obj_set_zrot(obs[ii], (float)degree / 100);
        }

	SDL_Delay(10);

	degree += 1;
    }

    return 0;
}
