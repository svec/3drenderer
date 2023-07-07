#pragma once

#include <stdint.h>
#include "gfx-vector.h"
#include "texture.h"
#include "upng.h"

// Struct for vertex index.
typedef struct {
    int a;
    int b;
    int c;
    tex2_t a_uv;
    tex2_t b_uv;
    tex2_t c_uv;
    uint32_t color;
} face_t; 

// Struct for projected points on the screen.
typedef struct {
    vec4_t points[3];
    tex2_t texcoords[3]; // UV texture coordinates
    uint32_t color;
    upng_t * texture;
} triangle_t;

vec3_t get_triangle_normal(vec4_t vertices[3]);

void draw_filled_triangle(int x0, int y0, float z0, float w0,
                          int x1, int y1, float z1, float w1,
                          int x2, int x3, float z2, float w2,
                          uint32_t color);

void draw_textured_triangle(int x0, int y0, float z0, float w0, float u0, float v0, 
                            int x1, int y1, float z1, float w1, float u1, float v1,
                            int x2, int y2, float z2, float w2, float u2, float v2,
                            upng_t * texture);

void draw_texel(int x, int y, upng_t * texture,
                vec4_t point_a, vec4_t point_b, vec4_t point_c,
                tex2_t a_uv, tex2_t b_uv, tex2_t c_uv);