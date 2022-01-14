#include <stdio.h>

#include "../../basic.h"

#include "../la.h"
#include "../shapes.h"

int oldmain(void)
{    
    vec3f_t _a[3] = {vec3f(1.0f), vec3f(2.0f), vec3f(3.0f)};
    vec3f_t _b[3] = {vec3f(1.0f), vec3f(2.0f), vec3f(3.0f)};

    matrixf_t a = matrixf_init(_a, 3, 3);
    matrixf_t b = matrixf_init(_b, 3, 3);
    matrixf_print(&a);
    matrixf_print(&b);

    matrixf_t output = matrixf_product(a, b);
    matrixf_print(&output);

    return 0;

}
int main(void)
{
    vec4f_t _a[4] = {vec4f(100.1f), vec4f(2.0f), vec4f(3.0f), vec4f(4.0f)};
    vec4f_t _b[4] = {vec4f(12.0f), vec4f(2.0f), vec4f(3.0f), vec4f(4.0f)};

    matrixf_t a = matrixf_init(_a, 3, 3);
    matrixf_t b = matrixf_init(_b, 3, 1);
    matrixf_print(&a);
    matrixf_print(&b);

    matrixf_t output = matrixf_product(a, b);
    matrixf_print(&output);

    return 0;
}


int old2main(void)
{
    vec2f_t _a[2] = {vec2f(1.0f), vec2f(2.0f)};
    matrixf_t a = matrix2f_init(_a);

    vec2f_t _b[2] = {vec2f(1.0f), vec2f(2.0f)};
    matrixf_t b = matrix2f_init(_b);

    matrixf_print(&a);
    matrixf_print(&b);

    matrixf_t output = matrixf_product(a, b);
    matrixf_print(&output);

    return 0;
}

