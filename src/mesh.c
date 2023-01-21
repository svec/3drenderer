#include "gfx-vector.h"

#include "mesh.h"

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
vec3_t mesh_vertices[N_MESH_VERTICES] = {
    {.x = -1, .y = -1, .z = -1}, // p1
    {.x = -1, .y =  1, .z = -1}, // p2
    {.x =  1, .y =  1, .z = -1}, // p3
    {.x =  1, .y = -1, .z = -1}, // p4
    {.x =  1, .y =  1, .z =  1}, // p5
    {.x =  1, .y = -1, .z =  1}, // p6
    {.x = -1, .y =  1, .z =  1}, // p7
    {.x = -1, .y = -1, .z =  1}, // p8
};

face_t mesh_faces[N_MESH_FACES] = {
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