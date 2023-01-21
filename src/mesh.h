#pragma once

#include "gfx-vector.h"
#include "triangle.h"

// 2x2x2 cube has 8 vertices
#define N_MESH_VERTICES (8)

extern vec3_t mesh_vertices[N_MESH_VERTICES];

// Each cube side has 2 triangles, 6 cube sides = 12 triangle faces
#define N_MESH_FACES (6 * 2)
extern face_t mesh_faces[N_MESH_FACES];