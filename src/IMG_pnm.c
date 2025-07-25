/*
  SDL_image:  An example image loading library for use with SDL
  Copyright (C) 1997-2025 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

/*
 * PNM (portable anymap) image loader:
 *
 * Supports: PBM, PGM and PPM, ASCII and binary formats
 * (PBM and PGM are loaded as 8bpp surfaces)
 * Does not support: maximum component value > 255
 */

#include <SDL3_image/SDL_image.h>

#ifdef LOAD_PNM

/* See if an image is contained in a data source */
bool IMG_isPNM(SDL_IOStream *src)
{
    Sint64 start;
    bool is_PNM;
    char magic[2];

    if (!src) {
        return false;
    }

    start = SDL_TellIO(src);
    is_PNM = false;
    if (SDL_ReadIO(src, magic, sizeof(magic)) == sizeof(magic) ) {
        /*
         * PNM magic signatures:
         * P1   PBM, ascii format
         * P2   PGM, ascii format
         * P3   PPM, ascii format
         * P4   PBM, binary format
         * P5   PGM, binary format
         * P6   PPM, binary format
         * P7   PAM, a general wrapper for PNM data
         */
        if ( magic[0] == 'P' && magic[1] >= '1' && magic[1] <= '6' ) {
            is_PNM = true;
        }
    }
    SDL_SeekIO(src, start, SDL_IO_SEEK_SET);
    return is_PNM;
}

/* read a non-negative integer from the source. return -1 upon error */
static int ReadNumber(SDL_IOStream *src)
{
    int number;
    unsigned char ch;

    /* Initialize return value */
    number = 0;

    /* Skip leading whitespace */
    do {
        if (SDL_ReadIO(src, &ch, 1) != 1 ) {
            return -1;
        }
        /* Eat comments as whitespace */
        if ( ch == '#' ) {  /* Comment is '#' to end of line */
            do {
                if (SDL_ReadIO(src, &ch, 1) != 1 ) {
                    return -1;
                }
            } while ( (ch != '\r') && (ch != '\n') );
        }
    } while ( SDL_isspace(ch) );

    /* Add up the number */
    if (!SDL_isdigit(ch)) {
        return -1;
    }
    do {
        /* Protect from possible overflow */
        if (number >= (SDL_MAX_SINT32 / 10)) {
            return -1;
        }
        number *= 10;
        number += ch-'0';

        if (SDL_ReadIO(src, &ch, 1) != 1 ) {
            return -1;
        }
    } while ( SDL_isdigit(ch) );

    return number;
}

SDL_Surface *IMG_LoadPNM_IO(SDL_IOStream *src)
{
    Sint64 start;
    SDL_Surface *surface = NULL;
    int width, height;
    int maxval, y;
    size_t bpl;
    Uint8 *row;
    Uint8 *buf = NULL;
    char *error = NULL;
    Uint8 magic[2];
    int ascii;
    enum { PBM, PGM, PPM, PAM } kind;

#define ERROR(s) do { error = (s); goto done; } while(0)

    if ( !src ) {
        /* The error message has been set in SDL_IOFromFile */
        return NULL;
    }
    start = SDL_TellIO(src);

    if (SDL_ReadIO(src, magic, 2) != 2 ) {
        return NULL;
    }
    kind = magic[1] - '1';
    ascii = 1;
    if(kind >= 3) {
        ascii = 0;
        kind -= 3;
    }

    width = ReadNumber(src);
    height = ReadNumber(src);
    if(width <= 0 || height <= 0)
        ERROR("Unable to read image width and height");

    if(kind != PBM) {
        maxval = ReadNumber(src);
        if(maxval <= 0 || maxval > 255)
            ERROR("unsupported PNM format");
    } else
        maxval = 255;   /* never scale PBMs */

    /* binary PNM allows just a single character of whitespace after
       the last parameter, and we've already consumed it */

    if(kind == PPM) {
        /* 24-bit surface in R,G,B byte order */
        surface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGB24);
    } else {
        /* load PBM/PGM as 8-bit indexed images */
        surface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_INDEX8);
    }
    if ( surface == NULL )
        ERROR("Out of memory");
    bpl = width * SDL_BYTESPERPIXEL(surface->format);
    if(kind == PGM) {
        SDL_Palette *palette = SDL_CreateSurfacePalette(surface);
        SDL_Color *c;
        int i;
        if (!palette) {
            ERROR("Couldn't create palette");
        }
        c = palette->colors;
        for(i = 0; i < 256; i++)
            c[i].r = c[i].g = c[i].b = i;
    } else if(kind == PBM) {
        /* for some reason PBM has 1=black, 0=white */
        SDL_Palette *palette = SDL_CreatePalette(2);
        SDL_Color *c;
        if (!palette) {
            ERROR("Couldn't create palette");
        }
        c = palette->colors;
        c[0].r = c[0].g = c[0].b = 255;
        c[1].r = c[1].g = c[1].b = 0;
        SDL_SetSurfacePalette(surface, palette);
        SDL_DestroyPalette(palette);

        bpl = (width + 7) >> 3;
        buf = (Uint8 *)SDL_malloc(bpl);
        if(buf == NULL)
            ERROR("Out of memory");
    }

    /* Read the image into the surface */
    row = (Uint8 *)surface->pixels;
    for(y = 0; y < height; y++) {
        if(ascii) {
            if(kind == PBM) {
                int i;
                for(i = 0; i < width; i++) {
                    Uint8 ch;
                    do {
                        if(SDL_ReadIO(src, &ch, 1) != 1)
                               ERROR("file truncated");
                        ch -= '0';
                    } while(ch > 1);
                    row[i] = ch;
                }
            } else {
                size_t i;
                for(i = 0; i < bpl; i++) {
                    int c;
                    c = ReadNumber(src);
                    if(c < 0)
                        ERROR("file truncated");
                    row[i] = c;
                }
            }
        } else {
            Uint8 *dst = (kind == PBM) ? buf : row;
            if(SDL_ReadIO(src, dst, bpl) != bpl)
                ERROR("file truncated");
            if(kind == PBM) {
                /* expand bitmap to 8bpp */
                int i;
                for(i = 0; i < width; i++) {
                    int bit = 7 - (i & 7);
                    row[i] = (buf[i >> 3] >> bit) & 1;
                }
            }
        }
        if(maxval < 255) {
            /* scale up to full dynamic range (slow) */
            size_t i;
            for(i = 0; i < bpl; i++)
                row[i] = row[i] * 255 / maxval;
        }
        row += surface->pitch;
    }
done:
    SDL_free(buf);
    if(error) {
        SDL_SeekIO(src, start, SDL_IO_SEEK_SET);
        if ( surface ) {
            SDL_DestroySurface(surface);
            surface = NULL;
        }
        SDL_SetError("%s", error);
    }
    return surface;
}

#else

/* See if an image is contained in a data source */
bool IMG_isPNM(SDL_IOStream *src)
{
    (void)src;
    return false;
}

/* Load a PNM type image from an SDL datasource */
SDL_Surface *IMG_LoadPNM_IO(SDL_IOStream *src)
{
    (void)src;
    return NULL;
}

#endif /* LOAD_PNM */
