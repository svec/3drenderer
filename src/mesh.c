#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gfx-vector.h"

#include "mesh.h"
#include "array.h"

mesh_t mesh = {
    .vertices = NULL,
    .faces = NULL,
    .rotation = {0, 0, 0}
};

// Using left hand coordinate system
//           +y  +z
//           ^  / 
//           | /
//           |/
//    -------*------> +x
//          /|
//         / |
//        /  |
//
/*
    
    p7: -1, 1, 1    --------------   p5: 1, 1, 1
                  /              /|
                 /              / |
                /              /  |
p2: -1, 1,-1   /--------------/   | p8: -1, -1, 1 in back left, can't see
               |  p3: 1, 1, -1|   |
               |              |   | p6: 1, -1, 1
               |              |  /
               |              | / 
p1: -1,-1,-1   |--------------|/  p4: 1, -1, -1

*/
// Note that vertex numbers are 1-indexed, not 0-indexed.
vec3_t cube_vertices[N_CUBE_VERTICES] = {
    {.x = -1, .y = -1, .z = -1}, // p1
    {.x = -1, .y =  1, .z = -1}, // p2
    {.x =  1, .y =  1, .z = -1}, // p3
    {.x =  1, .y = -1, .z = -1}, // p4
    {.x =  1, .y =  1, .z =  1}, // p5
    {.x =  1, .y = -1, .z =  1}, // p6
    {.x = -1, .y =  1, .z =  1}, // p7
    {.x = -1, .y = -1, .z =  1}, // p8
};

face_t cube_faces[N_CUBE_FACES] = {
    // front face
    {.a = 1, .b = 2, .c = 3},
    {.a = 1, .b = 3, .c = 4},

    // right face
    {.a = 4, .b = 3, .c = 5},
    {.a = 4, .b = 5, .c = 6},

    // back face
    {.a = 6, .b = 5, .c = 7},
    {.a = 6, .b = 7, .c = 8},

    // left face
    {.a = 8, .b = 7, .c = 2},
    {.a = 8, .b = 2, .c = 1},

    // top face
    {.a = 2, .b = 7, .c = 5},
    {.a = 2, .b = 5, .c = 3},

    // bottom face
    {.a = 6, .b = 8, .c = 1},
    {.a = 6, .b = 1, .c = 4},

};

void load_cube_mesh_data(void)
{
    for (int ii=0; ii < N_CUBE_VERTICES; ii++) {
        vec3_t cube_vertex = cube_vertices[ii];
        array_push(mesh.vertices, cube_vertex);
    }
    for (int ii=0; ii < N_CUBE_FACES; ii++) {
        face_t cube_face = cube_faces[ii];
        array_push(mesh.faces, cube_face);
    }
}

bool load_obj_file_data(char * filename)
{
    FILE * fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error opening obj file: %s\n", filename);
        return false;
    }

    /*
    We're looking for lines like this (starting at character 0):
        v -1.000000 -1.000000 1.000000

    or these:
        f 1/1/1 2/2/1 3/3/1
        f 1/1 2/2 3/3
        f 1 2 3
    
    Ignore all other lines.
    */
    bool all_good = true;
    int vertex_num = 0;
    int face_num = 0;
    int line_num = 0;
    char line[1024];

    while (all_good && fgets(line, 1024, fp)) {
        line_num++;

        if (strncmp(line, "v ", 2) == 0) {
            vertex_num++;
            vec3_t vertex;
            if (sscanf(line, "v %f %f %f", &vertex.x, &vertex.y, &vertex.z) != 3) {
                fprintf(stderr, "Error reading line %d, expected a vertex line\n", line_num);
                all_good = false;
                break;
            }
            array_push(mesh.vertices, vertex);
        } else if (strncmp(line, "f ", 2) == 0) {
            face_num++;
            int vertex_indices[3];
            int texture_indices[3];
            int normal_indices[3];
            if (sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d",
                       &vertex_indices[0], &texture_indices[0], &normal_indices[0], 
                       &vertex_indices[1], &texture_indices[1], &normal_indices[1], 
                       &vertex_indices[2], &texture_indices[2], &normal_indices[2]) != 9) {
                fprintf(stderr, "Error reading line %d, expected a face line\n", line_num);
                all_good = false;
                break;
            }
            for (int ii=0; ii < 3; ii++) {
                if (vertex_indices[ii] > vertex_num) { 
                    fprintf(stderr, "Error on line %d: face uses vertex %d, which is not defined.\n", line_num, vertex_indices[ii]);
                    all_good = false;
                    break;
                }
            }
            face_t face = {
                .a = vertex_indices[0],
                .b = vertex_indices[1],
                .c = vertex_indices[2]
            };
            array_push(mesh.faces, face);
        }
    }

    if (fp) {
        fclose(fp);
    }

    if (all_good) {
        printf("Found %d vertices and %d faces.\n", vertex_num, face_num);
    }

    return all_good;
}