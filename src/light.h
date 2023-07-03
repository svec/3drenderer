#pragma once

#include <stdint.h>
#include "gfx-vector.h"


// "Global" light - light is everywhere, pointing in a direction (like the sun).
// Not a point source.
typedef struct {
    vec3_t direction;
} light_t;

void init_light(vec3_t direction);
vec3_t get_light_direction(void);
uint32_t light_apply_intensity(uint32_t original_color, float percentage_factor);