#include <stdio.h>
#include "clipping.h"

#define NUM_PLANES (6)
plane_t frustum_planes[NUM_PLANES];

/*/////////////////////////////////////////////////////////////////////////////
// Frustum planes are defined by a point and a normal vector
///////////////////////////////////////////////////////////////////////////////
// Near plane   :  P=(0, 0, znear), N=(0, 0,  1)
// Far plane    :  P=(0, 0, zfar),  N=(0, 0, -1)
// Top plane    :  P=(0, 0, 0),     N=(0, -cos(fov/2), sin(fov/2))
// Bottom plane :  P=(0, 0, 0),     N=(0, cos(fov/2), sin(fov/2))
// Left plane   :  P=(0, 0, 0),     N=(cos(fov/2), 0, sin(fov/2))
// Right plane  :  P=(0, 0, 0),     N=(-cos(fov/2), 0, sin(fov/2))
///////////////////////////////////////////////////////////////////////////////
//
//           /|\
//         /  | | 
//       /\   | |
//     /      | |
//  P*|-->  <-|*|   ----> +z-axis
//     \      | |
//       \/   | |
//         \  | | 
//           \|/
//
/////////////////////////////////////////////////////////////////////////////*/
void init_frustum_planes(float fov_x, float fov_y, float z_near, float z_far)
{
    float cos_half_fov_x = cos(fov_x / 2);
    float sin_half_fov_x = sin(fov_x / 2);
    float cos_half_fov_y = cos(fov_y / 2);
    float sin_half_fov_y = sin(fov_y / 2);

    frustum_planes[LEFT_FRUSTUM_PLANE].point = vec3_new(0, 0, 0);
    frustum_planes[LEFT_FRUSTUM_PLANE].normal.x = cos_half_fov_x;
    frustum_planes[LEFT_FRUSTUM_PLANE].normal.y = 0;
    frustum_planes[LEFT_FRUSTUM_PLANE].normal.z = sin_half_fov_x;

    frustum_planes[RIGHT_FRUSTUM_PLANE].point = vec3_new(0, 0, 0);
    frustum_planes[RIGHT_FRUSTUM_PLANE].normal.x = -cos_half_fov_x;
    frustum_planes[RIGHT_FRUSTUM_PLANE].normal.y = 0;
    frustum_planes[RIGHT_FRUSTUM_PLANE].normal.z = sin_half_fov_x;

    frustum_planes[TOP_FRUSTUM_PLANE].point = vec3_new(0, 0, 0);
    frustum_planes[TOP_FRUSTUM_PLANE].normal.x = 0;
    frustum_planes[TOP_FRUSTUM_PLANE].normal.y = -cos_half_fov_y;
    frustum_planes[TOP_FRUSTUM_PLANE].normal.z = sin_half_fov_y;

    frustum_planes[BOTTOM_FRUSTUM_PLANE].point = vec3_new(0, 0, 0);
    frustum_planes[BOTTOM_FRUSTUM_PLANE].normal.x = 0;
    frustum_planes[BOTTOM_FRUSTUM_PLANE].normal.y = cos_half_fov_y;
    frustum_planes[BOTTOM_FRUSTUM_PLANE].normal.z = sin_half_fov_y;

    frustum_planes[NEAR_FRUSTUM_PLANE].point = vec3_new(0, 0, z_near);
    frustum_planes[NEAR_FRUSTUM_PLANE].normal.x = 0;
    frustum_planes[NEAR_FRUSTUM_PLANE].normal.y = 0;
    frustum_planes[NEAR_FRUSTUM_PLANE].normal.z = 1;

    frustum_planes[FAR_FRUSTUM_PLANE].point = vec3_new(0, 0, z_far);
    frustum_planes[FAR_FRUSTUM_PLANE].normal.x = 0;
    frustum_planes[FAR_FRUSTUM_PLANE].normal.y = 0;
    frustum_planes[FAR_FRUSTUM_PLANE].normal.z = -1;
}

polygon_t create_polygon_from_triangle(vec3_t v0, vec3_t v1, vec3_t v2)
{
    polygon_t result = {
        .vertices = {v0, v1, v2},
        .num_vertices = 3,
    };
    return result;
}

void clip_polygon_against_plane(polygon_t * polygon, int plane)
{
    vec3_t plane_point = frustum_planes[plane].point;
    vec3_t plane_normal = frustum_planes[plane].normal;

    vec3_t inside_vertices[MAX_NUM_POLY_VERTICES]; // local var, we'll copy it out before we return.
    int num_inside_vertices = 0;

    // We always track 2 adjecent vertices. Start with the first and the last.
    vec3_t * current_vertex_p = &(polygon->vertices[0]);
    vec3_t * previous_vertex_p = &(polygon->vertices[polygon->num_vertices - 1]);

    // Get the dot product of the current point and the previous point to see if 
    // they're inside or ourside the frustum plane.
    float current_dot = 0.0; // We'll calculate this first thing in the while loop below.
    float previous_dot = vec3_dot(vec3_sub(*previous_vertex_p, plane_point), plane_normal);

    // Loop through all the polygon vertices.
    // Using one element past the end of the polygon->vertices[] array as as the stopping point.
    while (current_vertex_p != &(polygon->vertices[polygon->num_vertices]))
    {
        current_dot = vec3_dot(vec3_sub(*current_vertex_p, plane_point), plane_normal);

        // If the signs of dot product of the current vs previous are different, then we know we 
        // we changed from (inside to outside) or (outside to inside) of the frustum plane,
        // meaning the line between the previous vertex to the current vertex crosses the
        // frustum plane.
        // We use "multiple and test if < negative" to see if the signs of the dot products
        // are different.
        if ((current_dot * previous_dot) < 0.0) {
            // Since we've crossed the frustum plane, find out where we intersect it.
            // We'll want to keep the part of the polygon that's inside the frustum plane,
            // and discard the part of the polygon that's outside the frustum plane.
            // The intersection point of the line between the previous and current vertex is
            // Intersection Point I = Q1 + t(Q2 - Q1), where Q1 is the previous vertex and
            // Q2 is the current vertex.
            // t = dot(Q1) / (dot(Q1) - dot(Q2))
            float t = 0.0;
            if ((previous_dot - current_dot) != 0.0) {
                t = previous_dot / (previous_dot - current_dot);
                // Start with I = (Q2 - Q1).
                vec3_t intersection_point = vec3_sub(*current_vertex_p, *previous_vertex_p); // I = (Q2-Q1)
                intersection_point = vec3_mul(intersection_point, t);                        // Now I = t(Q2 - Q1)
                intersection_point = vec3_add(*previous_vertex_p, intersection_point);       // Now I = Q1+t(Q2-Q1) - done!

                inside_vertices[num_inside_vertices] = vec3_clone(&intersection_point);
                num_inside_vertices++;
            }
            else {
                fprintf(stderr, "Warning: dot products difference == 0.0, avoiding a div-by-0 hackily\n");
            }
        }

        if (current_dot > 0) {
            // If the dot product is > 0, that means this vertex is inside the frustum plane.
            inside_vertices[num_inside_vertices] = vec3_clone(current_vertex_p);
            num_inside_vertices++;
        }

        // Done with this iteration. Move the current vertex one ahead, with the
        // previous vertex following along.
        // The previous and current vertex are always adjacent.
        previous_dot = current_dot;
        previous_vertex_p = current_vertex_p;
        current_vertex_p++;
    }

    // Now that we're done building the list of inside_vertices, copy those vertices into the
    // polygon pointer parameter for use outside this function.
    for (int ii = 0; ii < num_inside_vertices; ii++) {
        polygon->vertices[ii] = vec3_clone(&inside_vertices[ii]);
    }
    polygon->num_vertices = num_inside_vertices;
}

bool clip_polygon(polygon_t *polygon)
{
    clip_polygon_against_plane(polygon, LEFT_FRUSTUM_PLANE);
    clip_polygon_against_plane(polygon, RIGHT_FRUSTUM_PLANE);
    clip_polygon_against_plane(polygon, TOP_FRUSTUM_PLANE);
    clip_polygon_against_plane(polygon, BOTTOM_FRUSTUM_PLANE);
    clip_polygon_against_plane(polygon, NEAR_FRUSTUM_PLANE);
    clip_polygon_against_plane(polygon, FAR_FRUSTUM_PLANE);

    return true;
}

bool triangles_from_polygon(polygon_t *polygon, triangle_t triangles[], int *num_triangles)
{
    for (int ii=0; ii < (polygon->num_vertices - 2); ii++) {
        // We use the 0th point as a point in all triangles, and then choose
        // the next 2 points from the polygon in each loop iteration.
        int index0 = 0;
        int index1 = ii+1;
        int index2 = ii+2;

        triangles[ii].points[0] = vec4_from_vec3(polygon->vertices[index0]);
        triangles[ii].points[1] = vec4_from_vec3(polygon->vertices[index1]);
        triangles[ii].points[2] = vec4_from_vec3(polygon->vertices[index2]);

    }
    // We always get (polygon->num_vertices - 2) triangles from our polygon.
    *num_triangles = polygon->num_vertices - 2;

    return true;
}