#pragma once

#include <stdint.h>
#include "gfx-vector.h"


// "Global" light - light is everywhere, pointing in a direction (like the sun).
// Not a point source.
typedef struct {
    vec3_t direction;
} light_t;

extern light_t light;

uint32_t light_apply_intensity(uint32_t original_color, float percentage_factor);