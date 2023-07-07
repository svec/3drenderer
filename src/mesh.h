#pragma once

#include <stdbool.h>

#include "gfx-vector.h"
#include "triangle.h"
#include "upng.h"

// This struct is a mesh, with dynamically sized vertices and faces,
// as well as the rotation of this mesh.
typedef struct {
    vec3_t * vertices;   // dynamic array of vertices for this mesh
    face_t * faces;      // dynamic array of faces for this mesh
    upng_t * texture;    // PNG texture pointer
    vec3_t rotation;     // rotation of this mesh with x, y, z
    vec3_t scale;        // scale with x, y, z
    vec3_t translation;  // translation with x, y, z
} mesh_t;

void free_meshes(void);
int get_num_meshes(void);
mesh_t * get_mesh(int index);

bool load_mesh_obj_data(mesh_t * mesh, char * obj_filename);
bool load_mesh_png_data(mesh_t * mesh, char * obj_filename);

bool load_mesh(char * obj_filename, char * png_texture_filename,
               vec3_t scale, vec3_t translation, vec3_t rotation);
