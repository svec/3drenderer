#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#define FPS (60)
#define FRAME_TARGET_TIME_MS (1000 / FPS)

bool initialize_window(void);
int get_window_width(void);
int get_window_height(void);
void draw_pixel(int x, int y, uint32_t color);
void clear_color_buffer(uint32_t color);
void clear_z_buffer(void);

float get_zbuffer_at(int x, int y);
void update_zbuffer_at(int x, int y, float value);

void render_color_buffer(void);
void draw_grid(void);
void draw_rect(int rect_x, int rect_y, int width, int height, uint32_t color);
void destroy_window(void);

void draw_line(int x0, int y0, int x1, int y1, uint32_t color);
void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);