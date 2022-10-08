#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

extern SDL_Window * window;
extern SDL_Renderer * renderer;

extern SDL_Texture * color_buffer_texture;
extern uint32_t * color_buffer;

extern int window_width;
extern int window_height;

bool initialize_window(void);
void set_color_buffer_pixel_color(int x, int y, uint32_t color);
void clear_color_buffer(uint32_t color);
void render_color_buffer(void);
void draw_grid(void);
void draw_rect(int rect_x, int rect_y, int width, int height, uint32_t color);
void destroy_window(void);