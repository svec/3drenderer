#pragma once

#include "gfx-vector.h"
#include "triangle.h"

// 2x2x2 cube has 8 vertices
#define N_CUBE_VERTICES (8)

extern vec3_t cube_vertices[N_CUBE_VERTICES];

// Each cube side has 2 triangles, 6 cube sides = 12 triangle faces
#define N_CUBE_FACES (6 * 2)
extern face_t cube_faces[N_CUBE_FACES];

typedef struct {
    vec3_t * vertices; // dynamic array of vertices for this mesh
    face_t * faces;    // dynamic array of faces for this mesh
} mesh_t;

extern mesh_t mesh;