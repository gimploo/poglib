#include <stdio.h>

#include "../../basic.h"
#include "../my_math.h"

#include "../la.h"


int main(void)
{
    f32 x = 3.03;
    f32 y = 3.0343;
    f32 z = 0.01;

    if (x ==y)
        printf("yah");

    vec2f mat02[4];

    vec2f mat01[4] = {
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
}
