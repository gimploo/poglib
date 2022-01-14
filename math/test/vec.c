#include "../vec.h"



int main(void)
{
    vec2f_t x = vec2f(1.0f);
    printf(VEC2F_FMT"\n", VEC2F_ARG(&x));
    vec2f_t y = vec2f(vec3f(2.0f));
    printf(VEC2F_FMT"\n", VEC2F_ARG(&y));
    vec2f_t z = vec2f(vec4f(3.0f));
    printf(VEC2F_FMT"\n", VEC2F_ARG(&z));

    vec3f_t a = vec3f(1.0f);
    printf(VEC3F_FMT"\n", VEC3F_ARG(&a));

    vec3f_t b = vec3f(vec4f(2.0f));
    printf(VEC3F_FMT"\n", VEC3F_ARG(&b));
    return 0;
}
