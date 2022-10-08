#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

SDL_Window * window = NULL;
SDL_Renderer * renderer = NULL;

SDL_Texture * color_buffer_texture = NULL;
uint32_t * color_buffer = NULL;

bool is_running = false;
int window_width = 800;
int window_height = 600;


bool initialize_window(void)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		fprintf(stderr, "Error: SDL_Init() failed\n");
		return false;
	}

	// Query SDL for max fullscreen resolution.
	SDL_DisplayMode display_mode;
	SDL_GetCurrentDisplayMode(0, &display_mode);

	window_width = display_mode.w;
	window_height = display_mode.h;

	// Create SDL window at the center of the screen.
	window = SDL_CreateWindow(
		NULL,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		window_width,
		window_height,
		SDL_WINDOW_BORDERLESS
	);
	// TODO 
	if (! window) {
		fprintf(stderr, "Error: SDL_CreateWindow() failed\n");
		return false;
	}

	// Create SDL render
	renderer = SDL_CreateRenderer(window, -1, 0);
	if (! renderer) {
		fprintf(stderr, "Error: SDL_CreateRender failed\n");
		return false;
	}

	SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

    return true;
}

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

void set_color_buffer_pixel_color(int x, int y, uint32_t color)
{
	color_buffer[(window_width * y) + x] = color;
}

void clear_color_buffer(uint32_t color)
{
    for (int y = 0; y < window_height; y++) {
		for (int x = 0; x < window_width; x++) {
			set_color_buffer_pixel_color(x, y, color);
		}
	}
}

void render_color_buffer(void) {
    SDL_UpdateTexture(
		color_buffer_texture,
		NULL,
		color_buffer,
		window_width * sizeof(uint32_t)
	);
	SDL_RenderCopy(
		renderer,
		color_buffer_texture,
		NULL,
		NULL
	);
}

void draw_grid(void)
{
	const int x_incr = 10;
	const int y_incr = 10;
	const uint32_t color = 0xFF555555;

	for (int x=0; x < window_width; x += x_incr) {
		for (int y=0; y < window_height; y += y_incr) {
			set_color_buffer_pixel_color(x, y, color);
		}
	}
}

void draw_rect(int rect_x, int rect_y, int width, int height, uint32_t color)
{
    for (int x = rect_x; x < (rect_x + width); x++) {
		for (int y = rect_y; y < (rect_y + height); y++) {
			set_color_buffer_pixel_color(x, y, color);
		}
	}
}


void render(void)
{
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	SDL_RenderClear(renderer);

    draw_grid();

	draw_rect(100,200,300,400, 0xFFFF0000);

    render_color_buffer();

	clear_color_buffer(0xFF000000);

	SDL_RenderPresent(renderer);
}

void destroy_window(void)
{
	if (color_buffer) {
		free(color_buffer);
		color_buffer = NULL;
	}
	if (renderer) {
		SDL_DestroyRenderer(renderer);
	}
	if (window) {
		SDL_DestroyWindow(window);
    }

	SDL_Quit();
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
