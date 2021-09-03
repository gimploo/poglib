#pragma once

#include "./common.h"

INTERNAL bool __frame_is_mouse_over(frame_t *frame)
{
    vec2f_t norm_mouse_position = window_mouse_get_norm_position(frame->gui_handler->window_handler);
    return quadf_is_point_in_quad(frame->__quad_vertices, norm_mouse_position);
}

INTERNAL vec2f_t __frame_get_mouse_position_relative_to_frame_coordinate_axis(frame_t *frame)
{
    //NOTE: here we convert the mouse position in the frame to the window position, 
    //because all the ui components holds the quad vertices relative to the window and not to the frame 

    vec2i_t mouse_position = window_mouse_get_position(frame->gui_handler->window_handler);

    u32 frame_width = ((frame->norm_width)/2.0f) * frame->gui_handler->window_handler->width;
    u32 frame_height = ((frame->norm_height)/2.0f) * frame->gui_handler->window_handler->height;

    vec2f_t output = {
         .cmp[X] = (f32) (-1.0 + 2.0 *  (f32) mouse_position.cmp[X] / frame_width),
         .cmp[Y] = (f32) (1.0 - 2.0 * (f32) mouse_position.cmp[Y] / frame_height),
    };

    return output;
}

frame_t frame_init(crapgui_t *gui, vec2f_t norm_position, f32 norm_width, f32 norm_height)
{
    frame_t frame = {
        .gui_handler = gui,
        .norm_position = norm_position,
        .norm_width = norm_width,
        .norm_height = norm_height,
        .norm_color  = FRAME_DEFAULT_COLOR,
        .__quad_vertices = quadf_init(norm_position, norm_width, norm_height),
        .fbo =  gl_framebuffer_init(gui->window_handler->width, gui->window_handler->height),
        .is_open = true,
    };
    frame.ui_components = stack_static_array_init(frame.__stack_array, FRAME_DEFAULT_UI_COMPONENT_COUNT);
    return frame;
}


void frame_begin(frame_t *frame)
{
    glClearColor(
            frame->norm_color.cmp[X], 
            frame->norm_color.cmp[Y],
            frame->norm_color.cmp[Z],
            0.0f);
    gl_framebuffer_begin_scene(&frame->fbo);
}


void frame_end(frame_t *frame)
{
    gl_framebuffer_end_scene(&frame->fbo);
}

void frame_draw(frame_t *frame)
{
    gl_quad_t quad = gl_quad(
            quadf_init(frame->norm_position, frame->norm_width, frame->norm_height),
            frame->norm_color, 
            quadf_init((vec2f_t ){0.0f, 1.0f}, 1.0f, 1.0f));

    gl_renderer2d_draw_frame_buffer(&frame->fbo, quad); 
    

}

void frame_destroy(frame_t *frame)
{
    gl_framebuffer_destroy(&frame->fbo);
}
