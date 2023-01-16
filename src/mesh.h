#pragma once

#include "gfx-vector.h"

// 2x2x2 cube
#define N_MESH_VERTICES (8)

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