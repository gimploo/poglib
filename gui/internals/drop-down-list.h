#pragma once

#include "./common.h"
#include "./frame.h"
#include "./button.h"

drop_down_list_t drop_down_list_init(const char *label, vec2f_t norm_position, f32 norm_width, f32 norm_height, vec3f_t norm_color)
{
    drop_down_list_t bar = {
        .label = label,
        .norm_position = norm_position,
        .norm_width = norm_width,
        .norm_height = norm_height,
        .norm_color = norm_color,
        .__stack_array = (button_t *)malloc(sizeof(button_t) * DROP_DOWN_LIST_MAX_BUTTON_COUNT),
        .is_drop_down = false,
    };

    bar.buttons         = stack_dynamic_array_init(bar.__stack_array, sizeof(button_t) * DROP_DOWN_LIST_MAX_BUTTON_COUNT, DROP_DOWN_LIST_MAX_BUTTON_COUNT);
    bar.__quad_vertices = quadf_init(norm_position, norm_width, norm_height);

    return bar;
}

void drop_down_list_push_button(drop_down_list_t *list, const char *label)
{
    i64 offset = list->buttons.top + 2;

    button_t button = {
        .label = label,
        .norm_position = vec2f_add(list->norm_position, (vec2f_t ){0.0f, -(list->norm_height * offset)}),
        .norm_width = list->norm_width,
        .norm_height = list->norm_height,
        .norm_color = list->norm_color,
    };
    button.__quad_vertices = quadf_init(
            button.norm_position, 
            button.norm_width, 
            button.norm_height);

    stack_push_by_value(&list->buttons, button); 
}

bool drop_down_list_is_mouse_over(frame_t *frame, drop_down_list_t *list)
{
    if (!__frame_is_mouse_over(frame)) return false;

    vec2f_t mouse_position = __frame_get_mouse_position_relative_to_frame_coordinate_axis(frame);
    return quadf_is_point_in_quad(list->__quad_vertices, mouse_position);
}

INTERNAL void __drop_down_list_update_buttons(frame_t *frame, drop_down_list_t *list)
{
    //NOTE: this updates the color of the button
    
    button_t *button = NULL;
    if (list->is_drop_down)
    {
        for (int i = 0; i <= list->buttons.top; i++)
        {
            button = (button_t *)list->buttons.array + i;
            if (button_is_mouse_over(frame, button))
                button_update_color(button, BLUE_COLOR);
        }
    }
}

bool drop_down_list_is_clicked(frame_t *frame, drop_down_list_t *list)
{
    bool output = ( window_mouse_button_just_pressed(frame->gui_handler->window_handler) 
            && drop_down_list_is_mouse_over(frame, list));

    if (output) {
        printf("drop_down_list pressed\n");
        list->is_drop_down = !list->is_drop_down;
    } 
    __drop_down_list_update_buttons(frame, list);

    return output;
}

void drop_down_list_draw(crapgui_t *gui, drop_down_list_t *list)
{
    u32 total_quads = list->buttons.top + 2;

    // Here it was supposed to be of total_quads size but in msvc gives an error, so i did this instead
    gl_quad_t quads[KB] = {0};

    //NOTE: the drop list header
    quads[0] = gl_quad( list->__quad_vertices, list->norm_color, quadf(0.0f));

    if (list->is_drop_down) {

        stack_t *stack = &list->buttons;
        for(int i = 0; i <= stack->top; i++)
        {
            button_t *button = (button_t *)stack->array + i;
            quads[i+1] = gl_quad(
                    button->__quad_vertices,
                    button->norm_color,
                    quadf(0.0f));
            //printf(QUAD_FMT"\n", QUAD_ARG(button->__quad_vertices));
        }

        gl_batch_t batch = {
            .shape_type         = BT_QUAD,
            .shape_count        = total_quads,
            .vertex_buffer      = (gl_vertex_t *)quads,
            .vertex_buffer_size = sizeof(quads),
        };
        gl_renderer2d_draw_from_batch(&gui->renderer_handler, &batch);

    } else {

        gl_renderer2d_draw_quad(&gui->renderer_handler, quads[0]);

    }

    //gl_ascii_font_render_text(
            //gui->font_handler,
            //list->label, 
            //vec2f_add(
                //list->norm_position, 
                //(vec2f_t ){ 0, (-list->norm_height/2)}), 
            //list->norm_height/6);
}
