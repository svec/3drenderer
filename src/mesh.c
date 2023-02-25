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

    size_t max_line_length = 64;
    char * line_p = (char *)malloc(sizeof(char) * max_line_length);
    if (line_p == NULL) {
        fprintf(stderr, "Error: malloc failed\n");
        if (fp) {
            free(fp);
        }
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
    int vertex_num = 1;
    int face_num = 1;
    int line_num = 0;
    while (all_good) {
        line_num++;

        size_t ret = getline(&line_p, &max_line_length, fp);
        //printf("ret: %zu\n", ret);
        if (ret == (size_t)-1) {
            break;
        }
        printf("  max_line_length: %zu line:#%s#\n", max_line_length, line_p);

        if (ret < 3) {
            continue;
        }

        if (    ((line_p[0] != 'v') && (line_p[0] != 'f'))
             || (line_p[1] != ' ')) {
            continue;
        }

        // Line is either a vertex or a face.
        bool line_type_vertex = (line_p[0] == 'v');

        int data_count = 0;
        float new_vertex_points[3];
        int new_face_points[3];

        // Skip past the first two characters.
        char * tokenizing_string = line_p + 2;

        for (char *p = strtok(tokenizing_string, " "); p != NULL; p = strtok(NULL, " ")) {
            if (data_count < 3) {
                if (line_type_vertex) {
                    float f = strtof(p, NULL);
                    new_vertex_points[data_count] = f;
                } else {
                    int i = strtoul(p, NULL, 10);

                    if (i > vertex_num) {
                        fprintf(stderr, "Error on line %d: face uses vertex %d, which is not defined.\n", line_num, i);
                        all_good = false;
                        break;
                    }
                    new_face_points[data_count] = i;
                }
                data_count++;
            }
        }

        if (all_good && (data_count == 3)) {
            if (line_type_vertex) {
                vec3_t v = {.x = new_vertex_points[0], .y = new_vertex_points[1], .z = new_vertex_points[2]};
                array_push(mesh.vertices, v);
                printf("vertex # %d: %f %f %f\n", vertex_num, v.x, v.y, v.z);
                vertex_num++;
            } else {
                face_t f = {.a = new_face_points[0], .b = new_face_points[1], .c = new_face_points[2]};
                array_push(mesh.faces, f);
                printf("face # %d: %d %d %d\n", face_num, f.a, f.b, f.c);
                face_num++;
            }
        } else {
            fprintf(stderr, "Error: line %d did not look valid.\n", line_num);
            all_good = false;
        }
    }

    if (line_p) {
        free(line_p);
    }

    if (fp) {
        fclose(fp);
    }

    if (all_good) {
        printf("Found %d vertices and %d faces.\n", vertex_num, face_num);
    }

    return all_good;
}