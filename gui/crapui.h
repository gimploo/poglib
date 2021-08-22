#ifndef __MY__CRAP__UI__H__
#define __MY__CRAP__UI__H__

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "../basic.h"
#include "../ds/stack.h"
#include "../math/la.h"

#include "../simple/gl_renderer.h"
#include "../simple/window.h"

#include "../simple/font/gl_ascii_font.h"

/*===============================================================
 //                     CrapGui
=============================================================== */

typedef struct crapgui_t {

    window_t        *sub_window;
    gl_ascii_font_t font; 
    gl_renderer2d_t *renderer;
    u64             ui_component_count;

} crapgui_t;

/*--------------------------------------------
 // Declarations
--------------------------------------------*/

crapgui_t   gui_init(window_t *window);
void        gui_begin(crapgui_t *gui);
void        gui_end(crapgui_t *gui);


/*--------------------------------------------
 // Implementations
--------------------------------------------*/

// FIXME: how to manage fonts
crapgui_t gui_init(window_t *window)
{
    return (crapgui_t) {
        //.sub_window = window_init(540, 460, SDL_INIT_VIDEO),
        .sub_window = window,
        //.font       = gl_ascii_font_init(
                        //"/home/gokul/Documents/projects/poglib/res/ascii_fonts/glyph_atlas.png", 
                        //16, 6),
        .ui_component_count = 0,
    };
}

void gui_begin(crapgui_t *gui)
{
    if (gui == NULL) eprint("gui argument is null");

    //window_set_background(&gui->sub_window, (vec4f_t) {0.0f, 0.0f, 1.0f, 1.0f}); 
    //window_process_user_input(&gui->sub_window);
    //window_render(&gui->sub_window, NULL, NULL);

}
void gui_end(crapgui_t *gui)
{
    if (gui == NULL) eprint("gui argument is null");

    //window_destroy(&gui->sub_window);
    gl_ascii_font_destroy(&gui->font);

}


/*=========================================================
 //                 Button
=========================================================*/

// OpenGL vertices index position in buffer
#define TOP_LEFT        0
#define TOP_RIGHT       1
#define BOTTOM_RIGHT    2
#define BOTTOM_LEFT     3

#define DEFAULT_COLOR   (vec4f_t) {0.4f, 0.3f, 0.8f, 1.0f}
#define DEFAULT_POS     (vec2f_t) {-0.8f, +0.8f}
#define DEFAULT_WIDTH   0.3f
#define DEFAULT_HEIGHT  0.2f


typedef struct button_t {

    const char  *label;
    bool        is_hot;
    bool        is_active;

    vec2f_t     position; 
    f32         width, height;
    vec4f_t     color;

    vao_t       vao;
    vbo_t       vbo;
    ebo_t       ebo;
    vec3f_t     vertices[4];
    gl_shader_t shader;

} button_t ;

const u32 RECT_INDICES[] = {          
    0, 1, 3, // first triangle  
    1, 2, 3  // second triangle 
};                                  

const char *vertex_source = 
"#version 430 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"    gl_Position = vec4(aPos, 1.0f);\n"
"}";

const char *fragment_source = 
"#version 430 core\n"
"out vec4 FragColor;\n"
"uniform vec4 u_color;\n"
"void main(void)\n"
"{\n"
"    FragColor = u_color;\n"
"}\n";


/*---------------------------
// Declarations
---------------------------*/

button_t    button_begin(crapgui_t *const gui, const char *label);

#define     button_set_width(pbutton, width)    ((pbutton)->width = width)
#define     button_set_height(pbutton,height)   ((pbutton)->height = height)
void        button_set_position(const crapgui_t *gui, button_t *button, vec2f_t position);
void        button_set_color(const crapgui_t *gui, button_t *button, vec4f_t color);

bool        button_is_pressed(const crapgui_t *gui, button_t *button);
bool        button_is_dragged(const crapgui_t *gui, button_t *button); 
bool        button_is_mouseover(const crapgui_t *gui, button_t *button);

#define     button_end(pbutton) (__button_destroy(pbutton))



/*---------------------------
// Implementations
---------------------------*/

button_t __button_init(const crapgui_t *gui, const char *label, vec4f_t color, vec2f_t position, f32 width, f32 height) 
{
    button_t button = {
        .label = label,
        .is_hot = false, 
        .is_active = false,
        .position = position,
        .width = width,
        .height = height,
        .color = color,
        .vao    = vao_init(1),
        .shader = gl_shader_from_cstr_init(vertex_source, fragment_source),
    };

    vec3f_t vertices[4]; 

    // top left
    memcpy(&vertices[0], &button.position, sizeof(button.position)); 
    vertices[0].cmp[Z] = 0.0f; 
    
    // top right
    vertices[1] = (vec3f_t) {     
        button.position.cmp[X] + button.width,
        button.position.cmp[Y],
        0.0f
    }; 

    // bottom right
    vertices[2] = (vec3f_t) {     
        button.position.cmp[X] + button.width,
        button.position.cmp[Y] - button.height,
        0.0f
    }; 


    // bottom left
    vertices[3] = (vec3f_t) {     
        button.position.cmp[X],
        button.position.cmp[Y] - button.height,
        0.0f
    }; 

    memcpy(button.vertices, vertices, sizeof(vertices));

    gl_shader_push_uniform(&button.shader, "u_color", &button.color, sizeof(button.color), UT_VEC4F);
    

    vao_bind(&button.vao);
    {
        button.vbo = vbo_init(button.vertices, sizeof(button.vertices));
        button.ebo = ebo_init(&button.vbo, RECT_INDICES, 6);

        GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0));
        GL_CHECK(glEnableVertexAttribArray(0));


    }
    vao_unbind();


    return button;
} 




static inline void __button_draw(const crapgui_t *gui, button_t * button)
{
    if (button == NULL) eprint("button argument is null");

    u32 indices[] = {          
        0, 1, 2, // first triangle  
        2, 3, 0  // second triangle 
    };                                  

    vec2f_t text_position;
    text_position.cmp[X] = button->vertices[0].cmp[X];
    text_position.cmp[Y] = (button->vertices[0].cmp[Y] + button->vertices[2].cmp[Y]) /2;

    vao_bind(&button->vao);
    {

        gl_shader_bind(&button->shader);
        GL_CHECK(glDrawElements(GL_TRIANGLES, button->vbo.indices_count, GL_UNSIGNED_INT, 0));

        // Renders the font
        gl_ascii_font_render_text(&gui->font, button->label, text_position, 0.1f);
    }
    vao_unbind();
}

button_t button_begin(crapgui_t *const gui, const char *label) 
{ 
    if (gui == NULL) eprint("gui argument is null");
    if (label == NULL) eprint("label argument is null");

    vec2f_t generate_position = vec2f_add( 
                DEFAULT_POS, 
                (vec2f_t) {0.0f, -(DEFAULT_WIDTH * gui->ui_component_count)} 
    );
    
    button_t button = __button_init(gui, label, DEFAULT_COLOR, generate_position, DEFAULT_WIDTH, DEFAULT_HEIGHT);

    __button_draw(gui, &button);

    gui->ui_component_count++;

    return button;
}

static void __button_destroy(button_t *button)
{
    if (button == NULL) eprint("button argument is null");

    button->label = NULL;

    vao_destroy(&button->vao);
    vbo_destroy(&button->vbo);
    ebo_destroy(&button->ebo);

}


bool button_is_pressed(const crapgui_t *gui, button_t *button)
{
    if (button == NULL) eprint("button argument is null");
    if (gui == NULL) eprint("gui argument is null");

    f32 mouse_x = gui->sub_window->mouse_handler.position.cmp[X];
    f32 mouse_y = gui->sub_window->mouse_handler.position.cmp[Y];

    bool is_mouse_in_region = (
            button->vertices[TOP_LEFT].cmp[X]       < mouse_x && 
            button->vertices[TOP_RIGHT].cmp[X]      > mouse_x &&
            button->vertices[TOP_LEFT].cmp[Y]       > mouse_y &&
            button->vertices[BOTTOM_LEFT].cmp[Y]    < mouse_y
    );

    if (gui->sub_window->mouse_handler.is_active == true && gui->sub_window->mouse_handler.is_dragged == false && is_mouse_in_region) button->is_active = true;
    else button->is_active = false;

    return button->is_active;
}

bool button_is_mouseover(const crapgui_t *gui, button_t *button)
{
    if (button == NULL) eprint("button argument is null");
    if (gui == NULL) eprint("gui argument is null");


    f32 button_x = gui->sub_window->mouse_handler.position.cmp[X];
    f32 button_y = gui->sub_window->mouse_handler.position.cmp[Y];

    bool is_mouse_in_region = (
            button->vertices[TOP_LEFT].cmp[X]       < button_x && 
            button->vertices[TOP_RIGHT].cmp[X]      > button_x &&
            button->vertices[TOP_LEFT].cmp[Y]       > button_y &&
            button->vertices[BOTTOM_LEFT].cmp[Y]    < button_y
    );

    if (is_mouse_in_region)
        button->is_hot = true;
    else 
        button->is_hot = false;

    return button->is_hot;
}

void button_set_color(const crapgui_t *gui, button_t *button, vec4f_t color) 
{
    if (button == NULL) eprint("button argument is null");

    memcpy(&button->color, &color, sizeof(f32) * 4);
    gl_shader_push_uniform(&button->shader, "u_color", &button->color, sizeof(button->color), UT_VEC4F);
    __button_draw(gui, button);
}
void button_set_position(const crapgui_t *gui, button_t *button, vec2f_t pos)  
{
    if (button == NULL) eprint("button argument is null");

    button_t new_button = __button_init(
            gui,
            button->label, 
            button->color,
            pos,
            button->width,
            button->height
    );

    __button_destroy(button);

    *button = new_button;

    __button_draw(gui, button);
}

bool button_is_released(button_t *button)
{
    if (button == NULL) eprint("button argument is null");

    return button->is_active == false ? true : false;
}

bool button_is_dragged(const crapgui_t *gui, button_t *button) 
{
    if (button == NULL) eprint("button argument is null");
    if (gui == NULL) eprint("gui argument is null");

    if (button_is_pressed(gui, button))
        if (gui->sub_window->mouse_handler.is_dragged)
            return true;
    return false;
}

/*============================================================================
 //                 Sliders
============================================================================*/

typedef struct {

    const char  *label;
    f32         value;
    struct { 
        f32 min; 
        f32 max;
    } range;
    vec2f_t     position; 
    f32         width; 
    f32         height;

    // Refactor vao, vbo and ebo into a renderer struct

} slider_t;

/*-------------------------------------------------------------------------------
 // Declarations
-------------------------------------------------------------------------------*/

//slider_t slider_begin(const crapgui_t *gui, const char *label);


/*-------------------------------------------------------------------------------
 // Implementations
-------------------------------------------------------------------------------*/

slider_t slider_begin(const crapgui_t *gui)
{
    return (slider_t ) {
    };

}

/*============================================================================
 //                 
============================================================================*/

#endif //__MY__CRAP__UI__H__
