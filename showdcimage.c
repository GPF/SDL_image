/*
  showimage: A test application for SDL_image on Dreamcast
*/

#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include "SDL_dreamcast.h"
#include <dc/video.h>

// gpf@GPF:~/code/dreamcast/SDL_image$ /opt/toolchains/dc/kos/utils/pvrtex/pvrtex -i romdisk/Troy2024.png -f rgb565 -o romdisk/Troy2024.pvr
// Reading input...
// Twiddling...
// Converting as uncompressed...
// Writing .PVR to "romdisk/Troy2024.pvr"...

void BlitImageToPVR(SDL_Surface *src, SDL_Surface *dst) {
    Uint16 *dstPixels = (Uint16 *)dst->pixels;
    int dstPitch = dst->pitch / 2; // pixels per row in dst (512 for 512x256 texture)

    for (int y = 0; y < src->h; ++y) {
        Uint8 *srcRow = (Uint8 *)src->pixels + y * src->pitch;
        Uint16 *dstRow = dstPixels + y * dstPitch;

        for (int x = 0; x < src->w; ++x) {
            Uint8 r, g, b;
            if (src->format->BytesPerPixel == 3) {
                r = srcRow[x * 3 + 0];
                g = srcRow[x * 3 + 1];
                b = srcRow[x * 3 + 2];
            } else {
                r = srcRow[x * 4 + 0];
                g = srcRow[x * 4 + 1];
                b = srcRow[x * 4 + 2];
            }
            dstRow[x] = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
        }
        // Fill the rest of the row with black
        for (int x = src->w; x < dst->w; ++x) {
            dstRow[x] = 0x0000;
        }
    }
}

int main(int argc, char *argv[]) {
    // Initialize SDL
    SDL_DC_MapKey(0, SDL_DC_START, SDLK_ESCAPE);
    SDL_DC_MapKey(0, SDL_DC_A, SDLK_SPACE);
    // SDL_DC_SetVideoDriver(SDL_DC_DIRECT_VIDEO);
    // SDL_DC_SetVideoDriver(SDL_DC_DMA_VIDEO);
    // SDL_DC_SetVideoDriver(SDL_DC_TEXTURED_VIDEO);    

    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK) < 0) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    SDL_JoystickOpen(0);
    // Set the image path (romdisk)
    const char *image_path = "/rd/Troy2024.pvr";
    printf("Loading PVR: %s\n", image_path);

    // Load the PNG using SDL_image
    SDL_Surface *image = IMG_Load(image_path);
    if (!image) {
        printf("IMG_Load failed: %s\n", IMG_GetError());
        SDL_Quit();
        return 1;
    }
    printf("Image loaded: %dx%d Bpp=%d\n", image->w, image->h, image->format->BitsPerPixel);

    // Set the Dreamcast video driver and screen mode

    SDL_Surface *screen = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE | SDL_DOUBLEBUF );
    // SDL_Surface *screen = SDL_SetVideoMode(512, 256, 16, SDL_HWSURFACE | SDL_DOUBLEBUF ); //textured video
    if (!screen) {
        printf("SDL_SetVideoMode failed: %s\n", SDL_GetError());
        SDL_FreeSurface(image);
        SDL_Quit();
        return 1; 
    }
    // SDL_DC_SetWindow(320,240); //textured video
    // Optimize the image to the screen format
    // Clear texture to black
    // memset(screen->pixels, 0, screen->h *  screen->pitch);
    // BlitImageToPVR(image, screen);
    // SDL_Flip(screen);
		SDL_BlitSurface(image, NULL, screen, NULL);
		SDL_UpdateRect(screen, 0, 0, 0, 0);
        SDL_Flip(screen);
    printf("Image displayed. Press START or any key to exit...\n");

    // Wait for key or quit event
    SDL_Event e;
    int running = 1;
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.key.keysym.sym == SDLK_SPACE) {
                // Take single screenshot
                vid_screen_shot("/pc/screenshot.ppm");
            }
            if (e.type == SDL_KEYDOWN || e.type == SDL_QUIT) {
                running = 0;
            }
        }
        SDL_Delay(100);
    }

    SDL_FreeSurface(image);
    SDL_Quit();
    return 0;
}
