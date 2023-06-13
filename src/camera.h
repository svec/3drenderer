#pragma once
#include "gfx-vector.h"

typedef struct {
    vec3_t position;
    vec3_t direction;
    vec3_t forward_velocity;
    float yaw;
} camera_t;

extern camera_t camera;