#pragma once

#include "./common.h"

INTERNAL bool __frame_is_mouse_over(frame_t *frame)
{
    vec2f_t norm_mouse_position = window_mouse_get_norm_position(frame->gui_handler->window_handler);
    return quadf_is_point_in_quad(frame->__quad_vertices, vec3f(norm_mouse_position));
}

INTERNAL vec3f_t __frame_get_mouse_position_relative_to_frame_coordinate_axis(frame_t *frame)
{
    //NOTE: here we convert the mouse position in the frame to the window position, 
    //because all the ui components holds the quad vertices relative to the window and not to the frame 

    vec2i_t mouse_position = window_mouse_get_position(frame->gui_handler->window_handler);

    u32 frame_width = ((frame->norm_width)/2.0f) * frame->gui_handler->window_handler->width;
    u32 frame_height = ((frame->norm_height)/2.0f) * frame->gui_handler->window_handler->height;

    vec3f_t output = {
         .cmp[X] = (f32) (-1.0 + 2.0 *  (f32) mouse_position.cmp[X] / frame_width),
         .cmp[Y] = (f32) (1.0 - 2.0 * (f32) mouse_position.cmp[Y] / frame_height),
         .cmp[Z] = 1.0f,
    };

    return output;
}

frame_t frame_init(crapgui_t *gui, vec3f_t norm_position, f32 norm_width, f32 norm_height)
{
    frame_t frame = {
        .__quad_vertices = quadf(norm_position, norm_width, norm_height),
        .__fbo =  glframebuffer_init(gui->window_handler->width, gui->window_handler->height),
        .gui_handler = gui,
        .norm_position = norm_position,
        .norm_width = norm_width,
        .norm_height = norm_height,
        .norm_color  = FRAME_DEFAULT_COLOR,
        .is_open = true,
        .ui_cmps = list_init(UICMP_COUNT, hashtable_t )
    };

    list_iterator(&frame.ui_cmps, ui) {
        hashtable_t table = hashtable_init(MAX_UICMPS_PER_FRAME, uicmp_t );
        memcpy((uicmp_t *)ui, &table, sizeof(table)); 
    }
    return frame;
}


void frame_begin(frame_t *frame)
{
    glClearColor(
            frame->norm_color.cmp[X], 
            frame->norm_color.cmp[Y],
            frame->norm_color.cmp[Z],
            0.0f);
    glframebuffer_begin_scene(&frame->__fbo);
}


void frame_end(frame_t *frame)
{
    glframebuffer_end_scene(&frame->fbo);
}

void frame_draw(frame_t *frame)
{
    glquad_t quad = glquad(
            quadf(frame->norm_position, frame->norm_width, frame->norm_height),
            frame->norm_color, 
            quadf(vec3f(0.0f), 1.0f, 1.0f),
            0);

    glrenderer2d_draw_frame_buffer(&frame->__fbo, quad); 
    

}

void frame_destroy(frame_t *frame)
{
    glframebuffer_destroy(&frame->__fbo);
}
