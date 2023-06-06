#include "triangle.h"
#include "swap.h"
#include "display.h"

/* /////////////////////////////////////////////////////////////////////////////
// Draw a filled a triangle with a flat bottom
///////////////////////////////////////////////////////////////////////////////
//
//        (x0,y0)
//          / \
//         /   \
//        /     \
//       /       \
//      /         \
//  (x1,y1)------(x2,y2)
//
//  Note this (x2,y2) is the same as (Mx,My) from the calling function.
/////////////////////////////////////////////////////////////////////////////// */
void fill_flat_bottom_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
    // Find the slopes of x0,y0-x1,y1 and x0,y0-x2,y2.
    // Note this "slope" isn't delta-y, it's delta-x: how much x changes with a unit
    // change in y, since we start at the top pixel and move down 1 step in y each time.
    // We call this "inverse" slope because it's delta-x, not delta-y.
    float inverse_slope_left  =  (float)(x1 - x0) / (y1 - y0); // left triangle leg
    float inverse_slope_right =  (float)(x2 - x0) / (y2 - y0); // right triangle leg

    // x_start and x_end will be the start/end pixels to draw for each scanline.
    // Start at the top of the triangle.
    float x_start = x0;
    float x_end = x0;

    // Loop all the scanlines from the top to bottom.
    for (int y = y0; y <= y2; y++) {
        draw_line(x_start, y, x_end, y, color);

        x_start += inverse_slope_left;
        x_end += inverse_slope_right;

    }
}

/* ///////////////////////////////////////////////////////////////////////////////
// Draw a filled a triangle with a flat top
///////////////////////////////////////////////////////////////////////////////
//  Note this (x1,y1) is the same as (Mx,My) from the calling function.
//
//  (x0,y0)------(x1,y1)
//      \         /
//       \       /
//        \     /
//         \   /
//          \ /
//        (x2,y2)
//
/////////////////////////////////////////////////////////////////////////////// */
void fill_flat_top_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
    // Similar to fill_flat_bottom_triangle, but we fill from the bottom to the top.

    // Find the slopes of x0,y0-x2,y2 and x1,y1-x2,y2.
    // Note this "slope" isn't delta-y, it's delta-x: how much x changes with a unit
    // change in y, since we start at the top pixel and move down 1 step in y each time.
    // We call this "inverse" slope because it's delta-x, not delta-y.
    float inverse_slope_left  =  (float)(x2 - x0) / (y2 - y0);
    float inverse_slope_right =  (float)(x2 - x1) / (y2 - y1);

    // x_start and x_end will be the start/end pixels to draw for each scanline.
    // Start at the bottom of the triangle.
    float x_start = x2;
    float x_end = x2;

    // Loop all the scanlines from the bottom to top.
    for (int y = y2; y >= y0; y--) {
        draw_line(x_start, y, x_end, y, color);

        x_start -= inverse_slope_left;
        x_end -= inverse_slope_right;

    }
}

/* /////////////////////////////////////////////////////////////////////////////
// Draw a filled triangle with the flat-top/flat-bottom method.
// We split the original triangle in two, half flat-bottom (the top half of the triangle
// had a flat bottom) and half flat-top (the bottom half of the triangle has a
// flat top).
///////////////////////////////////////////////////////////////////////////////
//
//          (x0,y0)
//            / \ 
//           /   \ 
//          /     \ 
//         / flat- \ 
//        / bottom  \ 
//   (x1,y1)------(Mx,My)
//       \_  flat-top \ 
//          \_         \ 
//             \_       \ 
//                \_     \ 
//                   \    \ 
//                     \_  \ 
//                        \_\ 
//                           \ 
//                         (x2,y2)
//
/////////////////////////////////////////////////////////////////////////////// */

void draw_filled_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
    // First sort the triangle so that y0 < y1 < y2 (so y0 is at the top and y2 is at
    // the bottom of the triangle).
    if (y0 > y1) {
        int_swap(&x0, &x1);
        int_swap(&y0, &y1);
    }
    if (y1 > y2) {
        int_swap(&x1, &x2);
        int_swap(&y1, &y2);
    }
    if (y0 > y1) {
        int_swap(&x0, &x1);
        int_swap(&y0, &y1);
    }

    // There are 2 special cases where y0==y1 and y1==y2. In these cases, one of the
    // fill_flat_*_triangle() functions would do a div by zero, and it turns out
    // we don't need to call both fill_flat_*_triangle() functions in these
    // cases anyway.
    if (y0 == y1) {
        // This triangle has a flat top.
        fill_flat_top_triangle(x0, y0, x1, y1, x2, y2, color);

    } else if (y1 == y2) {
        // This triangle has a flat bottom.
        fill_flat_bottom_triangle(x0, y0, x1, y1, x2, y2, color);
    } else {
        // Calculate midpoint (Mx,My) using triangle similarity.
        int My = y1;
        int Mx = ((float)((x2 - x0) * (y1 - y0)) / (float)(y2 - y0)) + x0;

        // Draw flat-bottom triangle.
        fill_flat_bottom_triangle(x0, y0, x1, y1, Mx, My, color);

        // Draw flat-top triangle.
        fill_flat_top_triangle(x1, y1, Mx, My, x2, y2, color);
    }
}

/*/////////////////////////////////////////////////////////////////////////////
// Return the barycentric weights alpha, beta, and gamma for point p
///////////////////////////////////////////////////////////////////////////////
//
//         (B)
//         /|\
//        / | \
//       /  |  \
//      /  (P)  \
//     /  /   \  \
//    / /       \ \
//   //           \\
//  (A)------------(C)
//
//////////////////////////////////////////////////////////////////////////////*/
vec3_t barycentric_weights(vec2_t a, vec2_t b, vec2_t c, vec2_t p) {
    // Find the vectors between the vertices ABC and point p
    vec2_t ac = vec2_sub(c, a);
    vec2_t ab = vec2_sub(b, a);
    vec2_t ap = vec2_sub(p, a);
    vec2_t pc = vec2_sub(c, p);
    vec2_t pb = vec2_sub(b, p);

    // Compute the area of the full parallegram/triangle ABC using 2D cross product
    float area_parallelogram_abc = (ac.x * ab.y - ac.y * ab.x); // || AC x AB ||

    // Alpha is the area of the small parallelogram/triangle PBC divided by the area of the full parallelogram/triangle ABC
    float alpha = (pc.x * pb.y - pc.y * pb.x) / area_parallelogram_abc;

    // Beta is the area of the small parallelogram/triangle APC divided by the area of the full parallelogram/triangle ABC
    float beta = (ac.x * ap.y - ac.y * ap.x) / area_parallelogram_abc;

    // Weight gamma is easily found since barycentric coordinates always add up to 1.0
    float gamma = 1 - alpha - beta;

    vec3_t weights = { alpha, beta, gamma };
    return weights;
}

// Function to draw the textured pixel at position (x,y) on screen, using interpolation
// from 3 points of the triangle (points are a, b, and c).
void draw_texel(int x, int y, uint32_t * texture,
                vec2_t point_a, vec2_t point_b, vec2_t point_c,
                float u0, float v0, float u1, float v1, float u2, float v2)
{
    vec2_t point_p = { x, y };
    vec3_t weights = barycentric_weights(point_a, point_b, point_c, point_p);

    float alpha = weights.x;
    float beta = weights.y;
    float gamma = weights.z;

    // Interpolate the u and v values using barycentric weights alpha, beta, and gamma.
    float interpolated_u = (u0 * alpha) + (u1 * beta) + (u2 * gamma);
    float interpolated_v = (v0 * alpha) + (v1 * beta) + (v2 * gamma);

    // Map the interpolated u and v values to the right pixel in the texture.
    int texture_x = abs( (int)(interpolated_u * texture_width));
    int texture_y = abs( (int)(interpolated_v * texture_height));

    uint32_t texture_array_index = (texture_width * texture_y) + texture_x;

    draw_pixel(x, y, texture[texture_array_index]);
}


/* ////////////////////////////////////////////////////////////////////////////
// Draw a textured triangle based on a texture array of colors.
// We split the original triangle in two, half flat-bottom and half flat-top.
///////////////////////////////////////////////////////////////////////////////
//
//        v0
//        /\ 
//       /  \ 
//      /    \ 
//     /      \ 
//   v1--------\ 
//     \_       \ 
//        \_     \ 
//           \_   \ 
//              \_ \ 
//                 \\ 
//                   \ 
//                    v2
//
*/

void draw_textured_triangle(int x0, int y0, float u0, float v0, 
                            int x1, int y1, float u1, float v1,
                            int x2, int y2, float u2, float v2,
                            uint32_t *texture)
{
    // First sort the triangle so that y0 < y1 < y2 (so y0 is at the top and y2 is at
    // the bottom of the triangle).
    if (y0 > y1) {
        int_swap(&x0, &x1);
        int_swap(&y0, &y1);
        float_swap(&u0, &u1);
        float_swap(&v0, &v1);
    }
    if (y1 > y2) {
        int_swap(&x1, &x2);
        int_swap(&y1, &y2);
        float_swap(&u1, &u2);
        float_swap(&v1, &v2);
    }
    if (y0 > y1) {
        int_swap(&x0, &x1);
        int_swap(&y0, &y1);
        float_swap(&u0, &u1);
        float_swap(&v0, &v1);
    }

    // Create the vector points of the sorted triangle for use in interpolation.
    vec2_t point_a = { x0, y0 };
    vec2_t point_b = { x1, y1 };
    vec2_t point_c = { x2, y2 };

    // Render the upper part of the triangle - with a flat bottom.
    float inverse_slope_1 = 0.0; // left leg of triangle
    float inverse_slope_2 = 0.0; // right leg of triangle

    if ((y1 - y0) != 0) {
        inverse_slope_1 = (float)(x1 - x0) / (y1 - y0);
    }
    if ((y2 - y0) != 0) {
        inverse_slope_2 = (float)(x2 - x0) / (y2 - y0);
    }

    if ((y1 - y0) != 0) {

        for (int y = y0; y <= y1; y++) {
            int x_start = x1 + ((y - y1) * inverse_slope_1);
            int x_end = x0 + ((y - y0) * inverse_slope_2);

            // Depending on orientation of triangle, x_start and x_end may be on either side of each
            // other. Swap them so x_start is always on the left and x_end is always on the right.
            if (x_end < x_start) {
                int_swap(&x_start, &x_end);
            }

            for (int x = x_start; x < x_end; x++) {
                // Draw the pixel with the color that comes from the texture.
                //draw_pixel(x, y, 0xFFFF00FF);
                draw_texel(x, y, texture,
                           point_a, point_b, point_c,
                           u0, v0, u1, v1, u2, v2);
            }
        }
    }

    // Render the lower part of the triangle - with a flat top.
    inverse_slope_1 = 0.0; // left leg of triangle
    inverse_slope_2 = 0.0; // right leg of triangle

    if ((y2 - y1) != 0) {
        inverse_slope_1 = (float)(x2 - x1) / (y2 - y1);
    }
    if ((y2 - y0) != 0) {
        inverse_slope_2 = (float)(x2 - x0) / (y2 - y0);
    }

    if ((y2 - y1) != 0) {

        for (int y = y1; y <= y2; y++) {
            int x_start = x1 + ((y - y1) * inverse_slope_1);
            int x_end = x0 + ((y - y0) * inverse_slope_2);

            // Depending on orientation of triangle, x_start and x_end may be on either side of each
            // other. Swap them so x_start is always on the left and x_end is always on the right.
            if (x_end < x_start) {
                int_swap(&x_start, &x_end);
            }

            for (int x = x_start; x < x_end; x++) {
                // Draw the pixel with the color that comes from the texture.
                //draw_pixel(x, y, 0xFFFF0055);
                draw_texel(x, y, texture,
                           point_a, point_b, point_c,
                           u0, v0, u1, v1, u2, v2);
            }
        }
    }
}