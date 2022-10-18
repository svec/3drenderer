#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "display.h"
#include "gfx-vector.h"

const int CUBE_N_POINTS = 9*9*9; // 9 points in each dimension

vec3_t cube_points[CUBE_N_POINTS];
vec2_t projected_points[CUBE_N_POINTS];
float fov_factor = 128;

vec3_t camera_position = { .x = 0, .y = 0, .z = -5};

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
	for (float x = -1.0; x <= 1; x += 0.25) {
		for (float y = -1.0; y <= 1; y += 0.25) {
			for (float z = -1.0; z <= 1; z += 0.25) {
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

// Project a 3D point (our 3D space) into a 2D point (for the 2D screen).
vec2_t project(vec3_t point3d)
{
	// Simple perspective divide projection: 
	//     Divide by z, so things that are further away (larger z) are smaller.
	// Scale by fov_factor.
	vec2_t projected_point = {
		.x = (fov_factor * point3d.x) / point3d.z,
		.y = (fov_factor * point3d.y) / point3d.z
	};
	return projected_point;
}

void update(void)
{
	for (int ii = 0; ii < CUBE_N_POINTS; ii++) {
		vec3_t point3d = cube_points[ii];

        // Move the points away from the camera.
        point3d.z = point3d.z - camera_position.z;

        // Project the current 3D point into 2D space.
		vec2_t projected_point = project(point3d);
		// Save the projected 2D vector into the array of projected points.
		projected_points[ii] = projected_point;
	}
}

void render(void)
{
    draw_grid();

	// Render all projected points.
	for (int ii=0; ii < CUBE_N_POINTS; ii++) {
		// Draw each point as a small 4x4 yellow rectangle so we can see it.
		vec2_t projected_point = projected_points[ii];
		draw_rect(
			projected_point.x + (window_width / 2),  // translate to center of window
			projected_point.y + (window_height / 2), // translate to center of window
			4,
		    4, 
			0xFFFFFF00);
	}

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
