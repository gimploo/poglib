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


/*------------------------------------------------------------------------------------------------
 // Buttons
------------------------------------------------------------------------------------------------*/

#define BUTTON_DEFAULT_WIDTH    0.3f
#define BUTTON_DEFAULT_HEIGHT   0.2f
#define BUTTON_DEFAULT_COLOR    (vec3f_t ){0.662f, 0.643f, 0.670f}

typedef struct button_t {

    const char *label;
    vec2f_t     norm_position;
    f32         width;
    f32         height;
    vec3f_t     norm_color;

    quadf_t     __quad;

    // caching button state
    bool is_hot;
    bool is_active;
    bool is_dragged;

} button_t;

button_t    button_init(const char *label, vec2f_t norm_position);
void        button_begin(crapgui_t *gui, button_t *button);
bool        button_is_mouse_over(crapgui_t *gui, button_t *button);
bool        button_is_mouse_clicked(crapgui_t *gui, button_t *button);
bool        button_is_mouse_dragging(crapgui_t *gui, button_t *button);

button_t button_init(const char *label, vec2f_t norm_position)
{
    return (button_t ) {
        .label = label,
        .norm_position = norm_position,
        .width = BUTTON_DEFAULT_WIDTH,
        .height = BUTTON_DEFAULT_HEIGHT,
        .norm_color = BUTTON_DEFAULT_COLOR,
        .__quad = quadf_init(norm_position, BUTTON_DEFAULT_WIDTH, BUTTON_DEFAULT_HEIGHT),
        .is_hot = false,
        .is_active = false,
        .is_dragged = false,
    };
}

#define button_update_color(pbutton, rgb) (pbutton)->norm_color = rgb
#define button_update_label(pbutton, text) (pbutton)->label = text

#define button_update_norm_position(pbutton, vec2) do {\
    (pbutton)->norm_position = vec2;\
    (pbutton)->__quad = quadf_init(vec2, BUTTON_DEFAULT_WIDTH, BUTTON_DEFAULT_HEIGHT);\
} while(0)

void button_draw(crapgui_t *gui, button_t *button) 
{

    quadf_t vertices;

    const gl_quad_t quad= gl_quad(button->__quad, button->norm_color, quadf(0));
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
    button->is_hot =  quadf_is_point_in_quad(button->__quad, mouse_position);
    return button->is_hot;
}

bool button_is_mouse_clicked(crapgui_t *gui, button_t *button)
{
    button->is_active = (window_mouse_button_just_pressed(gui->window_handle) && button_is_mouse_over(gui, button));
    return button->is_active;
}

bool button_is_mouse_dragging(crapgui_t *gui, button_t *button)
{
    // NOTE: this code will have the button rendererd as the mouse at the center
    button->is_dragged = (window_mouse_button_is_held(gui->window_handle) && button_is_mouse_over(gui, button));
    if (button->is_dragged) {

        vec2f_t mouse_position = window_mouse_get_norm_position(gui->window_handle);
        vec2f_t mouse_at_center_offset_position = {
            .cmp[X] = (mouse_position.cmp[X] - (BUTTON_DEFAULT_WIDTH/2)),
            .cmp[Y] = (mouse_position.cmp[Y] + (BUTTON_DEFAULT_HEIGHT/2))
        };
        button_update_norm_position(button, mouse_at_center_offset_position);
    } 
    return button->is_dragged;
}


/*------------------------------------------------------------------------------------------
 // Slider
------------------------------------------------------------------------------------------*/

typedef struct slider_t {

    vec2f_t     norm_position;
    f32         width;
    f32         height;
    vec3f_t     norm_color;

    quadf_t     __quad;

    f32         value;
    vec2f_t     range;


} slider_t;

slider_t slider_begin(vec2f_t range, vec2f_t norm_position)
{
    return (slider_t ) {
        .norm_position = norm_position,
        .width = BUTTON_DEFAULT_WIDTH,
        .height = BUTTON_DEFAULT_HEIGHT,
        .norm_color = BUTTON_DEFAULT_COLOR,
        .__quad = quadf_init(norm_position, BUTTON_DEFAULT_WIDTH, BUTTON_DEFAULT_HEIGHT),

        .value = ((range.cmp[X] + range.cmp[Y]) / 2),
        .range = range,
    };
}

#define slider_update_color(pslider, color) (pslider)->norm_color = color;
#define slider_update_value(pslider, value) (pslider)->value = value;

bool slider_is_mouse_over(crapgui_t *gui, slider_t *slider)
{
    vec2f_t mouse_position = window_mouse_get_norm_position(gui->window_handle);
    return quadf_is_point_in_quad(slider->__quad, mouse_position);
}

bool slider_is_mouse_clicked(crapgui_t *gui, slider_t *slider)
{
    return (slider_is_mouse_over(gui, slider) && window_mouse_button_just_pressed(gui->window_handle));
}

void slider_end(crapgui_t *gui, slider_t *slider) 
{
    const gl_quad_t quad = gl_quad(slider->__quad, slider->norm_color, quadf(0));
    gl_renderer2d_draw_quad(&gui->renderer_handle, quad); 
    gl_ascii_font_render_text(
            gui->font_handle, 
            "?",
            vec2f_add(
                slider->norm_position, 
                (vec2f_t ){ 0, (-slider->height/2)}), 
            slider->height/6);
}

