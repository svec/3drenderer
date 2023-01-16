#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "display.h"
#include "gfx-vector.h"

float fov_factor = 640;

int previous_frame_time = 0;

vec3_t camera_position = { .x = 0, .y = 0, .z = -5};
vec3_t cube_rotation = {0, 0, 0};

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
	// Do we need to delay before updating the frame?
	int time_to_wait_ms = FRAME_TARGET_TIME_MS - (SDL_GetTicks() - previous_frame_time);

    if ((time_to_wait_ms > 0) && (time_to_wait_ms <= FRAME_TARGET_TIME_MS)) {
		// Delay until it's time for the next frame.
		SDL_Delay(time_to_wait_ms);
	}

    // How many ms have passed since we last were called?
    previous_frame_time = SDL_GetTicks();

    // Rotate the cube by a little bit in the y direction each frame.
	cube_rotation.x += 0.01;
	cube_rotation.y += 0.01;
	cube_rotation.z += 0.01;

/*
	for (int ii = 0; ii < CUBE_N_POINTS; ii++) {
		vec3_t point3d = cube_points[ii];

        // Transform the point around the y axis.
		vec3_t transformed_point3d = vec3_rotate_x(point3d, cube_rotation.x);
		transformed_point3d = vec3_rotate_y(transformed_point3d, cube_rotation.y);
		transformed_point3d = vec3_rotate_z(transformed_point3d, cube_rotation.z);

        // Translate the point away from the camera.
        transformed_point3d.z -= camera_position.z;

        // Project the current 3D point into 2D space.
		vec2_t projected_point = project(transformed_point3d);
		// Save the projected 2D vector into the array of projected points.
		projected_points[ii] = projected_point;
	}
*/
}

void render(void)
{
    draw_grid();

/*
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
*/

    render_color_buffer();

	clear_color_buffer(0xFF000000);

	SDL_RenderPresent(renderer);
}

int main(void) {

	is_running = initialize_window();

	if (! setup()) {
		// If setup failed, don't run.
		printf("ERROR: setup failed.\n");
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
