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


/*===============================================================
 //                     CrapGui
=============================================================== */

#define MAX_GLOBAL_UI_COMPONENTS_CAPCITY 10
global void * GLOBAL_UI_COMPONENTS_ARRAY[MAX_GLOBAL_UI_COMPONENTS_CAPCITY];

typedef enum component_type {
    CT_BUTTON = 0,
    CT_COUNT
} component_type;

typedef struct ui_component_t ui_component_t;
struct ui_component_t {

    void            *component; // holds the address to a component
    component_type  type;
};

typedef struct crapgui_t crapgui_t;
struct crapgui_t {

    Window      *window;
    stack_t     ui_components;
    u64         ui_component_count;
    bool        is_close;
};

/*--------------------------------------------
 // Declarations
--------------------------------------------*/

crapgui_t   gui_init(Window *window);
void        gui_begin(crapgui_t *gui);
void        gui_end(crapgui_t *gui);


/*--------------------------------------------
 // Implementations
--------------------------------------------*/


crapgui_t gui_init(Window *window)
{
    if (window == NULL) eprint("window argument is null");

    return (crapgui_t) {
        .window = window,
        /*.ui_components = stack_init(GLOBAL_UI_COMPONENTS_ARRAY, MAX_GLOBAL_UI_COMPONENTS_CAPCITY),*/
        .ui_component_count = 0,
        .is_close = false
    };
}

void gui_begin(crapgui_t *gui)
{
    if (gui == NULL) eprint("gui argument is null");
}
void gui_end(crapgui_t *gui)
{
    if (gui == NULL) eprint("gui argument is null");
}





/*=========================================================
 //                 Button
=========================================================*/

// OpenGL vertices index position in buffer
#define TOP_LEFT        0
#define TOP_RIGHT       1
#define BOTTOM_RIGHT    2
#define BOTTOM_LEFT     3

#define DEFAULT_COLOR   (vec4f) {0.4f, 0.3f, 0.8f, 1.0f}
#define DEFAULT_POS     (vec2f) {-0.8f, +0.8f}
#define DEFAULT_WIDTH   0.3f
#define DEFAULT_HEIGHT  0.2f


typedef struct button_t button_t;
struct button_t {

    const crapgui_t * gui_handler;

    const char  *label;
    bool        is_hot;
    bool        is_active;

    vec2f       position; 
    f32         width, height;
    vec4f       color;

    vao_t       vao;
    vbo_t       vbo;
    ebo_t       ebo;
    vec3f       vertices[4];
    gl_shader_t shader;
};

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

button_t    button_begin(crapgui_t *gui, const char *label);

#define     button_set_width(pbutton, width)        ((pbutton)->width = width)
#define     button_set_height(pbutton,height)       ((pbutton)->height = height)
void        button_set_position(button_t *button, vec2f position);
void        button_set_color(button_t *button, vec4f color);

bool        button_is_pressed(button_t *button);         
bool        button_is_dragged(button_t *button);
bool        button_is_mouseover(button_t *button);

#define     button_end(pbutton) (__button_destroy(pbutton))



/*---------------------------
// Implementations
---------------------------*/

button_t __button_init(const crapgui_t *gui, const char *label, vec4f color, vec2f position, f32 width, f32 height) 
{
    button_t button = {
        .gui_handler = gui,
        .label = label,
        .is_hot = false, 
        .is_active = false,
        .position = position,
        .width = width,
        .height = height,
        .color = color,
        .vao    = vao_init(1),
        .shader = shader_load_code(vertex_source, fragment_source),
    };

    vec3f vertices[4]; 

    // top left
    memcpy(&vertices[0], &button.position, sizeof(button.position)); 
    vertices[0].cmp[Z] = 0.0f; 
    
    // top right
    vertices[1] = (vec3f) {     
        button.position.cmp[X] + button.width,
        button.position.cmp[Y],
        0.0f
    }; 

    // bottom right
    vertices[2] = (vec3f) {     
        button.position.cmp[X] + button.width,
        button.position.cmp[Y] - button.height,
        0.0f
    }; 


    // bottom left
    vertices[3] = (vec3f) {     
        button.position.cmp[X],
        button.position.cmp[Y] - button.height,
        0.0f
    }; 

    memcpy(button.vertices, vertices, sizeof(vertices));

    //for (int i = 0; i < 4; i++)
    //{
        //printf("(%f, %f, %f) \n", 
                //button.vertices[i].cmp[X], 
                //button.vertices[i].cmp[Y], 
                //button.vertices[i].cmp[Z]); 
    //}
    //printf("\n");

    shader_set_farr(&button.shader, "u_color", (float *)&button.color);
    

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




void __button_draw(button_t * button)
{
    if (button == NULL) eprint("button argument is null");

    u32 indices[] = {          
        0, 1, 2, // first triangle  
        2, 3, 0  // second triangle 
    };                                  

    vao_bind(&button->vao);
    {
        shader_bind(&button->shader);
        GL_CHECK(glDrawElements(GL_TRIANGLES, button->vbo.indices_count, GL_UNSIGNED_INT, 0));
    }
    vao_unbind();
}

button_t button_begin(crapgui_t *gui, const char *label) 
{ 
    if (gui == NULL) eprint("gui argument is null");
    if (label == NULL) eprint("label argument is null");

    vec2f generate_position = vec2f_add( 
                DEFAULT_POS, 
                (vec2f) {0.0f, -(DEFAULT_WIDTH * gui->ui_component_count)} 
    );
    
    button_t button = __button_init(gui, label, DEFAULT_COLOR, generate_position, DEFAULT_WIDTH, DEFAULT_HEIGHT);

    __button_draw(&button);

    gui->ui_component_count++;

    return button;
}

static void __button_destroy(button_t *button)
{
    if (button == NULL) eprint("button argument is null");

    button->label = NULL;
    button->gui_handler = NULL;

    vao_destroy(&button->vao);
    vbo_destroy(&button->vbo);
    ebo_destroy(&button->ebo);

}


bool button_is_pressed(button_t *button)
{
    if (button == NULL) eprint("button argument is null");

    const crapgui_t *gui = button->gui_handler;
    if (gui == NULL) eprint("gui is null");

    f32 mouse_x = gui->window->mouse_handler.position.cmp[X];
    f32 mouse_y = gui->window->mouse_handler.position.cmp[Y];

    bool is_mouse_in_region = (
            button->vertices[TOP_LEFT].cmp[X]       < mouse_x && 
            button->vertices[TOP_RIGHT].cmp[X]      > mouse_x &&
            button->vertices[TOP_LEFT].cmp[Y]       > mouse_y &&
            button->vertices[BOTTOM_LEFT].cmp[Y]    < mouse_y
    );

    //printf("\n\n%f > %f\n", button->vertices[TOP_LEFT].cmp[X]  ,mouse_x ); 
    //printf("%f < %f\n", button->vertices[TOP_RIGHT].cmp[X]  ,mouse_x ); 
    //printf("%f > %f\n", button->vertices[TOP_LEFT].cmp[Y]  ,mouse_y ); 
    //printf("%f < %f\n", button->vertices[BOTTOM_LEFT].cmp[Y]  ,mouse_y ); 


    if (gui->window->mouse_handler.is_active == true && gui->window->mouse_handler.is_dragged == false && is_mouse_in_region) button->is_active = true;
    else button->is_active = false;

    return button->is_active;
}

bool button_is_mouseover(button_t *button)
{
    if (button == NULL) eprint("button argument is null");

    const crapgui_t *gui = button->gui_handler;
    ;
    if (gui == NULL) eprint("gui is null");

    f32 button_x = gui->window->mouse_handler.position.cmp[X];
    f32 button_y = gui->window->mouse_handler.position.cmp[Y];

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

void button_set_color(button_t *button, vec4f color) 
{
    if (button == NULL) eprint("button argument is null");

    memcpy(&button->color, &color, sizeof(f32) * 4);
    shader_set_farr(&button->shader, "u_color", (float *)&button->color);
    __button_draw(button);
}
void button_set_position(button_t *button, vec2f pos)  
{
    if (button == NULL) eprint("button argument is null");

    button_t new_button = __button_init(
            button->gui_handler, 
            button->label, 
            button->color,
            pos,
            button->width,
            button->height
    );

    __button_destroy(button);

    *button = new_button;

    __button_draw(button);
}

bool button_is_released(button_t *button)
{
    if (button == NULL) eprint("button argument is null");

    return button->is_active == false ? true : false;
}

bool button_is_dragged(button_t *button) 
{
    if (button == NULL) eprint("button argument is null");

    if (button_is_pressed(button))
        if (button->gui_handler->window->mouse_handler.is_dragged)
            return true;
    return false;
}

/*============================================================================
 //                 
============================================================================*/

#endif //__MY__CRAP__UI__H__
