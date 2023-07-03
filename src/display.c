#include "display.h"

static SDL_Window * window = NULL;
static SDL_Renderer * renderer = NULL;

static SDL_Texture * color_buffer_texture = NULL;
static uint32_t * color_buffer = NULL;

static float * z_buffer = NULL;

static int window_width = 800;
static int window_height = 600;

static bool use_fullscreen = false;

int get_window_width(void)
{
    return window_width;
}

int get_window_height(void)
{
    return window_height;
}

bool initialize_window(void)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Error: SDL_Init() failed\n");
        return false;
    }

    // Query SDL for max fullscreen resolution.
    SDL_DisplayMode display_mode;
    SDL_GetCurrentDisplayMode(0, &display_mode);

    // Note: I skipped the "Simulating Low-Resolution Displays" lecture because
    // doing those changes created a bunch of errors like:
    //   ERROR: z_buffer_index is too big: 2234303, used x:1727 y:1292
    // which I didn't want to track down now.
    window_width = display_mode.w;
    window_height = display_mode.h;

    if (! use_fullscreen) {
        window_width = 800;
        window_height = 600;
    }

    // Create SDL window at the center of the screen.
    window = SDL_CreateWindow(
        NULL,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        window_width,
        window_height,
        SDL_WINDOW_BORDERLESS
    );
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

    if (use_fullscreen) {
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
    }

    color_buffer = (uint32_t *)malloc(window_width * window_height * sizeof(uint32_t));
    
    if (!color_buffer) {
        fprintf(stderr, "Error: malloc failed for color_buffer.\n");
        return false;
    }

    z_buffer = (float *)malloc(window_width * window_height * sizeof(float));

    if (!z_buffer) {
        fprintf(stderr, "Error: malloc failed for z_buffer.\n");
        return false;
    }

    // Create a texture buffer that displays the color buffer.
    color_buffer_texture = SDL_CreateTexture(
        renderer,
        //SDL_PIXELFORMAT_ARGB8888,
        SDL_PIXELFORMAT_RGBA32,
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

void draw_pixel(int x, int y, uint32_t color)
{
    if (    (x >= 0) && (x < window_width)
         && (y >= 0) && (y < window_height)) {
        color_buffer[(window_width * y) + x] = color;
    }
}

void clear_color_buffer(uint32_t color)
{
    for (int y = 0; y < window_height; y++) {
        for (int x = 0; x < window_width; x++) {
            draw_pixel(x, y, color);
        }
    }
}

void clear_z_buffer(void)
{
    // Note that we clear the z buffer by setting the values to 1.0.
    // Since we use 1/w (the inverted depth value) instead of the non-inverted
    // depth (because 1/w is linear, but w is not), 1.0 is maximum depth,
    // not 0.0.
    for (int y = 0; y < window_height; y++) {
        for (int x = 0; x < window_width; x++) {
            z_buffer[(y * window_width) + x] = 1.0;
        }
    }
}

float get_zbuffer_at(int x, int y)
{
    float value = 1.0;

    if (    (x >= 0) && (x < window_width)
         && (y >= 0) && (y < window_height)) {
            value = z_buffer[(window_width * y) + x];
    }
    return value;
}

void update_zbuffer_at(int x, int y, float value)
{

    if (    (x >= 0) && (x < window_width)
         && (y >= 0) && (y < window_height)) {
            z_buffer[(window_width * y) + x] = value;
    }
}

void render_color_buffer(void)
{
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

    SDL_RenderPresent(renderer);
}

void draw_grid(void)
{
    const int x_incr = 10;
    const int y_incr = 10;
    const uint32_t color = 0xFF555555;

    for (int x=0; x < window_width; x += x_incr) {
        for (int y=0; y < window_height; y += y_incr) {
            draw_pixel(x, y, color);
        }
    }
}

void draw_rect(int rect_x, int rect_y, int width, int height, uint32_t color)
{
    for (int x = rect_x; x < (rect_x + width); x++) {
        for (int y = rect_y; y < (rect_y + height); y++) {
            draw_pixel(x, y, color);
        }
    }
}

void destroy_window(void)
{
    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }
    if (window) {
        SDL_DestroyWindow(window);
    }

    if (color_buffer) {
        free(color_buffer);
        color_buffer = NULL;
    }

    if (z_buffer) {
        free(z_buffer);
        z_buffer = NULL;
    }

    SDL_Quit();
}


void draw_line(int x0, int y0, int x1, int y1, uint32_t color)
{
    // Draw a line between two points using the DDA algorithm.
    int delta_x = x1 - x0;
    int delta_y = y1 - y0;

    // Figure out the longest dimension of line: x or y?
    int longest_side_length = (abs(delta_x) > abs(delta_y)) ? abs(delta_x) : abs(delta_y);

    float inc_x = delta_x / (float)longest_side_length;
    float inc_y = delta_y / (float)longest_side_length;

    float current_x = x0;
    float current_y = y0;
    for (int ii=0; ii <= longest_side_length; ii++) {
        draw_pixel(round(current_x), round(current_y), color);
        current_x += inc_x;
        current_y += inc_y;
    }
}

void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
    draw_line(x0, y0, x1, y1, color);
    draw_line(x1, y1, x2, y2, color);
    draw_line(x2, y2, x0, y0, color);
}
