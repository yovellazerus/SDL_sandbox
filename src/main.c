#include <SDL3/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "font.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

typedef struct Screen_t {
    uint32_t width;
    uint32_t hight;
    // SDL classes
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;

    uint32_t color;
    float scale;

    float courser_x;
    float courser_y;
} Screen_t;

Screen_t* Screen_create(uint32_t width, uint32_t hight, uint32_t color, float scale){
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Texture* texture   = NULL;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s\n", SDL_GetError());
        return NULL;
    }

    window = SDL_CreateWindow("RISCV32 emulator screen (in the early stages)", width, hight, SDL_WINDOW_RESIZABLE);
    if (!window) return NULL;

    renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) return NULL;

    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        width,
        hight
    );
    if(!texture) return NULL;    

    Screen_t* res = (Screen_t*)malloc(sizeof(*res));
    if(!res) return NULL;

    res->width = width;
    res->hight = hight;
    res->window = window;
    res->renderer = renderer;
    res->texture = texture;
    res->color = color;
    res->scale = scale;
    return res;
}

void Screen_destroy(Screen_t* screen){
    if(!screen) return;
    SDL_DestroyRenderer(screen->renderer);
    SDL_DestroyWindow(screen->window);
    SDL_DestroyTexture(screen->texture);
    SDL_Quit();
}

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

void screen_putc(Screen_t* screen, char c){
    if(!screen) return;
    if(c >= ' ' && c < '~'){
        SDL_SetRenderTarget(screen->renderer, screen->texture);
        screen_render_glyph(screen->renderer, screen->courser_x, screen->courser_y, FONT0[c - ' '], screen->color, screen->scale);
        SDL_SetRenderTarget(screen->renderer, NULL); // reset to screen
        screen->courser_x += 8*screen->scale;
        if(screen->courser_x - 8*screen->scale >= screen->width - 8*screen->scale){
            screen->courser_x = screen->width;
            screen->courser_y += 8*screen->scale;
            if(screen->courser_y - 8*screen->scale >= screen->hight - 8*screen->scale){
                screen->courser_y = screen->hight;
            }
        }
    }
    if(c == '\n'){ screen->courser_x = screen->width; screen->courser_y += 8*screen->scale; }
    if(c == '\r'){ screen->courser_x = screen->width; }
    if(c == '\t'){ screen->courser_x += 8*4*screen->scale; }
    return;
}

int main(void) {
    
    Screen_t* screen = Screen_create(WINDOW_WIDTH, WINDOW_HEIGHT, 0xffffffff, 2.0);
    if(!screen) return 1;

    // clear texture once
    SDL_SetRenderTarget(screen->renderer, screen->texture);
    SDL_SetRenderDrawColor(screen->renderer, 0,0,0,255);
    SDL_RenderClear(screen->renderer);
    SDL_SetRenderTarget(screen->renderer, NULL);

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
                
                screen_putc(screen, c);
                c = 0;
            }
        }
        // draw the texture to the screen
        SDL_SetRenderTarget(screen->renderer, NULL);
        SDL_RenderTexture(screen->renderer, screen->texture, NULL, NULL);
        SDL_RenderPresent(screen->renderer);
    }
    
    Screen_destroy(screen);

    return 0;
}
