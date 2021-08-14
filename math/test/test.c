#include <stdio.h>

#include "../../basic.h"

#include "../la.h"
#include "../shapes.h"

int oldmain(void)
{
    vec4f_t black = {0.0f, 0.0f, 0.0f, 0.0f};
    vec4f_t white = {1.0f, 1.0f, 1.0f, 1.0f};

    vec4f_t copy;
    vec4f_copy(&copy, &white);
    printf(VEC4F_FMT"\n", VEC4F_ARG(copy));

    vec4f_copy(&copy, &black);
    printf(VEC4F_FMT"\n", VEC4F_ARG(copy));

    return 0;
}

void print_float(void *vertices)
{
    vec2f_t *all = (vec2f_t *)vertices;

    for (int i = 0; i < 5; i++)
        printf(VEC2F_FMT"\n", VEC2F_ARG(all[i]));
    printf("\n");
}

int main(void)
{

    quadf_t rect01 = quadf_init((vec2f_t) {0.0f, 0.0f}, 1.0f, 1.0f);
    quadf_t rect02 = quadf_init((vec2f_t) {-1.0f, 1.0f}, 1.0f, 1.0f);
    quadf_t rect03 = quadf_init((vec2f_t) {-1.0f, -1.0f}, 1.0f, 1.0f);

    print_float(&rect01);

    /*printf(QUAD_FMT"\n", QUAD_ARG(rect01));*/
    /*quadf_translate(&rect01, (vec2f_t_t) {0.5f, 0.5f});*/

    /*printf(QUAD_FMT, QUAD_ARG(rect01));*/
    return 0;
}


int oldmain01(void)
{
    f32 x = 3.03;
    f32 y = 3.0343;
    f32 z = 0.01;

    if (x ==y)
        printf("yah");

    vec2f_t mat02[4];

    vec2f_t mat01[4] = {
        {1.0f, 1.0f},
        {0.0f, 0.0f},
        {1.0f, 0.0f},
        {0.0f, 1.0f},
    };

    matrix4f_copy(mat02, mat01);
    for (int i = 0; i < 4; i++)
        printf(VEC2F_FMT"\n", VEC2F_ARG(mat01[i]));

    for (int i = 0; i < 4; i++)
        printf(VEC2F_FMT"\n", VEC2F_ARG(mat02[i]));

    /*
    printf("x = %f\n", abs(x));
    printf("y = %f\n", abs(y));
    printf("z = %f\n", abs(z));
    */
    return 0;
}
