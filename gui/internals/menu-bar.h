#pragma once

#include "./common.h"
#include "./drop-down-list.h"


menu_bar_t menu_bar_init(vec2f_t norm_position, u32 drop_down_list_count) 
{
    menu_bar_t bar = {
        .norm_position = norm_position,
        .norm_width = MENU_BAR_DEFAULT_WIDTH,
        .norm_height = MENU_BAR_DEFAULT_HEIGHT,
        .norm_color = WHITE_COLOR,
        .__stack_array = (drop_down_list_t *)malloc(sizeof(void *) * drop_down_list_count)
    };

    bar.drop_down_lists = stack_dynamic_array_init((void **)bar.__stack_array, sizeof(void *) * drop_down_list_count, drop_down_list_count);
    bar.__quad_vertices = quadf_init(norm_position, bar.norm_width, bar.norm_height);

    return bar;
}



void menu_bar_push_drop_down_list(menu_bar_t *menu_bar, drop_down_list_t *list)
{
    i64 offset = menu_bar->drop_down_lists.top + 1;

    list->norm_width        = list->norm_width;
    list->norm_height       = menu_bar->norm_height;

    list->norm_position = vec2f_add(
            menu_bar->norm_position, 
            (vec2f_t ){ (list->norm_width * offset), 0.0f });

    list->__quad_vertices  = quadf_init(list->norm_position, list->norm_width, list->norm_height);

    stack_push_by_ref(&menu_bar->drop_down_lists, list); 
}



bool menu_bar_is_mouse_over(frame_t *frame, menu_bar_t *menu_bar)
{
    for (int i = 0; i <= menu_bar->drop_down_lists.top; i++)
    {
        drop_down_list_t *drop_down = (drop_down_list_t *)menu_bar->drop_down_lists.array[i];
        if (drop_down_list_is_mouse_over(frame, drop_down))
        {
            // ...
        }
    }
}



bool menu_bar_is_clicked(frame_t *frame, menu_bar_t *menu_bar)
{
    for (int i = 0; i <= menu_bar->drop_down_lists.top; i++)
    {
        drop_down_list_t *drop_down = (drop_down_list_t *)menu_bar->drop_down_lists.array[i];
        if (drop_down_list_is_clicked(frame, drop_down))
        {
            // ...
        }
    }
    
}



menu_bar_t menu_bar_draw(crapgui_t *gui, menu_bar_t *menu_bar)
{
    gl_quad_t quad = gl_quad(menu_bar->__quad_vertices, menu_bar->norm_color, quadf(0.0f));

    gl_renderer2d_draw_quad(&gui->renderer_handler, quad);
    for(int i = 0; i <= menu_bar->drop_down_lists.top; i++)
    {
        drop_down_list_t *drop_down = (drop_down_list_t *)menu_bar->drop_down_lists.array[i];
        drop_down_list_draw(gui, drop_down); 
    }
}

