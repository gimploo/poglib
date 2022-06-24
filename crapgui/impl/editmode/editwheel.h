#pragma once
#include <poglib/application.h>
#include <poglib/ds.h>

typedef enum optiontype {

    OT_FRAME_EDIT,
    OT_UI_EDIT,
    OT_COUNT,

} optiontype;

typedef struct editwheel_t {

    glvertex_t vertices[OT_COUNT];

} editwheel_t ;

#define EDIT_WHEEL_RADIUS 0.2f

void editwheel_render(void)
{
    editwheel_t wheel = {0};
    window_t *win = global_window;
    assert(win);
    vec2f_t pos = window_mouse_get_norm_position(win);

    vec2f_t points[OT_COUNT * 3] = {0};

    const f32 twicepi = 2.0f * (f32)PI;

    points[0] = pos;
    for (u64 i = 1; i < ARRAY_LEN(points); i++)
    {
        if (i%3 == 0) {
            points[i] = pos;
            continue;
        }

        f32 angle = i * twicepi / OT_COUNT;

        vec2f_t point = {
            .cmp[X] = pos.cmp[X] + (f32)cos(angle) * EDIT_WHEEL_RADIUS,
            .cmp[Y] = pos.cmp[Y] + (f32)sin(angle) * EDIT_WHEEL_RADIUS,
        };
        points[i] = point; 
    }

    for (u64 i = 0; i < ARRAY_LEN(points); i++)
    {
        wheel.vertices[i].position = vec3f(points[i]);
        wheel.vertices[i].color = COLOR_WHITE; 
    }

    vao_t vao = vao_init();
    vbo_t vbo; 

    vao_bind(&vao);

            vbo = vbo_init(wheel.vertices);
            vao_set_attributes(&vao, &vbo, 3, GL_FLOAT, false, sizeof(glvertex_t), offsetof(glvertex_t, position));
            vao_set_attributes(&vao, &vbo, 4, GL_FLOAT, false, sizeof(glvertex_t), offsetof(glvertex_t, color));

            /*glshader_bind(&GLOBAL_COMMON_GLSHADER);*/
            vao_draw_with_vbo_in_mode(&vao, &vbo, GL_TRIANGLE_STRIP);

    vao_unbind();

    vbo_destroy(&vbo);
    vao_destroy(&vao);

}



