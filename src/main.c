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
#include "upng.h"
#include "camera.h"
#include "clipping.h"

int previous_frame_time = 0;
float delta_time_s = 0;
bool g_display_back_face_culling = true;
bool g_display_vertex_dot = false;
bool g_display_wireframe_lines = true;
bool g_display_filled_trianges = false;
bool g_display_texture = false;

#define MAX_TRIANGLES_PER_MESH (10000)

triangle_t triangles_to_render[MAX_TRIANGLES_PER_MESH];
int num_triangles_to_render = 0;

mat4_t proj_matrix;
mat4_t world_matrix;
mat4_t view_matrix;

bool is_running = false;

bool load_objects_to_display(void)
{
#if 0
    bool all_good = false;
    
    all_good = load_mesh("./assets/f22.obj", "./assets/f22.png",
                         vec3_new(1, 1, 1),
                         vec3_new(-3, 0, 8), // offset a bit left and into the monitor
                         vec3_new(0, 0, 0));

    if (! all_good) {
        fprintf(stderr, "Error: load_mesh failed.\n");
        return false;
    }

    all_good = load_mesh("./assets/efa.obj", "./assets/efa.png",
                         vec3_new(1, 1, 1),
                         vec3_new(+3, 0, 8), // offset a bit right and into the monitor
                         vec3_new(0, 0, 0));

    if (! all_good) {
        fprintf(stderr, "Error: load_mesh failed.\n");
        return false;
    }
#endif

    //load_mesh("./assets/runway.obj", "./assets/runway.png", vec3_new(1, 1, 1), vec3_new(0, -1.5, +23), vec3_new(0, 0, 0));
    //load_mesh("./assets/f22.obj", "./assets/f22.png", vec3_new(1, 1, 1), vec3_new(0, -1.3, +5), vec3_new(0, -M_PI/2, 0));
    //load_mesh("./assets/efa.obj", "./assets/efa.png", vec3_new(1, 1, 1), vec3_new(-2, -1.3, +9), vec3_new(0, -M_PI/2, 0));
    //load_mesh("./assets/f117.obj", "./assets/f117.png", vec3_new(1, 1, 1), vec3_new(+2, -1.3, +9), vec3_new(0, -M_PI/2, 0));

    // Don't have texture assets for the simple pyramid, so just re-use the f117's.
    load_mesh("./assets/simple_pyramid.obj", "./assets/f117.png", vec3_new(1, 1, 1), vec3_new(+2, -1.3, +9), vec3_new(0, -M_PI/2, 0));

    return true;
}

bool setup(void)
{
    // Initialize the scene light direction.
    init_light(vec3_new(0, 0, 1));

    // Initialize the perspective projection matrix.
    float aspect_x = (float)get_window_width() / (float)get_window_width();
    float aspect_y = (float)get_window_width() / (float)get_window_width();
    float fov_y = M_PI/3.0; // 60 degrees = 180/3, = PI/3 in radians

    // Math for horizonal field of view comes from
    // https://en.wikipedia.org/wiki/Field_of_view_in_video_games
    float fov_x = 2.0 * atan(tan(fov_y/2.0) * aspect_x);

    float z_near = 0.1;
    float z_far = 100.0;
    proj_matrix = mat4_make_projection(fov_y, aspect_y, z_near, z_far);

    // Initialize 6 frustum planes.
    // (left, right, top, bottom, front, back)
    init_frustum_planes(fov_x, fov_y, z_near, z_far);

    bool all_good = load_objects_to_display();

    return all_good;
}

void process_input(void)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {

        switch (event.type)
        {
        case SDL_QUIT: // User clicks window close button
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
                Pressing “x” we should disable the back-face culling
                */
            if (event.key.keysym.sym == SDLK_ESCAPE)
            {
                is_running = false;
            }
            if (event.key.keysym.sym == SDLK_1)
            {
                g_display_vertex_dot = true;
                g_display_wireframe_lines = true;
            }
            if (event.key.keysym.sym == SDLK_2)
            {
                g_display_vertex_dot = false;
                g_display_wireframe_lines = true;
                g_display_filled_trianges = false;
            }
            if (event.key.keysym.sym == SDLK_3)
            {
                g_display_vertex_dot = false;
                g_display_wireframe_lines = false;
                g_display_filled_trianges = true;
                g_display_texture = false;
            }
            if (event.key.keysym.sym == SDLK_4)
            {
                g_display_vertex_dot = false;
                g_display_wireframe_lines = true;
                g_display_filled_trianges = true;
                g_display_texture = false;
            }
            if (event.key.keysym.sym == SDLK_5)
            {
                g_display_vertex_dot = false;
                g_display_wireframe_lines = false;
                g_display_filled_trianges = false;
                g_display_texture = true;
            }
            if (event.key.keysym.sym == SDLK_6)
            {
                g_display_vertex_dot = false;
                g_display_wireframe_lines = true;
                g_display_filled_trianges = false;
                g_display_texture = true;
            }
            if (event.key.keysym.sym == SDLK_c)
            {
                g_display_back_face_culling = true;
            }
            if (event.key.keysym.sym == SDLK_x)
            {
                g_display_back_face_culling = false;
            }
            if (event.key.keysym.sym == SDLK_UP)
            {
                update_camera_forward_velocity(vec3_mul(get_camera_direction(), 5.0 * delta_time_s));
                update_camera_position(vec3_add(get_camera_position(), get_camera_forward_velocity()));
            }
            if (event.key.keysym.sym == SDLK_DOWN)
            {
                update_camera_forward_velocity(vec3_mul(get_camera_direction(), 5.0 * delta_time_s));
                update_camera_position(vec3_sub(get_camera_position(), get_camera_forward_velocity()));
            }
            if (event.key.keysym.sym == SDLK_w)
            {
                rotate_camera_pitch(+3.0 * delta_time_s);
            }
            if (event.key.keysym.sym == SDLK_s)
            {
                rotate_camera_pitch(-3.0 * delta_time_s);
            }
            if (event.key.keysym.sym == SDLK_RIGHT)
            {
                rotate_camera_yaw(+1.0 * delta_time_s);
            }
            if (event.key.keysym.sym == SDLK_LEFT)
            {
                rotate_camera_yaw(-1.0 * delta_time_s);
            }
            break;
        default:
            break;
        }
    }
}

/* /////////////////////////////////////////////////////////////////////////////
// Process the graphics pipeline stages for all the mesh triangles
///////////////////////////////////////////////////////////////////////////////
// +-------------+
// | Model space |  <-- original mesh vertices
// +-------------+
// |   +-------------+
// `-> | World space |  <-- multiply by world matrix
//     +-------------+
//     |   +--------------+
//     `-> | Camera space |  <-- multiply by view matrix
//         +--------------+
//         |    +------------+
//         `--> |  Clipping  |  <-- clip against the six frustum planes
//              +------------+
//              |    +------------+
//              `--> | Projection |  <-- multiply by projection matrix
//                   +------------+
//                   |    +-------------+
//                   `--> | Image space |  <-- apply perspective divide
//                        +-------------+
//                        |    +--------------+
//                        `--> | Screen space |  <-- ready to render
//                             +--------------+
/////////////////////////////////////////////////////////////////////////////// */
void process_graphics_pipeline_stages(mesh_t * mesh)
{
    // Create the view matrix using the current camera position and target.
    vec3_t target = get_camera_lookat_target();
    vec3_t up_direction = {0, 1, 0}; // normalized y axis
    mat4_t view_matrix = mat4_look_at(get_camera_position(), target, up_direction);

    // Create scale, translation, and rotation matrices that will be used to multiply the mesh vertices.
    mat4_t scale_matrix = mat4_make_scale(mesh->scale.x, mesh->scale.y, mesh->scale.z);
    mat4_t translation_matrix = mat4_make_translation(mesh->translation.x, mesh->translation.y, mesh->translation.z);
    mat4_t rotation_matrix_x = mat4_make_rotation_x(mesh->rotation.x);
    mat4_t rotation_matrix_y = mat4_make_rotation_y(mesh->rotation.y);
    mat4_t rotation_matrix_z = mat4_make_rotation_z(mesh->rotation.z);

    // Loop all triangle faces of the mesh.
    int num_faces = array_length(mesh->faces);

    for (int face_i = 0; face_i < num_faces; face_i++)
    {
        // Handle 1 triangle face per iteration.

        face_t mesh_face = mesh->faces[face_i];
        vec3_t face_vertices[3];
        face_vertices[0] = mesh->vertices[mesh_face.a];
        face_vertices[1] = mesh->vertices[mesh_face.b];
        face_vertices[2] = mesh->vertices[mesh_face.c];

        vec4_t transformed_vertices[3];

        // For all 3 vertices of this triangle, apply transformations.
        for (int vertex_i = 0; vertex_i < 3; vertex_i++)
        {
            vec4_t transformed_vertex = vec4_from_vec3(face_vertices[vertex_i]);

            // Creating a single World Matrix combining the scale, rotation, and translation matrices.
            // Note that the order matters: Must be scale first, then rotation, and finally translation last.
            world_matrix = mat4_identity();
            world_matrix = mat4_mul_mat4(scale_matrix, world_matrix);
            world_matrix = mat4_mul_mat4(rotation_matrix_x, world_matrix);
            world_matrix = mat4_mul_mat4(rotation_matrix_y, world_matrix);
            world_matrix = mat4_mul_mat4(rotation_matrix_z, world_matrix);
            world_matrix = mat4_mul_mat4(translation_matrix, world_matrix);

            // Multiply (apply) the World Matrix by the vertex to get the transformed vertex.
            transformed_vertex = mat4_mul_vec4(world_matrix, transformed_vertex);

            // Multiply the view matrix by the vertex vector to transform the scene to camera space.
            transformed_vertex = mat4_mul_vec4(view_matrix, transformed_vertex);

            // Save off the transformed vertex.
            transformed_vertices[vertex_i] = transformed_vertex;
        }

        // Calculate the triangle face normal.
        vec3_t face_normal = get_triangle_normal(transformed_vertices);

        if (g_display_back_face_culling)
        {
            // Find the vector between a point in the triangle (point A) and the axes origin.
            vec3_t origin = {0, 0, 0};
            vec3_t camera_ray = vec3_sub(origin, vec3_from_vec4(transformed_vertices[0]));

            // How aligned is the face's normal with the camera ray?
            float dot_normal_camera = vec3_dot(face_normal, camera_ray);
            if (dot_normal_camera < 0)
            {
                // If the dot product is < 0, then the face is pointing away from the camera,
                // and we don't need to display it.
                continue;
            }
        }

        // Clipping!
        // First, create a polygon starting with the triangle.
        polygon_t polygon = create_polygon_from_triangle(vec3_from_vec4(transformed_vertices[0]),
                                                         vec3_from_vec4(transformed_vertices[1]),
                                                         vec3_from_vec4(transformed_vertices[2]),
                                                         mesh_face.a_uv,
                                                         mesh_face.b_uv,
                                                         mesh_face.c_uv);

        // Now clip the polygon against the frustum so we only display things we can actually see.
        // Note that the polygon starts as a triangle, but the act of clipping it may turn it into
        // a polygon with more than just 3 points. It could also be empty if the entire polygon
        // is clipped!
        clip_polygon(&polygon); // Note the polygon structure is modified inside clip_polygon().

        // After the polygon has been clipped, we'll need to break it up into triangles for projection
        // and display.
        triangle_t triangles_after_clipping[MAX_NUM_POLY_TRIANGLES];
        int num_triangles_after_clipping = 0;

        triangles_from_polygon(&polygon, triangles_after_clipping, &num_triangles_after_clipping);

        // Now that we have the array of triangles to display after being clipped, loop through
        // all the triangles to project them.
        for (int tri = 0; tri < num_triangles_after_clipping; tri++)
        {

            triangle_t triangle_after_clipping = triangles_after_clipping[tri];

            vec4_t projected_points[3];

            for (int vertex_i = 0; vertex_i < 3; vertex_i++)
            {
                // Project the current vertex.
                projected_points[vertex_i] = mat4_mul_vec4_project(proj_matrix, triangle_after_clipping.points[vertex_i]);

                // Scale into the view.
                projected_points[vertex_i].x *= (get_window_width() / 2.0);
                projected_points[vertex_i].y *= (get_window_width() / 2.0);

                // Invert the y values to account for flipped screen y coordinate: in our object coordinate system,
                // y increases going "up" the screen, but in SDL the y increases going "down" the screen.
                projected_points[vertex_i].y *= -1.0;

                // Translate the projected points to the middle of the screen.
                projected_points[vertex_i].x += (get_window_width() / 2.0); // translate to center of window
                projected_points[vertex_i].y += (get_window_width() / 2.0); // translate to center of window
            }

            // Calculate the triangle color based on the original triangle color and the angle of the light
            // on the triangle.
            // We compare the light ray's direction and the triangle face normal vector to see how aligned the triangle is
            // with the light. The more aligned the triangle face is with the light, the more the light will
            // brighten the triangle.
            // We use the negative of the dot product because we actually care about the opposite of the light
            // direction: we want max light if the normal is pointed directly opposite the light direction.
            // Using the negative of the dot product does this.
            float light_intensity_factor = -vec3_dot(face_normal, get_light_direction());
            uint32_t triangle_color = light_apply_intensity(mesh_face.color, light_intensity_factor);
            // uint32_t triangle_color = mesh_face.color;

            triangle_t triangle_to_render = {
                .points = {
                    {projected_points[0].x, projected_points[0].y, projected_points[0].z, projected_points[0].w},
                    {projected_points[1].x, projected_points[1].y, projected_points[1].z, projected_points[1].w},
                    {projected_points[2].x, projected_points[2].y, projected_points[2].z, projected_points[2].w},
                },
                .texcoords = {
                    {triangle_after_clipping.texcoords[0].u, triangle_after_clipping.texcoords[0].v},
                    {triangle_after_clipping.texcoords[1].u, triangle_after_clipping.texcoords[1].v},
                    {triangle_after_clipping.texcoords[2].u, triangle_after_clipping.texcoords[2].v},
                },
                .color = triangle_color,
                .texture = mesh->texture,
            };

            // Saves the projected triangle to the the array of triangles to render.
            if (num_triangles_to_render < MAX_TRIANGLES_PER_MESH)
            {
                triangles_to_render[num_triangles_to_render] = triangle_to_render;
                num_triangles_to_render++;
            }
            else
            {
                fprintf(stderr, "ERROR: trying to render %d triangles, which is more than the max allowed: %d\n",
                        num_triangles_to_render, MAX_TRIANGLES_PER_MESH);
            }
        }
    }
}

void update(void)
{
    // Do we need to delay before updating the frame?
    int time_to_wait_ms = FRAME_TARGET_TIME_MS - (SDL_GetTicks() - previous_frame_time);

    if ((time_to_wait_ms > 0) && (time_to_wait_ms <= FRAME_TARGET_TIME_MS)) {
        // Delay until it's time for the next frame.
        SDL_Delay(time_to_wait_ms);
    }

    // Get the time in seconds that's passed since we rendered the last frame.
    delta_time_s = (SDL_GetTicks() - previous_frame_time) / 1000.0;

    // How many ms have passed since we last were called?
    previous_frame_time = SDL_GetTicks();

    // Reset the triangle counter for this tick.
    num_triangles_to_render = 0;

    for (int mesh_index = 0; mesh_index < get_num_meshes(); mesh_index++)
    {
        mesh_t *mesh = get_mesh(mesh_index);

        if (mesh_index == 1) {
            mesh->rotation.x += 0.6 * delta_time_s;
        }
        else if (mesh_index == 2) {
            mesh->rotation.y += 0.6 * delta_time_s;
        }
        else if (mesh_index == 3) {
            mesh->rotation.z += 0.6 * delta_time_s;
        }

        // mesh->scale.x += 0.02 * delta_time_s;
        // mesh->scale.y += 0.01 * delta_time_s;
        // mesh->scale.z += 0.03 * delta_time_s;

        // mesh->translation.x += 0.1 * delta_time_s;
        // mesh->translation.y += 0.2 * delta_time_s;

        // Translate the vertex away from the camera.
        // mesh->translation.z = 5.0;

        process_graphics_pipeline_stages(mesh);
    }
}

void render(void)
{
    clear_color_buffer(0xFF000000);
    clear_z_buffer();

    draw_grid();

    // Loop all projected triangles and render them.

    // earliest example:
    //draw_filled_triangle(300, 100, 50, 400, 500, 700, 0xFF00FF00);

    // triangles_to_render is already sorted from back to front.
    for (int ii=0; ii < num_triangles_to_render; ii++) {
        triangle_t triangle = triangles_to_render[ii];

        if (g_display_filled_trianges) {
            draw_filled_triangle(
                triangle.points[0].x,
                triangle.points[0].y,
                triangle.points[0].z,
                triangle.points[0].w,
                triangle.points[1].x,
                triangle.points[1].y,
                triangle.points[1].z,
                triangle.points[1].w,
                triangle.points[2].x,
                triangle.points[2].y,
                triangle.points[2].z,
                triangle.points[2].w,
                triangle.color);
        }

        if (g_display_texture) {
            draw_textured_triangle(
                triangle.points[0].x,
                triangle.points[0].y,
                triangle.points[0].z,
                triangle.points[0].w,
                triangle.texcoords[0].u,
                triangle.texcoords[0].v,
                triangle.points[1].x,
                triangle.points[1].y,
                triangle.points[1].z,
                triangle.points[1].w,
                triangle.texcoords[1].u,
                triangle.texcoords[1].v,
                triangle.points[2].x,
                triangle.points[2].y,
                triangle.points[2].z,
                triangle.points[2].w,
                triangle.texcoords[2].u,
                triangle.texcoords[2].v,
                triangle.texture);
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

    render_color_buffer();
}

void free_resources(void)
{
    free_meshes();
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
