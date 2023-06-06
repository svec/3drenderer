#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "display.h"
#include "gfx-vector.h"
#include "mesh.h"
#include "triangle.h"
#include "array.h"
#include "matrix.h"
#include "light.h"
#include "texture.h"

int previous_frame_time = 0;
bool g_display_back_face_culling = true;
bool g_display_vertex_dot = true;
bool g_display_wireframe_lines = true;
bool g_display_filled_trianges = false;
bool g_display_texture = true;

triangle_t *triangles_to_render = NULL;

vec3_t camera_position = { .x = 0, .y = 0, .z = 0};

mat4_t proj_matrix;

bool is_running = false;

bool setup(void)
{
    color_buffer = (uint32_t *)malloc(window_width * window_height * sizeof(uint32_t));
    
    if (!color_buffer) {
        fprintf(stderr, "Error: malloc failed for color_buffer.\n");
        return false;
    }

    // Create a texture buffer that displays the color buffer.
    color_buffer_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        window_width,
        window_height
    );

    if (! color_buffer_texture) {
        fprintf(stderr, "Error: SDL_CreateTexture failed\n");
        return false;
    }

    // Initialize the perspective projection matrix.
    float fov = M_PI/3.0; // 60 degrees = 180/3, = PI/3 in radians
    float aspect = (float)window_height / (float)window_width;
    float znear = 0.1;
    float zfar = 100.0;
    proj_matrix = mat4_make_projection(fov, aspect, znear, zfar);

    // Manually load the hardcoded texture data from the static array.
    // The REDBRICK_TEXTURE array is uint8_t's, cast it to uint32_t.
    mesh_texture = (uint32_t *)REDBRICK_TEXTURE;
    texture_width = 64;
    texture_height = 64;
    
    load_cube_mesh_data();
    //load_obj_file_data("./assets/cube.obj");
    //load_obj_file_data("./assets/f22.obj");

    return true;
}

void process_input(void)
{
    SDL_Event event;
    SDL_PollEvent(&event);

    switch (event.type) {
        case SDL_QUIT:  // User clicks window close button
            is_running = false;
            break;
        case SDL_KEYDOWN:
            /*
                Pressing “1” displays the wireframe and a small red dot for each triangle vertex
                Pressing “2” displays only the wireframe lines
                Pressing “3” displays filled triangles with a solid color
                Pressing “4” displays both filled triangles and wireframe lines
                Pressing “5” displays textured triangles
                Pressing “6” displays textured triangles and wireframe lines
                Pressing “c” we should enable back-face culling
                Pressing “d” we should disable the back-face culling
                */
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                is_running = false;
            }
            if (event.key.keysym.sym == SDLK_1) {
                g_display_vertex_dot = true;
                g_display_wireframe_lines = true;
            }
            if (event.key.keysym.sym == SDLK_2) {
                g_display_vertex_dot = false;
                g_display_wireframe_lines = true;
                g_display_filled_trianges = false;
            }
            if (event.key.keysym.sym == SDLK_3) {
                g_display_vertex_dot = false;
                g_display_wireframe_lines = false;
                g_display_filled_trianges = true;
                g_display_texture = false;
            }
            if (event.key.keysym.sym == SDLK_4) {
                g_display_vertex_dot = false;
                g_display_wireframe_lines = true;
                g_display_filled_trianges = true;
                g_display_texture = false;
            }
            if (event.key.keysym.sym == SDLK_5) {
                g_display_vertex_dot = false;
                g_display_wireframe_lines = false;
                g_display_filled_trianges = false;
                g_display_texture = true;
            }
            if (event.key.keysym.sym == SDLK_6) {
                g_display_vertex_dot = false;
                g_display_wireframe_lines = true;
                g_display_filled_trianges = false;
                g_display_texture = true;
            }
            if (event.key.keysym.sym == SDLK_c) {
                g_display_back_face_culling = true;
            }
            if (event.key.keysym.sym == SDLK_d) {
                g_display_back_face_culling = false;
            }
            break;
        default:
            break;

    }
}

int compare_triangles(const void * a, const void * b)
{
    triangle_t * p1 = (triangle_t *)a;
    triangle_t * p2 = (triangle_t *)b;

    if (p1->avg_depth < p2->avg_depth) {
        return 1;
    } else if (p1->avg_depth > p2->avg_depth) {
        return -1;
    }
    return 0;
}

void update(void)
{
    // Do we need to delay before updating the frame?
    int time_to_wait_ms = FRAME_TARGET_TIME_MS - (SDL_GetTicks() - previous_frame_time);

    if ((time_to_wait_ms > 0) && (time_to_wait_ms <= FRAME_TARGET_TIME_MS)) {
        // Delay until it's time for the next frame.
        SDL_Delay(time_to_wait_ms);
    }

    // How many ms have passed since we last were called?
    previous_frame_time = SDL_GetTicks();

    // Initialize the array of triangles to render.
    triangles_to_render = NULL;

    // Rotate the cube by a little bit in the y direction each frame.
    mesh.rotation.x += 0.01;
    mesh.rotation.y += 0.01;
    mesh.rotation.z += 0.01;

    //mesh.scale.x += 0.0002;
    //mesh.scale.y += 0.0001;
    //mesh.scale.z += 0.0003;

    //mesh.translation.x += 0.001;
    //mesh.translation.y += 0.002;


    // Translate the vertex away from the camera.
    mesh.translation.z = 5.0; 

    // Create scale, translation, and rotation matrices that will be used to multiply the mesh vertices.
    mat4_t scale_matrix = mat4_make_scale(mesh.scale.x, mesh.scale.y, mesh.scale.z);
    mat4_t translation_matrix = mat4_make_translation(mesh.translation.x, mesh.translation.y, mesh.translation.z);
    mat4_t rotation_matrix_x = mat4_make_rotation_x(mesh.rotation.x);
    mat4_t rotation_matrix_y = mat4_make_rotation_y(mesh.rotation.y);
    mat4_t rotation_matrix_z = mat4_make_rotation_z(mesh.rotation.z);

    // Loop all triangle faces.
    int num_faces = array_length(mesh.faces);

    for (int face_i=0; face_i < num_faces; face_i++) {
        // Handle 1 triangle face per iteration.

        face_t mesh_face = mesh.faces[face_i];
        vec3_t face_vertices[3];
        face_vertices[0] = mesh.vertices[mesh_face.a - 1]; // -1 because vertices are 1-indexed
        face_vertices[1] = mesh.vertices[mesh_face.b - 1]; // -1 because vertices are 1-indexed
        face_vertices[2] = mesh.vertices[mesh_face.c - 1]; // -1 because vertices are 1-indexed

        vec4_t transformed_vertices[3];

        // For all 3 vertices of this triangle, apply transformations.
        for (int vertex_i = 0; vertex_i < 3; vertex_i++)
        {
            vec4_t transformed_vertex = vec4_from_vec3(face_vertices[vertex_i]);

            // Creating a single World Matrix combining the scale, rotation, and translation matrices.
            // Note that the order matters: Must be scale first, then rotation, and finally translation last.
            mat4_t world_matrix = mat4_identity();
            world_matrix = mat4_mul_mat4(scale_matrix, world_matrix);
            world_matrix = mat4_mul_mat4(rotation_matrix_x, world_matrix);
            world_matrix = mat4_mul_mat4(rotation_matrix_y, world_matrix);
            world_matrix = mat4_mul_mat4(rotation_matrix_z, world_matrix);
            world_matrix = mat4_mul_mat4(translation_matrix, world_matrix);

            // Multiply (apply) the World Matrix by the vertex to get the transformed vertex.
            transformed_vertex = mat4_mul_vec4(world_matrix, transformed_vertex);

            // Save off the transformed vertex.
            transformed_vertices[vertex_i] = transformed_vertex;
        }

        // Backface culling.
        // Remember that triangles are "clockwise", going A-B-C.
        // Also remember that we use a left-handed axis system, so z gets larger
        // going "into" the screen away from the viewer.
        vec3_t vector_a = vec3_from_vec4(transformed_vertices[0]);   /*     A     */
        vec3_t vector_b = vec3_from_vec4(transformed_vertices[1]);   /*    / \    */
        vec3_t vector_c = vec3_from_vec4(transformed_vertices[2]);   /*   C---B   */

        vec3_t vector_ab = vec3_sub(vector_b, vector_a); // Vector AB
        vec3_t vector_ac = vec3_sub(vector_c, vector_a); // Vector AC
        vec3_normalize(&vector_ab);
        vec3_normalize(&vector_ac);

        // Compute the face normal using the cross product to find a perpendicular line to the face.
        vec3_t normal = vec3_cross(vector_ab, vector_ac);
        vec3_normalize(&normal);

        // Find the vector between a point in the triangle (point A) and the camera origin.
        vec3_t camera_ray = vec3_sub(camera_position, vector_a);

        // How aligned is the face's normal with the camera ray?
        float dot_normal_camera = vec3_dot(normal, camera_ray);

        if (g_display_back_face_culling && (dot_normal_camera < 0)) {
            // If the dot product is < 0, then the face is pointing away from the camera, 
            // and we don't need to display it.
            continue;
        }

        vec4_t projected_points[3];

        for (int vertex_i = 0; vertex_i < 3; vertex_i++) {
            // Project the current vertex.
            projected_points[vertex_i] = mat4_mul_vec4_project(proj_matrix, transformed_vertices[vertex_i]);

            // Scale into the view.
            projected_points[vertex_i].x *= (window_width / 2.0);
            projected_points[vertex_i].y *= (window_height / 2.0);

            // Invert the y values to account for flipped screen y coordinate: in our object coordinate system,
            // y increases going "up" the screen, but in SDL the y increases going "down" the screen.
            projected_points[vertex_i].y *= -1.0;

            // Translate the projected points to the middle of the screen.
            projected_points[vertex_i].x += (window_width / 2.0);  // translate to center of window
            projected_points[vertex_i].y += (window_height / 2.0); // translate to center of window
        }

        // Calculate the average z depth for each triangle now that all transformations are done.
        // Using a very naive definition of triangle depth: just average all 3 z points.
        float avg_depth = (transformed_vertices[0].z + transformed_vertices[1].z + transformed_vertices[2].z) / 3.0;

        // Calculate the triangle color based on the original triangle color and the angle of the light 
        // on the triangle.
        // We compare the light ray's direction and the triangle face normal vector to see how aligned the triangle is 
        // with the light. The more aligned the triangle face is with the light, the more the light will
        // brighten the triangle.
        // We use the negative of the dot product because we actually care about the opposite of the light
        // direction: we want max light if the normal is pointed directly opposite the light direction.
        // Using the negative of the dot product does this.
        float light_intensity_factor = -vec3_dot(normal, light.direction);
        uint32_t triangle_color = light_apply_intensity(mesh_face.color, light_intensity_factor);
        //uint32_t triangle_color = mesh_face.color;

        triangle_t projected_triangle = {
            .points = {
                {projected_points[0].x, projected_points[0].y},
                {projected_points[1].x, projected_points[1].y},
                {projected_points[2].x, projected_points[2].y},
            },
            .texcoords = {
                {mesh_face.a_uv.u, mesh_face.a_uv.v},
                {mesh_face.b_uv.u, mesh_face.b_uv.v},
                {mesh_face.c_uv.u, mesh_face.c_uv.v},
            },
            .color = triangle_color,
            .avg_depth = avg_depth
        };

        // Saves the projected triangle to the the array of triangles to render.
        array_push(triangles_to_render, projected_triangle);
    }

    // Sort the triangles_to_render by average depth so we can use the Painters Algorithm
    // to draw the furthest away triangles before nearer triangles.
    qsort(triangles_to_render, array_length(triangles_to_render), sizeof(triangle_t), compare_triangles);
}

void render(void)
{
    draw_grid();

    // Loop all projected triangles and render them.

    // earliest example:
    //draw_filled_triangle(300, 100, 50, 400, 500, 700, 0xFF00FF00);

    // triangles_to_render is already sorted from back to front.
    int num_triangles = array_length(triangles_to_render);

    for (int ii=0; ii < num_triangles; ii++) {
        triangle_t triangle = triangles_to_render[ii];

        if (g_display_filled_trianges) {
            draw_filled_triangle(
                triangle.points[0].x,
                triangle.points[0].y,
                triangle.points[1].x,
                triangle.points[1].y,
                triangle.points[2].x,
                triangle.points[2].y,
                triangle.color);
        }

        if (g_display_texture) {
            draw_textured_triangle(
                triangle.points[0].x,
                triangle.points[0].y,
                triangle.texcoords[0].u,
                triangle.texcoords[0].v,
                triangle.points[1].x,
                triangle.points[1].y,
                triangle.texcoords[1].u,
                triangle.texcoords[1].v,
                triangle.points[2].x,
                triangle.points[2].y,
                triangle.texcoords[2].u,
                triangle.texcoords[2].v,
                mesh_texture);
        }

        if (g_display_wireframe_lines) {
            // Draw the outlines for the triangle.
            draw_triangle(
                triangle.points[0].x,
                triangle.points[0].y,
                triangle.points[1].x,
                triangle.points[1].y,
                triangle.points[2].x,
                triangle.points[2].y,
                0xFF0000FF);
        }

        if (g_display_vertex_dot) {
            // Draw each point as a small 4x4 yellow rectangle so we can see it.
            draw_rect(triangle.points[0].x, triangle.points[0].y, 3, 3, 0xFFFF0000);
            draw_rect(triangle.points[1].x, triangle.points[1].y, 3, 3, 0xFFFF0000);
            draw_rect(triangle.points[2].x, triangle.points[2].y, 3, 3, 0xFFFF0000);
        }
    }

    // Now that we've rendered the triagles into the frame buffer, 
    // clear the triangle array.
    array_free(triangles_to_render);

    render_color_buffer();

    clear_color_buffer(0xFF000000);

    SDL_RenderPresent(renderer);
}

void free_resources(void)
{
    array_free(mesh.faces);
    array_free(mesh.vertices);

    if (color_buffer) {
        free(color_buffer);
        color_buffer = NULL;
    }
}

int main(void) {

    is_running = initialize_window();

    if (! setup()) {
        // If setup failed, don't run.
        printf("ERROR: setup failed.\n");
        is_running = false;
    }

    //is_running = false; // svechack

    while (is_running) {
        process_input();
        update();
        render();
    }

    destroy_window();
    free_resources();

    return 0;
}
