#include <stdio.h>
#include "texture.h"

tex2_t tex2_clone(tex2_t *p)
{
    tex2_t result = {p->u, p->v};
    return result;
}