
#include <GL/glew.h>
#include "r_local.h"
#include "r_draw.h"
#include "r_opengl.h"

void ren_take_screenshot(
    const char *fname
)
{
/*	  void glReadPixels( GLint x,
			     GLint y,
			     GLsizei width,
			     GLsizei height,
			     GLenum format,
			     GLenum type,
			     GLvoid *pixels )
*/

    //unsigned char pixels[200*200*4];

    unsigned int pixels[WIDTH * HEIGHT];

    tea_msg(mREN, 0, "taking screenshot: %s\n", fname);

    glReadPixels(0, 0, WIDTH, HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    int yy;

    /* flip upside down */

    /* "/2" : half way, because we don't want to undo our work */
    for (yy = 0; yy < HEIGHT / 2; yy++)
    {
        int xx;

        for (xx = 0; xx < WIDTH; xx++)
        {
            unsigned int tmp;

            unsigned long int a, b;

            a = xx + yy * WIDTH;
            b = xx + (HEIGHT - yy - 1) * WIDTH;

            assert(xx + yy * WIDTH < HEIGHT * WIDTH);
            tmp = pixels[a];
            pixels[a] = pixels[b];
            pixels[b] = tmp;
        }
    }

#if 0
    for (ii = 0; ii < (WIDTH * HEIGHT) / 2; ii++)
    {
        unsigned int tmp;

        int src;

        int dst;

        src = ii;
        dst = WIDTH * HEIGHT - ii;

        tmp = pixels[ii];
        pixels[ii] = pixels[WIDTH * HEIGHT - ii];
        pixels[WIDTH * HEIGHT - ii] = tmp;
        //tmp = pixels[ii];
        //pixels[ii] = pixels[WIDTH * HEIGHT - ii];
        //pixels[WIDTH * HEIGHT - ii] = tmp;
        //pixels[ii] = 10;
    }
#endif
    //SDL_Surface surf;
    //memset(&surf, 0, sizeof(SDL_Surface));
    //surf.pixels = pixels;

    SDL_Surface *surf;

    surf = SDL_CreateRGBSurface(SDL_SWSURFACE, WIDTH, HEIGHT, 32,
                                0x0000ff, 0x00ff00, 0xff0000, 0x0000000);
//         0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
//   SDL_Surface* surf = SDL_CreateRGBSurfaceFrom(pixels,
//                      SSHOT_W, SSHOT_W, 32, 200*4,
    //Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask);

    FIXME_NEEDS_WORK;   /* save to ropws */
    surf->pixels = pixels;
    SDL_SaveBMP(surf, fname);
}
