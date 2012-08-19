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
