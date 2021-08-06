#ifndef __MY__CRAP__UI__H__
#define __MY__CRAP__UI__H__

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "../basic.h"
#include "../ds/stack.h"

#include "../simple/gl_renderer.h"
#include "../simple/window.h"

#include "button.h"

#define MAX_COMPONENT_CAPACITY 10

Shader cmp_shader;

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

    Window      window;
    vao_t       vao;

    stack_t     components;
    bool        is_close;
};

static crapgui_t   gui_init(Window *window);
static void        gui_draw(crapgui_t *gui);
static bool        gui_destroy(crapgui_t *gui);


static crapgui_t gui_init(Window *window)
{
    if (window == NULL) eprint("window argument is null");

    return (crapgui_t) {
        .window = window,
        .vao = vao_init(MAX_COMPONENT_CAPACITY),
        .is_close = false
    };
}

bool gui_destroy(crapgui_t *gui) {}

#endif //__MY__CRAP__UI__H__
