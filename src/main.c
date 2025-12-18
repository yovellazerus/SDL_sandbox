#include <SDL3/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "font.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

typedef struct Window_t {
    // SDL classes
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    SDL_FRect box;
    uint32_t outline_color;

    // text info
    // const uint8_t font[95][8];
    uint32_t color;
    float scale;

    float courser_x;
    float courser_y;
} Window_t;

// Render an 8x8 glyph scaled by 'scale'
void screen_render_glyph(SDL_Renderer *renderer, float x, float y, const uint8_t glyph[8], uint32_t color, float scale) {
    if (!renderer || !glyph) return;

    Uint8 r = (color >> 24) & 0xFF;
    Uint8 g = (color >> 16) & 0xFF;
    Uint8 b = (color >> 8 ) & 0xFF;
    Uint8 a = (color >> 0 ) & 0xFF;
    SDL_SetRenderDrawColor(renderer, r, g, b, a);

    SDL_FRect pixel;
    pixel.w = scale;
    pixel.h = scale;

    for (int row = 0; row < 8; row++) {
        uint8_t row_bits = glyph[row];
        for (int col = 0; col < 8; col++) {
            if (row_bits & (1 << (7 - col))) { // MSB = leftmost pixel
                pixel.x = x + col * scale;
                pixel.y = y + row * scale;
                SDL_RenderFillRect(renderer, &pixel);
            }
        }
    }
}

void screen_putc(Window_t* win, char c){
    if(!win) return;
    if(c >= ' ' && c < '~'){
        SDL_SetRenderTarget(win->renderer, win->texture);
        screen_render_glyph(win->renderer, win->courser_x, win->courser_y, FONT0[c - ' '], win->color, win->scale);
        SDL_SetRenderTarget(win->renderer, NULL); // reset to screen
        win->courser_x += 8*win->scale;
        if(win->courser_x - 8*win->scale >= win->box.w - 8*win->scale){
            win->courser_x = win->box.x;
            win->courser_y += 8*win->scale;
            if(win->courser_y - 8*win->scale >= win->box.h - 8*win->scale){
                win->courser_y = win->box.y;
            }
        }
    }
    if(c == '\n'){ win->courser_x = win->box.x; win->courser_y += 8*win->scale; }
    if(c == '\r'){ win->courser_x = win->box.x; }
    if(c == '\t'){ win->courser_x += 8*4*win->scale; }
    return;
}

int main(void) {
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Texture* texture   = NULL;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("RISCV32 emulator screen (in the early stages)", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);
    if (!window) return 1;

    renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) return 1;

    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        WINDOW_WIDTH,
        WINDOW_HEIGHT
    );
    if(!texture) return 1;

    Window_t myWindow = {
        window,
        renderer,
        texture,
        .box = {.x = 0, .y = 0, .h = WINDOW_HEIGHT/2, .w = WINDOW_WIDTH/2},
        0xff0000ff,
        0xffffffff,
        2.0,

        // WINDOW_WIDTH/3,
        // WINDOW_HEIGHT/4,
    };

    // clear texture once
    SDL_SetRenderTarget(renderer, texture);
    SDL_SetRenderDrawColor(renderer, 0,0,0,255);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, NULL);

    SDL_SetRenderTarget(renderer, texture);
    SDL_SetRenderDrawColor(renderer, myWindow.outline_color, 0, 0, 255);
    SDL_RenderRect(renderer, &myWindow.box);
    SDL_SetRenderTarget(renderer, NULL);

    // input vars
    char c = 0;
    bool shift = false;

    // main loop
    bool running = true;
    SDL_Event e;
    while (running)
    {
        // event loop
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) running = false;
            if (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_ESCAPE) running = false;

            if (e.type == SDL_EVENT_KEY_DOWN) {

                shift = (SDL_GetModState() & SDL_KMOD_SHIFT) != 0;

                // letters
                if (e.key.key >= SDLK_A && e.key.key <= SDLK_Z) {
                    c = shift ? ('A' + (e.key.key - SDLK_A)) : ('a' + (e.key.key - SDLK_A));
                }

                // numbers and symbols
                else if (e.key.key >= SDLK_0 && e.key.key <= SDLK_9){
                    if (shift) {
                        const char symbols[] = {')','!','@','#','$','%','^','&','*','('};
                        c = symbols[e.key.key - SDLK_0];
                    } else {
                        c = '0' + (e.key.key - SDLK_0);
                    }
                }

                // space
                else if (e.key.key == SDLK_SPACE) {
                    c = ' ';
                } 

                // new line
                else if (e.key.key == SDLK_RETURN) {
                    c = '\n';
                }
                
                screen_putc(&myWindow, c);
                c = 0;
            }
        }
        // draw the texture to the screen
        SDL_SetRenderTarget(renderer, NULL);
        SDL_RenderTexture(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_DestroyTexture(texture);
    SDL_Quit();

    return 0;
}
