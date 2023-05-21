#include <math.h>
#include "matrix.h"

vec4_t mat4_mul_vec4(mat4_t m, vec4_t v)
{
    vec4_t result;

    result.x =    m.m[0][0] * v.x 
                 + m.m[0][1] * v.y
                 + m.m[0][2] * v.z
                 + m.m[0][3] * v.w;

    result.y =    m.m[1][0] * v.x 
                 + m.m[1][1] * v.y
                 + m.m[1][2] * v.z
                 + m.m[1][3] * v.w;

    result.z =    m.m[2][0] * v.x 
                 + m.m[2][1] * v.y
                 + m.m[2][2] * v.z
                 + m.m[2][3] * v.w;

    result.w =    m.m[3][0] * v.x 
                 + m.m[3][1] * v.y
                 + m.m[3][2] * v.z
                 + m.m[3][3] * v.w;


    return result;
}

mat4_t mat4_identity(void)
{
    // | 1 0 0 0 |
    // | 0 1 0 0 |
    // | 0 0 1 0 |
    // | 0 0 0 1 |

    mat4_t m = {{
        {1, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 1},
    }};

    return m;
}

mat4_t mat4_make_scale(float scale_x, float scale_y, float scale_z)
{
    // | sx  0  0  0 |
    // |  0 sy  0  0 |
    // |  0  0 sz  0 |
    // |  0  0  0  1 |

    mat4_t m = mat4_identity();
    m.m[0][0] = scale_x;
    m.m[1][1] = scale_y;
    m.m[2][2] = scale_z;

    return m;
}

mat4_t mat4_make_translation(float tx, float ty, float tz)
{
    // |  1  0  0  tx |
    // |  0  1  0  ty |
    // |  0  0  1  tz |
    // |  0  0  0   1 |

    mat4_t m = mat4_identity();
    m.m[0][3] = tx;
    m.m[1][3] = ty;
    m.m[2][3] = tz;

    return m;
}

mat4_t mat4_make_rotation_x(float angle)
{
    float c = cos(angle);
    float s = sin(angle);
    // |  1  0   0  0 |
    // |  0  c  -s  0 |
    // |  0  s   c  0 |
    // |  0  0   0  1 |

    mat4_t m = mat4_identity();
    m.m[1][1] = c;
    m.m[1][2] = -s;
    m.m[2][1] = s;
    m.m[2][2] = c;

    return m;
}

mat4_t mat4_make_rotation_y(float angle)
{
    // Note that the sin/-sin are flipped here to make y rotate
    // in a consistent direction with x and z.
    float c = cos(angle);
    float s = sin(angle);
    // |  c  0   s  0 |
    // |  0  1   0  0 |
    // | -s  0   c  0 |
    // |  0  0   0  1 |

    mat4_t m = mat4_identity();
    m.m[0][0] = c;
    m.m[0][2] = s;
    m.m[2][0] = -s;
    m.m[2][2] = c;

    return m;
}

mat4_t mat4_make_rotation_z(float angle)
{
    float c = cos(angle);
    float s = sin(angle);
    // |  c -s   0  0 |
    // |  s  c   0  0 |
    // |  0  0   1  0 |
    // |  0  0   0  1 |

    mat4_t m = mat4_identity();
    m.m[0][0] = c;
    m.m[0][1] = -s;
    m.m[1][0] = s;
    m.m[1][1] = c;

    return m;
}

mat4_t mat4_mul_mat4(mat4_t a, mat4_t b)
{
    mat4_t m;
    for (int row=0; row < 4; row++) {
        for (int col=0; col < 4; col++) {
            m.m[row][col] =   a.m[row][0] * b.m[0][col] + a.m[row][1] * b.m[1][col] 
                            + a.m[row][2] * b.m[2][col] + a.m[row][3] * b.m[3][col];
        }
    }
    return m;
}

mat4_t mat4_make_projection(float fov /* field of view angle*/, float aspect /* screen h/w */, float znear, float zfar)
{
    // | (h/w) * 1/tan(fov/2)              0                     0                             0 |
    // |                    0   1/tan(fov/2)                     0                             0 |
    // |                    0              0   zfar / (zfar-znear)    (-zfar*znear)/(zfar-znear) |
    // |                    0              0                     1                             0 |
    mat4_t m = {{{ 0 }}};
    m.m[0][0] = aspect * (1 / tan(fov/2));
    m.m[1][1] = 1/tan(fov/2);
    m.m[2][2] = zfar / (zfar - znear);
    m.m[2][3] = (-zfar * znear) / (zfar - znear);
    m.m[3][2] = 1.0; // copies original z to w for use later

    return m;
}

vec4_t mat4_mul_vec4_project(mat4_t mat_proj, vec4_t v)
{
    // Multiply the projection matrix by a vector.
    vec4_t result = mat4_mul_vec4(mat_proj, v);

    // Now perform perspective divide by dividing by the original z depth, which is
    // stored in the w value.
    if (result.w != 0.0) {
        result.x /= result.w;
        result.y /= result.w;
        result.z /= result.w;
    }

    return result;
}