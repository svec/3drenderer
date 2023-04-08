#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "display.h"
#include "gfx-vector.h"
#include "mesh.h"
#include "triangle.h"
#include "array.h"

float fov_factor = 640;

int previous_frame_time = 0;
bool g_display_back_face_culling = true;
bool g_display_vertex_dot = true;
bool g_display_wireframe_lines = true;
bool g_display_filled_trianges = true;

triangle_t *triangles_to_render = NULL;

vec3_t camera_position = { .x = 0, .y = 0, .z = 0};

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

    load_cube_mesh_data();
#if 0
    bool success = load_obj_file_data("assets/cube.obj");
    //bool success = load_obj_file_data("assets/f22.obj");
    if (success == false) {
        fprintf(stderr, "Error: loading graphics obj file failed.\n");
        return false;
    }
#endif

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
            }
            if (event.key.keysym.sym == SDLK_4) {
                g_display_vertex_dot = false;
                g_display_wireframe_lines = true;
                g_display_filled_trianges = true;
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

// Project a 3D point (our 3D space) into a 2D point (for the 2D screen).
vec2_t project(vec3_t point3d)
{
    // Simple perspective divide projection: 
    //     Divide by z, so things that are further away (larger z) are smaller.
    // Scale by fov_factor.
    vec2_t projected_point = {
        .x = (fov_factor * point3d.x) / point3d.z,
        .y = (fov_factor * point3d.y) / point3d.z
    };
    return projected_point;
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

    // Loop all triangle faces.
    int num_faces = array_length(mesh.faces);

    for (int face_i=0; face_i < num_faces; face_i++) {
        // Handle 1 triangle face per iteration.

        face_t mesh_face = mesh.faces[face_i];
        vec3_t face_vertices[3];
        face_vertices[0] = mesh.vertices[mesh_face.a - 1]; // -1 because vertices are 1-indexed
        face_vertices[1] = mesh.vertices[mesh_face.b - 1]; // -1 because vertices are 1-indexed
        face_vertices[2] = mesh.vertices[mesh_face.c - 1]; // -1 because vertices are 1-indexed

        vec3_t transformed_vertices[3];

        // For all 3 vertices of this triangle, apply transformations.
        for (int vertex_i = 0; vertex_i < 3; vertex_i++)
        {
            // Rotate each vertex.
            vec3_t transformed_vertex = face_vertices[vertex_i];

            transformed_vertex = vec3_rotate_x(transformed_vertex, mesh.rotation.x);
            transformed_vertex = vec3_rotate_y(transformed_vertex, mesh.rotation.y);
            transformed_vertex = vec3_rotate_z(transformed_vertex, mesh.rotation.z);

            // Translate the vertex away from the camera.
            transformed_vertex.z += 5; 

            // Save off the transformed vertex.
            transformed_vertices[vertex_i] = transformed_vertex;
        }

        // Backface culling.
        // Remember that triangles are "clockwise", going A-B-C.
        // Also remember that we use a left-handed axis system, so z gets larger
        // going "into" the screen away from the viewer.
        vec3_t vector_a = transformed_vertices[0];   /*     A     */
        vec3_t vector_b = transformed_vertices[1];   /*    / \    */
        vec3_t vector_c = transformed_vertices[2];   /*   C---B   */

        vec3_t vector_ab = vec3_sub(vector_b, vector_a); // Vector AB
        vec3_t vector_ac = vec3_sub(vector_c, vector_a); // Vector AC
        vec3_normalize(&vector_ab);
        vec3_normalize(&vector_ac);

        // Compute the face normal using the cross product to find a perpencicular line to the face.
        vec3_t normal = vec3_cross(vector_ab, vector_ac);
        vec3_normalize(&normal);

        // Find the vector between a point in the triangle (point A) and the camera origin.
        vec3_t camera_ray = vec3_sub(camera_position, vector_a);

        // How aligned is the face's normal with the camera ray?
        float dot_normal_camera = vec3_dot(camera_ray, normal);

        if (g_display_back_face_culling && (dot_normal_camera < 0)) {
            // If the dot product is < 0, then the face is pointing away from the camera, 
            // and we don't need to display it.
            continue;
        }

        vec2_t projected_points[3];

        for (int vertex_i = 0; vertex_i < 3; vertex_i++) {
            // Project the current vertex.
            projected_points[vertex_i] = project(transformed_vertices[vertex_i]);

            projected_points[vertex_i].x += (window_width / 2);  // translate to center of window
            projected_points[vertex_i].y += (window_height / 2); // translate to center of window

        }

        triangle_t projected_triangle = {
            .points = {
                {projected_points[0].x, projected_points[0].y},
                {projected_points[1].x, projected_points[1].y},
                {projected_points[2].x, projected_points[2].y},
            },
            .color = mesh_face.color
        };

        // Saves the projected triangle to the the array of triangles to render.
        array_push(triangles_to_render, projected_triangle);
    }
}

void render(void)
{
    draw_grid();

    // Loop all projected triangles and render them.

    // earliest example:
    //draw_filled_triangle(300, 100, 50, 400, 500, 700, 0xFF00FF00);

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
