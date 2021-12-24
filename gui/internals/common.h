#pragma once

//TODO: memory manage the allocated stack array in drop down list
//TODO: memory manage the allocated stack array in menu bar


#include "./../../simple/gl_renderer2d.h"
#include "./../../simple/window.h"
#include "./../../simple/font/gl_ascii_font.h"
#include "./../../color.h"

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

typedef struct button_t     button_t;
typedef struct label_t      label_t;
typedef struct frame_t      frame_t;
typedef struct slider_t     slider_t;

/*================================================================================================
 // Crapgui Header
================================================================================================*/

typedef struct crapgui_t {
    
    window_t        *window_handler;
    gl_ascii_font_t *font_handler;
    glrenderer2d_t renderer_handler;

} crapgui_t;


crapgui_t       crapgui_init(window_t *window, gl_ascii_font_t *font);
void            crapgui_begin_ui(crapgui_t *gui);
void            crapgui_end_ui(crapgui_t *gui); 
void            crapgui_destroy(crapgui_t *gui);


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
    ui_component_t      __stack_array[FRAME_DEFAULT_UI_COMPONENT_COUNT];

} frame_t;


//NOTE: this array holds the address of the ui components and not the components itself


frame_t         frame_init(crapgui_t *gui, vec2f_t norm_position, f32 norm_width, f32 norm_height);

void            frame_begin(frame_t *frame);
void            frame_end(frame_t *frame);

void            frame_draw(frame_t *frame);
void            frame_destroy(frame_t *frame);


/*================================================================================================
 // Buttons
================================================================================================*/

#define BUTTON_DEFAULT_WIDTH        0.3f
#define BUTTON_DEFAULT_HEIGHT       0.2f
#define BUTTON_DEFAULT_COLOR        (vec3f_t ){0.662f, 0.643f, 0.670f}

typedef struct button_t {

    const char *label;
    vec2f_t     norm_position;
    f32         norm_width;
    f32         norm_height;
    vec3f_t     norm_color;

    quadf_t     __quad_vertices;

} button_t;


button_t        button_init(const char *label, vec2f_t norm_position);
void            button_draw(crapgui_t *gui, button_t *button);

//NOTE:(macro)  button_update_color(button_t *, vec3f_t) -> void
//NOTE:(macro)  button_update_label(button_t *, const char *) -> void
//NOTE:(macro)  button_update_norm_position(button_t *, vec2f_t) -> void

bool            button_is_mouse_over(frame_t *frame, button_t *button);
bool            button_is_mouse_clicked(frame_t *frame, button_t *button);
bool            button_is_mouse_dragging(frame_t *frame, button_t *button);


/*================================================================================================
 // Label
================================================================================================*/

typedef struct label_t {

    const char  *string;
    u64         string_len;
    vec2f_t     norm_position;
    f32         norm_font_size;

} label_t;

label_t         label_init(const char *value, vec2f_t norm_position, f32 norm_font_size);
void            label_draw(crapgui_t *gui, label_t *label);

//NOTE:(macro)  label_update_value(label_t *, const char *) -> void



/*================================================================================================
 // Slider
================================================================================================*/

#define SLIDER_BODY_DEFAULT_WIDTH   0.5f
#define SLIDER_BODY_DEFAULT_HEIGHT  0.1f
#define SLIDER_BODY_DEFAULT_COLOR   {0.0f, 1.0f, 1.0f}
#define SLIDER_BOX_DEFAULT_WIDTH    0.1f
#define SLIDER_BOX_DEFAULT_HEIGHT   SLIDER_BODY_DEFAULT_HEIGHT
#define SLIDER_BOX_DEFAULT_COLOR    {1.0f, 1.0f, 0.0f}

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


slider_t        slider_init(vec2f_t range, vec2f_t norm_position);
void            slider_draw(crapgui_t *gui, slider_t *slider);

//NOTE:(macro)  slider_get_value(slider_t *) -> f32
//NOTE:(macro)  slider_body_update_color(slider_t *, vec3f_t) -> f32
//NOTE:(macro)  slider_box_update_color(slider_t *, vec3f_t) -> f32

bool            slider_box_is_mouse_dragging(frame_t *frame, slider_t *slider);



/*=================================================================================
 // Drop down list
=================================================================================*/


#define DROP_DOWN_LIST_MAX_BUTTON_COUNT 10

typedef struct drop_down_list_t {

    const char          *label;
    vec2f_t             norm_position;
    f32                 norm_width;
    f32                 norm_height;
    vec3f_t             norm_color;
    quadf_t             __quad_vertices;

    button_t            *__stack_array;
    stack_t             buttons;
    bool                is_drop_down;

} drop_down_list_t;

drop_down_list_t    drop_down_list_init(const char *label, vec2f_t norm_position, f32 norm_width, f32 norm_height, vec3f_t norm_color);

void                drop_down_list_push_button(drop_down_list_t *list, const char *label);
bool                drop_down_list_is_mouse_over(frame_t *frame, drop_down_list_t *list);
bool                drop_down_list_is_clicked(frame_t *frame, drop_down_list_t *list);

void                drop_down_list_draw(crapgui_t *gui, drop_down_list_t *list);



/*=================================================================================
 // Menu bar
=================================================================================*/

#define MENU_BAR_DEFAULT_WIDTH              2.0f
#define MENU_BAR_DEFAULT_HEIGHT             0.1f

typedef struct menu_bar_t {

    vec2f_t             norm_position;
    f32                 norm_width;
    f32                 norm_height;
    vec3f_t             norm_color;
    quadf_t             __quad_vertices;
    
    drop_down_list_t    *__stack_array;
    stack_t             drop_down_lists;

} menu_bar_t;

menu_bar_t      menu_bar_init(vec2f_t norm_position, u32 drop_down_list_count);

void            menu_bar_push_drop_down_list(menu_bar_t *menu_bar, drop_down_list_t *list);
bool            menu_bar_is_mouse_over(frame_t *frame, menu_bar_t *menu_bar);
bool            menu_bar_is_clicked(frame_t *frame, menu_bar_t *menu_bar);

menu_bar_t      menu_bar_draw(crapgui_t *gui, menu_bar_t *menu_bar);


