/*
  showimage: A test application for SDL_image on Dreamcast
*/

#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include "SDL/SDL_dreamcast.h"

int main(int argc, char *argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    // Set the image path (romdisk)
    const char *image_path = "/rd/Troy2024.png";
    printf("Loading PNG: %s\n", image_path);

    // Load the PNG using SDL_image
    SDL_Surface *image = IMG_Load(image_path);
    if (!image) {
        printf("IMG_Load failed: %s\n", IMG_GetError());
        SDL_Quit();
        return 1;
    }
    printf("Image loaded: %dx%d Bpp=%d\n", image->w, image->h, image->format->BitsPerPixel);

    // Set the Dreamcast video driver and screen mode
    SDL_DC_SetVideoDriver(SDL_DC_DIRECT_VIDEO);
    SDL_Surface *screen = SDL_SetVideoMode(640, 480, 32, SDL_SWSURFACE | SDL_DOUBLEBUF | SDL_FULLSCREEN);
    if (!screen) {
        printf("SDL_SetVideoMode failed: %s\n", SDL_GetError());
        SDL_FreeSurface(image);
        SDL_Quit();
        return 1;
    }

    // Optimize the image to the screen format
    SDL_Surface *optimized = SDL_DisplayFormat(image);
    SDL_FreeSurface(image);
    image = optimized;

    // Blit the image to the screen
    SDL_BlitSurface(image, NULL, screen, NULL);
    SDL_Flip(screen);

    printf("Image displayed. Press START or any key to exit...\n");

    // Wait for key or quit event
    SDL_Event e;
    int running = 1;
    while (running) {
        while (SDL_PollEvent(&e)) {
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
