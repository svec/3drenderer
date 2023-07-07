#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gfx-vector.h"

#include "mesh.h"
#include "array.h"

#define MAX_NUM_MESHES (10)
static mesh_t meshes[MAX_NUM_MESHES];
static int mesh_count = 0;

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

void free_meshes(void)
{
    for (int mesh_index = 0; mesh_index < mesh_count; mesh_index++) {
        array_free(meshes[mesh_index].faces);
        array_free(meshes[mesh_index].vertices);

        if (meshes[mesh_index].texture)
        {
            upng_free(meshes[mesh_index].texture);
        }
    }
}

int get_num_meshes(void)
{
    return mesh_count;
}

mesh_t * get_mesh(int index)
{
    mesh_t * ret = NULL;

    if ((index >= 0) && (index < mesh_count)) {
        ret =  &(meshes[index]);
    }

    return ret;
}

bool load_mesh_obj_data(mesh_t *mesh, char * obj_filename)
{
    FILE * fp = fopen(obj_filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error opening obj file: %s\n", obj_filename);
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
    int texture_num = 0;
    int face_num = 0;
    int line_num = 0;
    char line[1024];

    tex2_t * texcoords = NULL;

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
            array_push(mesh->vertices, vertex);
        } else if (strncmp(line, "vt ", 3) == 0) {
            // Texture coordinate info.
            texture_num++;
            tex2_t texcoord;
            if (sscanf(line, "vt  %f %f", &texcoord.u, &texcoord.v) != 2) {
                fprintf(stderr, "Error reading line %d, expected a texture coordinate line\n", line_num);
                all_good = false;
                break;
            }
            array_push(texcoords, texcoord); // stopped at 14:55
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
                if (texture_indices[ii] > texture_num) {
                    fprintf(stderr, "Error on line %d: face uses texture index %d, which is not defined.\n", line_num, texture_indices[ii]);
                    all_good = false;
                    break;
                }
            }
            if (! all_good) {
                break;
            }
            face_t face = {
                // Vertices and texture coordinates are 1-indexed, not 0-indexed, so we subtract 1.
                .a = vertex_indices[0] - 1,
                .b = vertex_indices[1] - 1,
                .c = vertex_indices[2] - 1,

                .a_uv = texcoords[texture_indices[0] - 1],
                .b_uv = texcoords[texture_indices[1] - 1],
                .c_uv = texcoords[texture_indices[2] - 1],
                .color = 0xFFFFFFFF
            };
            array_push(mesh->faces, face);
        }
    }

    array_free(texcoords);

    if (fp) {
        fclose(fp);
    }

    if (all_good) {
        printf("Found %d vertices, %d faces, and %d texture coords.\n", vertex_num, face_num, texture_num);
    }

    return all_good;
}

bool load_mesh_png_data(mesh_t * mesh, char * png_filename)
{
    bool all_good = false;

    upng_t * png_image = upng_new_from_file(png_filename);

    if (png_image != NULL) {
        upng_decode(png_image);
        upng_error error = upng_get_error(png_image);
        fprintf(stderr, "upng_get_error returned: %d\n", error);
        if (error == UPNG_EOK) {
            mesh->texture = png_image;
            all_good = true;
        }
    }

    return all_good;
}

bool load_mesh(char * obj_filename, char * png_texture_filename,
               vec3_t scale, vec3_t translation, vec3_t rotation)
{
    if (mesh_count >= MAX_NUM_MESHES) {
        fprintf(stderr, "ERROR: too many meshes, can only have %d\n", MAX_NUM_MESHES);
        return false;
    }

    mesh_t * new_mesh = &(meshes[mesh_count]);

    bool all_good = load_mesh_obj_data(new_mesh, obj_filename);
    if (! all_good) {
        fprintf(stderr, "Error: load_mesh_obj_data failed on filename: %s\n", obj_filename);
        return false;
    }

    all_good = load_mesh_png_data(new_mesh, png_texture_filename);

    if (! all_good) {
        fprintf(stderr, "Error: load_mesh_png_data failed on filename: %s\n", png_texture_filename);
        return false;
    }

    new_mesh->scale = scale;
    new_mesh->translation = translation;
    new_mesh->rotation = rotation;

    mesh_count++;

    return true;
}