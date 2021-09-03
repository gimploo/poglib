#include "./common.h"
#include "./frame.h"

INTERNAL bool __box_is_mouse_over(frame_t *frame, slider_t *slider)
{    
    //NOTE: this checks if the mouse is over the frame 
    if (!__frame_is_mouse_over(frame)) return false;

    vec2f_t mouse_position = __frame_get_mouse_position_relative_to_frame_coordinate_axis(frame);
    return quadf_is_point_in_quad(slider->__box_vertices, mouse_position);
}

#define slider_get_value(pslider) (pslider)->value;
#define slider_body_update_color(pslider, color)    (pslider)->body_norm_color = color;

#define slider_box_update_color(pslider, color)     (pslider)->box_norm_color = color;

slider_t slider_init(vec2f_t range, vec2f_t norm_position)
{
    return (slider_t ) {

        .body_norm_position = norm_position,
        .body_width         = SLIDER_BODY_DEFAULT_WIDTH,
        .body_height        = SLIDER_BODY_DEFAULT_HEIGHT,
        .body_norm_color    = SLIDER_BODY_DEFAULT_COLOR,
        .__body_vertices    = quadf_init(norm_position, SLIDER_BODY_DEFAULT_WIDTH, SLIDER_BODY_DEFAULT_HEIGHT),

        .box_norm_position  = norm_position,
        .box_width          = SLIDER_BOX_DEFAULT_WIDTH,
        .box_height         = SLIDER_BOX_DEFAULT_HEIGHT,
        .box_norm_color     = SLIDER_BOX_DEFAULT_COLOR,
        .__box_vertices     = quadf_init(norm_position, SLIDER_BOX_DEFAULT_WIDTH, SLIDER_BOX_DEFAULT_HEIGHT),

        .value              = 0.0f,
        .range              = range,

    };
}

bool slider_box_is_mouse_dragging(frame_t *frame, slider_t *slider)
{
    bool is_mouse_dragging  = (window_mouse_button_is_held(frame->gui_handler->window_handler) && __box_is_mouse_over(frame, slider));

    vec2f_t mouse_position  = __frame_get_mouse_position_relative_to_frame_coordinate_axis(frame);

    // NOTE: checks if the box goes out of the body of the slider
    if (mouse_position.cmp[X] <= (slider->__body_vertices.vertex[TOP_LEFT].cmp[X] + SLIDER_BOX_DEFAULT_WIDTH/2))
        mouse_position.cmp[X] = slider->body_norm_position.cmp[X] + SLIDER_BOX_DEFAULT_WIDTH/2;

    else if (mouse_position.cmp[X] >= (slider->__body_vertices.vertex[TOP_RIGHT].cmp[X] - SLIDER_BOX_DEFAULT_WIDTH/2))
        mouse_position.cmp[X] = (slider->__body_vertices.vertex[TOP_RIGHT].cmp[X] - SLIDER_BOX_DEFAULT_WIDTH/2);


    // NOTE: this code will have the slider box rendererd as the mouse at the center
    if (is_mouse_dragging) 
    {
        vec2f_t mouse_at_center_offset_position = {
            .cmp[X] = mouse_position.cmp[X] - (SLIDER_BOX_DEFAULT_WIDTH /2),
            .cmp[Y] = slider->box_norm_position.cmp[Y]
        };

        // NOTE: updates the box position
        slider->box_norm_position = mouse_at_center_offset_position;
        slider->__box_vertices = quadf_init(slider->box_norm_position, SLIDER_BOX_DEFAULT_WIDTH, SLIDER_BOX_DEFAULT_HEIGHT);

        //NOTE: updating the value
        f32 last_x_position     = mouse_position.cmp[X] + SLIDER_BOX_DEFAULT_WIDTH/2;
        f32 first_x_position    = slider->body_norm_position.cmp[X];
        f32 offset_per_box      = SLIDER_BODY_DEFAULT_WIDTH / slider->range.cmp[Y];
        f32 new_value           = (last_x_position - first_x_position) / offset_per_box;
        slider->value           = new_value - 1;

    } 
    return is_mouse_dragging;
}

void slider_draw(crapgui_t *gui, slider_t *slider)
{
    const gl_quad_t body    = gl_quad(slider->__body_vertices, slider->body_norm_color, quadf(0));
    const gl_quad_t box     = gl_quad(slider->__box_vertices, slider->box_norm_color, quadf(0));

    const gl_quad_t buffer[2] = { body, box };

    gl_batch_t batch = (gl_batch_t ){
        .shape_type         = BT_QUAD,
        .shape_count        = 2,
        .vertex_buffer      = (gl_vertex_t *)buffer,
        .vertex_buffer_size = sizeof(buffer)
    };

    gl_renderer2d_draw_from_batch(&gui->renderer_handler, &batch);

    char text[(int )slider->range.cmp[Y]];
    snprintf(text, sizeof(text), "%f", slider->value);
    const f32 font_size = SLIDER_BOX_DEFAULT_WIDTH / 4;

    gl_ascii_font_render_text(gui->font_handler, text, vec2f_add(slider->box_norm_position, (vec2f_t ){0.0f, -1 * SLIDER_BOX_DEFAULT_HEIGHT/ 2}), font_size);
}
