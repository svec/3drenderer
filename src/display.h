#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#define FPS (60)
#define FRAME_TARGET_TIME_MS (1000 / FPS)

extern SDL_Window * window;
extern SDL_Renderer * renderer;

extern SDL_Texture * color_buffer_texture;
extern uint32_t * color_buffer;
extern float * z_buffer;

extern int window_width;
extern int window_height;

bool initialize_window(void);
void draw_pixel(int x, int y, uint32_t color);
void clear_color_buffer(uint32_t color);
void clear_z_buffer(void);
void render_color_buffer(void);
void draw_grid(void);
void draw_rect(int rect_x, int rect_y, int width, int height, uint32_t color);
void destroy_window(void);

void draw_line(int x0, int y0, int x1, int y1, uint32_t color);
void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);