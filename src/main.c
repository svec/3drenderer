#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "display.h"
#include "gfx-vector.h"

const int CUBE_N_POINTS = 9*9*9; // 9 points in each dimension

vec3_t cube_points[CUBE_N_POINTS];

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

	// Setup cube, centered at 0,0,0, each side is length 2 (from -1 to +1).
	// Each point is 0.25 off from the other (CUBE_N_POINTS per dimension = 9).
	int point_count = 0;
	for (float x = -1.0; x < 1; x += 0.25) {
		for (float y = -1.0; y < 1; y += 0.25) {
			for (float z = -1.0; z < 1; z += 0.25) {
				vec3_t new_point = {x,y,z};
				cube_points[point_count++] = new_point;
			}
		}
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
