#pragma once

#include <stdint.h>
#include "gfx-vector.h"

// Struct for vertex index.
typedef struct {
    int a;
    int b;
    int c;
    uint32_t color;
} face_t; 

// Struct for projected points on the screen.
typedef struct {
    vec2_t points[3];
    uint32_t color;
    float avg_depth;
} triangle_t;

void draw_filled_triangle(int x0, int y0, int x1, int y1, int x2, int x3, uint32_t color);