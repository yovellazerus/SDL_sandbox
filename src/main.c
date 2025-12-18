#include <SDL3/SDL.h>
#include <stdio.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

int main(void) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) == 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    // Create a window (SDL3 signature: title, width, height, flags)
    SDL_Window *window = SDL_CreateWindow("SDL3 Minimal Template",
                                          WINDOW_WIDTH, WINDOW_HEIGHT,
                                          SDL_WINDOW_RESIZABLE);
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Show window explicitly (SDL3 sometimes starts hidden if flags are 0)
    SDL_ShowWindow(window);

    // Create a renderer for 2D drawing
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Main loop
    int running = 1;
    SDL_Event e;
    while (running) {
        // Handle events
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) running = 0;
            if (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_ESCAPE) running = 0;
        }

        // Clear screen with dark gray
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
        SDL_RenderClear(renderer);

        // Draw a magenta rectangle in the center
        SDL_FRect rect = { (WINDOW_WIDTH-160)/2.0f, (WINDOW_HEIGHT-120)/2.0f, 160, 120 };
        SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
        SDL_RenderFillRect(renderer, &rect);

        // Present to screen
        SDL_RenderPresent(renderer);

        SDL_Delay(16); // ~60 FPS idle
    }

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
