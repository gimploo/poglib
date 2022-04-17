#pragma once
#include "../simple/glrenderer2d.h"
#include "../font/glfreetypefont.h"

/*=============================================================================
                    - Immedaite Mode inspired gui -
=============================================================================*/


typedef struct crapgui_t {

    window_t            *__window;
    glrenderer2d_t      rend2d;
    glfreetypefont_t    font;

} crapgui_t ;


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

crapgui_t crapgui_init(void)
{
    if (global_window) eprint("window not found\n");

    glshader_t *gui_shader = (glshader_t *)malloc(sizeof(glshader_t));
    if (gui_shader == NULL) eprint("malloc returned null");

    *gui_shader = glshader_from_cstr_init(GUI_VS_SHADER_CODE, GUI_FS_SHADER_CODE);

    return (crapgui_t) {
        .__window            = window,
        .font_handler        = font,
        .renderer_handler    = glrenderer2d_init(gui_shader, NULL),
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
    glshader_destroy(gui->renderer_handler.__shader);
    free(gui->renderer_handler.__shader);

    // renderer
    glrenderer2d_destroy(&gui->renderer_handler);
}
