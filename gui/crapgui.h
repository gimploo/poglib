#pragma once

#include "./../simple/gl_renderer2d.h"
#include "./../simple/window.h"
#include "./../simple/font/gl_ascii_font.h"

/*===================================================================
 // Immediate style gui library
===================================================================*/

const char *const GUI_VS_SHADER_CODE = 
    "#version 330 core\n"
    "layout (location = 0) in vec3 v_pos;\n"
    "layout (location = 1) in vec3 v_color;\n"
    "\n"
    "out vec3 color;\n"
    "\n"
    "void main()\n"
    "{\n"
        "gl_Position = vec4(v_pos, 1.0f);\n"
        "color = v_color;\n"
    "}";

const char *const GUI_FS_SHADER_CODE = 
    "#version 330 core\n"
    "in vec3 color;\n"
    "\n"
    "out vec4 FragColor;\n"
    "\n"
    "void main()\n"
    "{\n"
        "FragColor = vec4(color, 1.0f);\n"
    "}";


typedef struct crapgui_t {
    
    window_t        *window_handle;
    gl_ascii_font_t *font_handle;
    gl_renderer2d_t renderer_handle;


} crapgui_t;


/*--------------------------------------------------------------------------------------
 // Declarations
--------------------------------------------------------------------------------------*/

crapgui_t       crapgui_init(window_t *window, gl_ascii_font_t *font);
void            crapgui_begin(crapgui_t *gui);
void            crapgui_end(crapgui_t *gui); 
void            crapgui_destroy(crapgui_t *gui);


/*--------------------------------------------------------------------------------------
 // Implementation
--------------------------------------------------------------------------------------*/

crapgui_t crapgui_init(window_t *window, gl_ascii_font_t *font)
{
    if (window == NULL) eprint("window argument is null");

    gl_shader_t *gui_shader = (gl_shader_t *)malloc(sizeof(gl_shader_t));
    if (gui_shader == NULL) eprint("malloc returned null");

    *gui_shader = gl_shader_from_cstr_init(GUI_VS_SHADER_CODE, GUI_FS_SHADER_CODE);

    return (crapgui_t ) {
        .window_handle      = window,
        .font_handle        = font,
        .renderer_handle    = gl_renderer2d_init(gui_shader, NULL),
    };
}

void crapgui_begin(crapgui_t *gui)
{
    window_gl_render_begin(gui->window_handle);
}

void crapgui_end(crapgui_t *gui)
{
    window_gl_render_end(gui->window_handle);
}

void crapgui_destroy(crapgui_t *gui)
{
    // shader
    gl_shader_destroy(gui->renderer_handle.shader);
    free(gui->renderer_handle.shader);

    // renderer
    gl_renderer2d_destroy(&gui->renderer_handle);
}

INTERNAL bool __is_mouse_over_quad(window_t *window, quadf_t norm_quad)
{
    vec2f_t mouse_position = window_mouse_get_norm_position(window);
    return quadf_is_point_in_quad(norm_quad, mouse_position);
}


/*================================================================================================
 // Buttons
================================================================================================*/

#define BUTTON_DEFAULT_WIDTH    0.3f
#define BUTTON_DEFAULT_HEIGHT   0.2f
#define BUTTON_DEFAULT_COLOR    (vec3f_t ){0.662f, 0.643f, 0.670f}

typedef struct button_t {

    const char *label;
    vec2f_t     norm_position;
    f32         width;
    f32         height;
    vec3f_t     norm_color;

    quadf_t     __quad_vertices;

} button_t;

/*------------------------------------------------------------------------
 // Declarations
------------------------------------------------------------------------*/

button_t        button_init(const char *label, vec2f_t norm_position);
void            button_draw(crapgui_t *gui, button_t *button);

//NOTE:(macro)  button_update_color(button_t *, vec3f_t) -> void
//NOTE:(macro)  button_update_label(button_t *, const char *) -> void
//NOTE:(macro)  button_update_norm_position(button_t *, vec2f_t) -> void

bool            button_is_mouse_over(crapgui_t *gui, button_t *button);
bool            button_is_mouse_clicked(crapgui_t *gui, button_t *button);
bool            button_is_mouse_dragging(crapgui_t *gui, button_t *button);

/*------------------------------------------------------------------------
 // Implementation
------------------------------------------------------------------------*/

button_t button_init(const char *label, vec2f_t norm_position)
{
    return (button_t ) {
        .label = label,
        .norm_position = norm_position,
        .width = BUTTON_DEFAULT_WIDTH,
        .height = BUTTON_DEFAULT_HEIGHT,
        .norm_color = BUTTON_DEFAULT_COLOR,
        .__quad_vertices = quadf_init(norm_position, BUTTON_DEFAULT_WIDTH, BUTTON_DEFAULT_HEIGHT),
    };
}

#define button_update_color(pbutton, rgb) (pbutton)->norm_color = rgb
#define button_update_label(pbutton, text) (pbutton)->label = text

#define button_update_norm_position(pbutton, vec2) do {\
    (pbutton)->norm_position    = vec2;\
    (pbutton)->__quad_vertices  = quadf_init(vec2, BUTTON_DEFAULT_WIDTH, BUTTON_DEFAULT_HEIGHT);\
} while(0)

void button_draw(crapgui_t *gui, button_t *button) 
{
    const gl_quad_t quad= gl_quad(button->__quad_vertices, button->norm_color, quadf(0));
    gl_renderer2d_draw_quad(&gui->renderer_handle, quad); 
    gl_ascii_font_render_text(
            gui->font_handle, 
            button->label, 
            vec2f_add(
                button->norm_position, 
                (vec2f_t ){ 0, (-button->height/2)}), 
            button->height/6);
}


bool button_is_mouse_over(crapgui_t *gui, button_t *button)
{
    vec2f_t mouse_position = window_mouse_get_norm_position(gui->window_handle);
    return quadf_is_point_in_quad(button->__quad_vertices, mouse_position);
}

bool button_is_mouse_clicked(crapgui_t *gui, button_t *button)
{
    return (window_mouse_button_just_pressed(gui->window_handle) && button_is_mouse_over(gui, button));
}

bool button_is_mouse_dragging(crapgui_t *gui, button_t *button)
{
    // NOTE: this code will have the button rendererd as the mouse at the center
    bool is_drag = (window_mouse_button_is_held(gui->window_handle) && button_is_mouse_over(gui, button));

    if (is_drag) 
    {
        vec2f_t mouse_position = window_mouse_get_norm_position(gui->window_handle);
        vec2f_t mouse_at_center_offset_position = {
            .cmp[X] = (mouse_position.cmp[X] - (BUTTON_DEFAULT_WIDTH/2)),
            .cmp[Y] = (mouse_position.cmp[Y] + (BUTTON_DEFAULT_HEIGHT/2))
        };
        button_update_norm_position(button, mouse_at_center_offset_position);
    } 
    return is_drag;
}


/*================================================================================================
 // Slider
================================================================================================*/

#define SLIDER_BODY_DEFAULT_WIDTH 0.5f
#define SLIDER_BODY_DEFAULT_HEIGHT 0.1f
#define SLIDER_BODY_DEFAULT_COLOR {0.0f, 1.0f, 1.0f}

#define SLIDER_BOX_DEFAULT_WIDTH 0.1f
#define SLIDER_BOX_DEFAULT_HEIGHT SLIDER_BODY_DEFAULT_HEIGHT
#define SLIDER_BOX_DEFAULT_COLOR {1.0f, 1.0f, 0.0f}

typedef struct slider_t {

    vec2f_t     body_norm_position;
    f32         body_width;
    f32         body_height;
    vec3f_t     body_norm_color;
    quadf_t     __body_vertices;

    vec2f_t     box_norm_position;
    f32         box_width;
    f32         box_height;
    vec3f_t     box_norm_color;
    quadf_t     __box_vertices;

    f32         value;
    vec2f_t     range;

} slider_t;


/*---------------------------------------------------------------
 // Declarations
---------------------------------------------------------------*/

slider_t        slider_init(vec2f_t range, vec2f_t norm_position);
//NOTE:(macro)  slider_get_value(slider_t *) -> f32
void            slider_draw(crapgui_t *gui, slider_t *slider);
bool            slider_box_is_mouse_dragging(crapgui_t *gui, slider_t *slider);


/*---------------------------------------------------------------
 // Implementation
---------------------------------------------------------------*/

INTERNAL bool __box_is_mouse_over(crapgui_t *gui, slider_t *slider)
{
    vec2f_t mouse_position = window_mouse_get_norm_position(gui->window_handle);
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

bool slider_box_is_mouse_dragging(crapgui_t *gui, slider_t *slider)
{
    bool is_mouse_dragging  = (window_mouse_button_is_held(gui->window_handle) && __box_is_mouse_over(gui, slider));

    vec2f_t mouse_position  = window_mouse_get_norm_position(gui->window_handle);

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

    gl_renderer2d_draw_from_batch(&gui->renderer_handle, &batch);

    char text[(int )slider->range.cmp[Y]];
    snprintf(text, sizeof(text), "%f", slider->value);
    const f32 font_size = SLIDER_BOX_DEFAULT_WIDTH / 4;

    gl_ascii_font_render_text(gui->font_handle, text, vec2f_add(slider->box_norm_position, (vec2f_t ){0.0f, -1 * SLIDER_BOX_DEFAULT_HEIGHT/ 2}), font_size);
}


/*================================================================================================
 // Label
================================================================================================*/

typedef struct label_t {

    const char  *string;
    u64         string_len;
    vec2f_t     norm_position;
    f32         norm_font_size;

} label_t;

/*-------------------------------------------------------------------------------------------------
 // Declarations
-------------------------------------------------------------------------------------------------*/

label_t         label_init(const char *value, vec2f_t norm_position, f32 norm_font_size);
//NOTE:(macro)  label_update_value(label_t *, const char *) -> void
void            label_draw(crapgui_t *gui, label_t *label);


/*-------------------------------------------------------------------------------------------------
 // Implementation
-------------------------------------------------------------------------------------------------*/

#define label_update_value(plabel, text) do {\
    (plabel)->string = text;\
    (plabel)->string_len = strlen(text);\
} while(0)


label_t label_init(const char *value, vec2f_t norm_position, f32 norm_font_size)
{
    return (label_t) {
        .string = value,
        .string_len = strlen(value),
        .norm_position = norm_position,
        .norm_font_size = norm_font_size
    };
}

bool label_is_mouse_dragging(crapgui_t *gui, label_t *label)
{
    vec2f_t norm_mouse_position  = window_mouse_get_norm_position(gui->window_handle);
    quadf_t quad = quadf_init(label->norm_position, label->norm_font_size * label->string_len, label->norm_font_size);

    bool is_mouse_dragging  = (window_mouse_button_is_held(gui->window_handle) && quadf_is_point_in_quad(quad, norm_mouse_position));

    if (is_mouse_dragging) 
    {
        vec2f_t mouse_at_center_offset_position = {
            .cmp[X] = norm_mouse_position.cmp[X] - (label->norm_font_size * label->string_len)/2,
            .cmp[Y] = norm_mouse_position.cmp[Y] + label->norm_font_size/2
        };

        // NOTE: updates the label position
        label->norm_position = mouse_at_center_offset_position;
    } 
    return is_mouse_dragging;
}

void label_draw(crapgui_t *gui, label_t *label)
{
    gl_ascii_font_render_text(gui->font_handle, label->string, label->norm_position, label->norm_font_size);
}

