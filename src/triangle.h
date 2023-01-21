#pragma once

#include "gfx-vector.h"

// Struct for vertex index.
typedef struct {
    int a;
    int b;
    int c;
} face_t; 

// Struct for projected points on the screen.
typedef struct {
    vec2_t points[3];
} triangle_t;