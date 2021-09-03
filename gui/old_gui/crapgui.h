#pragma once

#include "./../simple/gl_renderer2d.h"
#include "./../simple/window.h"
#include "./../simple/font/gl_ascii_font.h"

/*===================================================================
 // crapgui is an immediate style gui library
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


//NOTE: gui header
typedef struct crapgui_t {
    
    window_t        *window_handler;
    gl_ascii_font_t *font_handler;
    gl_renderer2d_t renderer_handler;

} crapgui_t;

//NOTE: the type of ui components currently supported
typedef enum ui_component_type {

    UI_FRAME = 0,
    UI_MENU_BAR,
    UI_BUTTON,
    UI_SLIDER,
    UI_LABEL,
    UI_COUNT

} ui_component_type;

typedef struct ui_component_t {

    void                *component_addr;
    ui_component_type   type;

} ui_component_t;

/*--------------------------------------------------------------------------------------
 // Declarations
--------------------------------------------------------------------------------------*/

crapgui_t       crapgui_init(window_t *window, gl_ascii_font_t *font);
void            crapgui_begin_ui(crapgui_t *gui);
void            crapgui_end_ui(crapgui_t *gui); 
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
        .window_handler      = window,
        .font_handler        = font,
        .renderer_handler    = gl_renderer2d_init(gui_shader, NULL),
    };
}

void crapgui_begin_ui(crapgui_t *gui)
{
    window_gl_render_begin(gui->window_handler);
}

void crapgui_end_ui(crapgui_t *gui)
{
    window_gl_render_end(gui->window_handler);
}

void crapgui_destroy(crapgui_t *gui)
{
    // shader
    gl_shader_destroy(gui->renderer_handler.shader);
    free(gui->renderer_handler.shader);

    // renderer
    gl_renderer2d_destroy(&gui->renderer_handler);
}




/*=================================================================================
 // Frame
=================================================================================*/

#define FRAME_DEFAULT_COLOR                 (vec3f_t ){0.2f, 0.2f, 0.2f}
#define FRAME_DEFAULT_UI_COMPONENT_COUNT    10 

typedef struct frame_t {

    crapgui_t           *gui_handler; 
    vec2f_t             norm_position;
    f32                 norm_width;
    f32                 norm_height;
    vec3f_t             norm_color;
    quadf_t             __quad_vertices;
    gl_framebuffer_t    fbo;

    bool                is_open;
    stack_t             ui_components;

} frame_t;


//NOTE: this array holds the address of the ui components and not the components itself

ui_component_t FRAME_STACK_ARRAY[FRAME_DEFAULT_UI_COMPONENT_COUNT];

/*-------------------------------------------------------------------------------
 // Declarations
-------------------------------------------------------------------------------*/

frame_t         frame_init(crapgui_t *gui, vec2f_t norm_position, f32 norm_width, f32 norm_height);

void            frame_begin(frame_t *frame);
void            frame_end(frame_t *frame);

void            frame_draw(frame_t *frame);
void            frame_destroy(frame_t *frame);

/*-------------------------------------------------------------------------------
 // Implementation
-------------------------------------------------------------------------------*/

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
    return (frame_t) {
        .gui_handler = gui,
        .norm_position = norm_position,
        .norm_width = norm_width,
        .norm_height = norm_height,
        .norm_color  = FRAME_DEFAULT_COLOR,
        .__quad_vertices = quadf_init(norm_position, norm_width, norm_height),
        .fbo =  gl_framebuffer_init(gui->window_handler->width, gui->window_handler->height),
        .is_open = true,
        .ui_components = stack_init((void **)FRAME_STACK_ARRAY, FRAME_DEFAULT_UI_COMPONENT_COUNT),
    };
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
    
    //Menu bar
    menu_bar_draw(frame->norm_position, MENU_BAR_DEFAULT_WIDTH, MENU_BAR_DEFAULT_HEIGHT);

}

void frame_destroy(frame_t *frame)
{
    gl_framebuffer_destroy(&frame->fbo);
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

bool            button_is_mouse_over(frame_t *frame, button_t *button);
bool            button_is_mouse_clicked(frame_t *frame, button_t *button);
bool            button_is_mouse_dragging(frame_t *frame, button_t *button);

/*------------------------------------------------------------------------
 // Implementation
------------------------------------------------------------------------*/

button_t button_init(const char *label, vec2f_t norm_position)
{
    return (button_t ){
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
    gl_renderer2d_draw_quad(&gui->renderer_handler, quad); 
    gl_ascii_font_render_text(
            gui->font_handler, 
            button->label, 
            vec2f_add(
                button->norm_position, 
                (vec2f_t ){ 0, (-button->height/2)}), 
            button->height/6);
}


bool button_is_mouse_over(frame_t *frame, button_t *button)
{
    //NOTE: this checks if the mouse is over the frame 
    if (!__frame_is_mouse_over(frame)) return false;

    vec2f_t mouse_position = __frame_get_mouse_position_relative_to_frame_coordinate_axis(frame);
    return quadf_is_point_in_quad(button->__quad_vertices, mouse_position);
}

bool button_is_mouse_clicked(frame_t *frame, button_t *button)
{
    return (window_mouse_button_just_pressed(frame->gui_handler->window_handler) && button_is_mouse_over(frame, button));
}

bool button_is_mouse_dragging(frame_t *frame, button_t *button)
{
    // NOTE: this code will have the button rendererd as the mouse at the center
    bool is_drag = (window_mouse_button_is_held(frame->gui_handler->window_handler) && button_is_mouse_over(frame, button));

    if (is_drag) 
    {
        vec2f_t mouse_position = __frame_get_mouse_position_relative_to_frame_coordinate_axis(frame);
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
void            slider_draw(crapgui_t *gui, slider_t *slider);

//NOTE:(macro)  slider_get_value(slider_t *) -> f32
//NOTE:(macro)  slider_body_update_color(slider_t *, vec3f_t) -> f32
//NOTE:(macro)  slider_box_update_color(slider_t *, vec3f_t) -> f32

bool            slider_box_is_mouse_dragging(frame_t *frame, slider_t *slider);


/*---------------------------------------------------------------
 // Implementation
---------------------------------------------------------------*/

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
void            label_draw(crapgui_t *gui, label_t *label);

//NOTE:(macro)  label_update_value(label_t *, const char *) -> void


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

void label_draw(crapgui_t *gui, label_t *label)
{
    gl_ascii_font_render_text(gui->font_handler, label->string, label->norm_position, label->norm_font_size);
}





