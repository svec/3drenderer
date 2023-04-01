#include "triangle.h"
#include "display.h"

void int_swap(int *a, int *b) 
{
    int temp = *a;
    *a = *b;
    *b = temp;
}

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
// Draw a filled triangle with the flat-top/flat-bottom method
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

    // Calculate midpoint (Mx,My) using triangle similarity.
    int My = y1;
    int Mx = ((float)((x2 - x0) * (y1 - y0)) / (float)(y2 - y0)) + x0;

    // Draw flat-bottom triangle.
    fill_flat_bottom_triangle(x0, y0, x1, y1, Mx, My, color);

    // Draw flat-top triangle.
    fill_flat_top_triangle(x1, y1, Mx, My, x2, y2, color);
}