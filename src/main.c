#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "display.h"
#include "gfx-vector.h"

bool is_running = false;

bool setup(void)
{
	color_buffer = (uint32_t *)malloc(window_width * window_height * sizeof(uint32_t));
	
	if (!color_buffer) {
		fprintf(stderr, "Error: malloc failed for color_buffer.\n");
		return false;
	}

    // Create a texture buffer that displays the color buffer.
	color_buffer_texture = SDL_CreateTexture(
		renderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		window_width,
		window_height
	);

    if (! color_buffer_texture) {
		fprintf(stderr, "Error: SDL_CreateTexture failed\n");
		return false;
	}

	return true;
}

void process_input(void)
{
    SDL_Event event;
	SDL_PollEvent(&event);

	switch (event.type) {
		case SDL_QUIT:  // User clicks window close button
		    is_running = false;
			break;
		case SDL_KEYDOWN:
		    if (event.key.keysym.sym == SDLK_ESCAPE) {
				is_running = false;
			}
			break;
		default:
		    break;

	}
}

void update(void)
{
	// TODO:
}

void render(void)
{
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	SDL_RenderClear(renderer);

    draw_grid();

    draw_pixel(10,10,0xFFFFFF00);
    draw_pixel(11,10,0xFFFFFF00);
    draw_pixel(10,11,0xFFFFFF00);
    draw_pixel(11,11,0xFFFFFF00);

	draw_rect(20,30,100,150, 0xFFFF0000);

	draw_rect(200, 250, 50, 75, 0xFFAAAAAA);

    render_color_buffer();

	clear_color_buffer(0xFF000000);

	SDL_RenderPresent(renderer);
}

int main(void) {

	is_running = initialize_window();

	if (! setup()) {
		// If setup failed, don't run.
		is_running = false;
	}

	while (is_running) {
		process_input();
		update();
		render();
	}

    destroy_window();
	return 0;
}
