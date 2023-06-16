#pragma once

#include <stdbool.h>
#include <math.h>
#include "gfx-vector.h"
#include "triangle.h"

// 10 is a hacky limit, but an effective one.
#define MAX_NUM_POLY_VERTICES (10)
#define MAX_NUM_POLY_TRIANGLES (10)

enum {
    LEFT_FRUSTUM_PLANE,
    RIGHT_FRUSTUM_PLANE,
    TOP_FRUSTUM_PLANE,
    BOTTOM_FRUSTUM_PLANE,
    NEAR_FRUSTUM_PLANE,
    FAR_FRUSTUM_PLANE,
};

typedef struct {
    vec3_t point;
    vec3_t normal;
} plane_t;

typedef struct {
    vec3_t vertices[MAX_NUM_POLY_VERTICES];
    int num_vertices;
} polygon_t;

void init_frustum_planes(float fov_x, float fov_y, float z_near, float z_far);

polygon_t create_polygon_from_triangle(vec3_t v0, vec3_t v1, vec3_t v2);
bool clip_polygon(polygon_t *polygon);
bool triangles_from_polygon(polygon_t *polygon, triangle_t triangles[], int *num_triangles);