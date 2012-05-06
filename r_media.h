
#include "SDL/SDL.h"
#include "GL/gl.h"
#include <SDL/SDL_image.h>

//
// FOREGROUND STUFF
//
typedef struct media_s
{
    int id;
    SDL_Surface *surface;
    GLuint glImage;
//      void*           surface;
//      unsigned int    glImage;
    char shader[255];           // max file name size
//      int             tagnum;
    int lastx, lasty;           // coord for drawing bg quickly
    int w, h;
    int wReal, hReal;
    int refCount;               /* references on this media */
    bool inuse;                 // is media being used?
    char *fname;
} media_t;
