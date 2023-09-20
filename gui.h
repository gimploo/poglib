#pragma once
#include <poglib/basic.h>
#include <poglib/gfx/glrenderer2d.h>
#include <poglib/font/glfreetypefont.h>

#include "gui/uielem.h"

typedef struct {
    vec2f_t         pos, text_pos;
    f32             width, height, text_width, text_height;
    f32             padding, margin;
    vec4f_t         color, text_color;
    const char *    text_value;
} uistyle_t ;

typedef struct gui_t {

    list_t ui_elements;

    struct {
        glfreetypefont_t font;
        glshader_t  shader;
        glbatch_t   texts;
        glbatch_t   quads;
    } gfx;

} gui_t;


gui_t           gui_init(void);
#define         gui_button(PGUI, NAME, STYLE)   __ui_elem_init(PGUI, NAME, UI_BUTTON, (STYLE))
#define         gui_label(PGUI, NAME, STYLE)    __ui_elem_init(PGUI, NAME, UI_LABEL, (STYLE))

void            gui_update(gui_t *);
uibutton_t  *   gui_get_button(const gui_t *gui, const char *name);

void            gui_render(gui_t *);

void            gui_destroy(gui_t *);




/*========================================================================================
                            ---- IMPLEMENTATION ----
========================================================================================*/

#ifndef IGNORE_POGLIB_GUI_IMPLEMENTATION

#define DEFAULT_FRAME_FONT_PATH             "lib/poglib/res/ttf_fonts/Roboto-Medium.ttf"
#define DEFAULT_FRAME_FONT_SIZE             30 
#define DEFAULT_BUTTON_COLOR            COLOR_BLUE
#define DEFAULT_BUTTON_WIDTH            0.4f
#define DEFAULT_BUTTON_HEIGHT           0.2f
#define DEFAULT_BUTTON_TEXT_HEIGHT 0.2f
#define DEFAULT_BUTTON_TEXT_WIDTH  0.2f
#define DEFAULT_BUTTON_TEXT_COLOR  COLOR_RED

#define DEFAULT_LABEL_WIDTH 0.2f
#define DEFAULT_LABEL_HEIGHT 0.2f
#define DEFAULT_LABEL_TEXT_HEIGHT 0.2f
#define DEFAULT_LABEL_TEXT_WIDTH 0.2f
#define DEFAULT_LABEL_TEXT_COLOR COLOR_GRAY

typedef enum ui_elem_type_t {
    UI_BUTTON,
    UI_LABEL,
    // ...
    UI_COUNT
} ui_elem_type_t;

typedef struct ui_elem_t {
    
    ui_elem_type_t type;
    void *data;

} ui_elem_t;

uibutton_t * gui_get_button(const gui_t *gui, const char *name)
{
    list_iterator(&gui->ui_elements, iter)
    {
        const ui_elem_t *ui = iter;
        if (ui->type != UI_BUTTON) continue;
        
        uibutton_t *button = ui->data;
        if (strcmp(button->style.text.value, name) == 0) 
            return button;
    }
    return NULL;
}

const char * const GUI_VSHADER = 
    "#version 330 core\n"
    "layout (location = 0) in vec3 v_pos;\n"
    "layout (location = 1) in vec4 v_color;\n"
    "layout (location = 2) in vec2 v_tex_coord;\n"
    "\n"
    "out vec4 color;\n"
    "\n"
    "void main()\n"
    "{\n"
        "gl_Position = vec4(v_pos, 1.0f);\n"
        "color = v_color;\n"
    "}";

const char * const GUI_FSHADER = 
    "#version 330 core\n"
    "in vec4 color;\n"
    "\n"
    "uniform sampler2D u_texture01;\n"
    "\n"
    "out vec4 FragColor;\n"
    "\n"
    "void main()\n"
    "{\n"
        "FragColor =  color;\n"
        "\n"
    "}";

gui_t gui_init(void)
{
    return (gui_t ) {
        .ui_elements = list_init(ui_elem_t),
        .gfx = {
            .shader = glshader_from_cstr_init(GUI_VSHADER, GUI_FSHADER),
            .font = glfreetypefont_init(DEFAULT_FRAME_FONT_PATH, DEFAULT_FRAME_FONT_SIZE),
            .texts = glbatch_init(1024, glquad_t),
            .quads = glbatch_init(10, glquad_t)
        },
    };
}



vec2f_t __ui_get_default_position(const gui_t *gui)
{
    if (gui->ui_elements.len == 0) return (vec2f_t ){-0.9f, 0.9f};

    const ui_elem_t *lastelem = list_get_value(&gui->ui_elements, gui->ui_elements.len - 1);

    vec2f_t lastpos = {0};
    f32 margin = 0;
    f32 height = 0;
    f32 width = 0;

    switch(lastelem->type)
    {
        case UI_BUTTON: {
            const uibutton_t *button = lastelem->data;
            lastpos = button->style.button.position;            
            margin = button->style.button.margin;
            height = button->style.button.height;
            width = button->style.button.width;
        } break;

        case UI_LABEL: {
            const uilabel_t *label = lastelem->data;
            lastpos = label->style.label.position;            
            margin = label->style.label.margin;
            height = label->style.label.height;
            width = label->style.label.width;
        } break;

        default: eprint("unknown elem type");
    }

    vec2f_t newpos = (vec2f_t ) {
        .x = lastpos.x + width,
        .y = lastpos.y 
    };

    if (newpos.x >= 1.0f) {
        newpos.x = 0.0f;
        newpos.y = lastpos.y + height;
    }

    return (vec2f_t ) {
        .x = newpos.x + margin,
        .y = newpos.y - margin
    };
}


uilabel_t __uilabel_init(const gui_t *gui, const char *label, const uistyle_t *style)
{
    const vec2f_t pos = __ui_get_default_position(gui);

    // freetype has its position at the bottom left
    const vec2f_t textpos = (vec2f_t ) {
        .x = pos.x + (style ? style->padding : 0.0f) ,
        .y = pos.y - (style ? style->padding + style->height : 0.0f + DEFAULT_LABEL_HEIGHT),
    };

    uilabel_t output = (uilabel_t ) {
        .style = {
            .label = {
                .position = style ? style->pos : pos,
                .color = style ? style->color : COLOR_BLACK,
                .width = style ? style->width : DEFAULT_LABEL_WIDTH,
                .height = style ? style->height : DEFAULT_LABEL_HEIGHT,
                .padding = style ? style->padding : 0.0f,
                .margin = style ? style->margin : 0.0f,
            },
            .text = {
                .position = style ? style->text_pos : textpos,
                .height = style ? style->text_height : DEFAULT_BUTTON_TEXT_HEIGHT,
                .width = style ? style->text_width : DEFAULT_BUTTON_TEXT_WIDTH,
                .color = style ? style->text_color : DEFAULT_BUTTON_TEXT_COLOR,
                .value = "LABEL"
            }
        },
    };
    if (style)
        memcpy(
            output.style.text.value, 
            style->text_value ? style->text_value : "NULL", 
            sizeof(output.style.text.value));

    return output;
}

uibutton_t __uibutton_init(const gui_t *gui, const char *label, const uistyle_t * style)
{
    const vec2f_t pos = __ui_get_default_position(gui);

    // freetype font has the text position start from the bottom left
    const vec2f_t textpos = (vec2f_t ) {
        .x = pos.x + (style ? style->padding : 0.0f),
        .y = pos.y - (style ? style->padding + style->height : 0.0f + DEFAULT_BUTTON_HEIGHT),
    };

    uibutton_t output = (uibutton_t ) {
        .style = {
            .button = {
                .position = style ? style->pos : pos,
                .color = style ? style->color : COLOR_BLUE,
                .width = style ? style->width : DEFAULT_BUTTON_WIDTH,
                .height = style ? style->height : DEFAULT_BUTTON_HEIGHT,
                .padding = style ? style->padding : 0.0f,
                .margin = style ? style->margin : 0.0f,
            },
            .text = {
                .position = style ? style->text_pos : textpos,
                .height = style ? style->text_height : DEFAULT_BUTTON_TEXT_HEIGHT,
                .width = style ? style->text_width : DEFAULT_BUTTON_TEXT_WIDTH,
                .color = style ? style->text_color : DEFAULT_BUTTON_TEXT_COLOR,
                .value = "BUTTON"
            }
        },
        .status = { false, false },
    };

    if (style)
        memcpy(
            output.style.text.value, 
            style->text_value ? style->text_value : "NULL", 
            sizeof(output.style.text.value));

    return output;
}


void __ui_elem_init(gui_t *gui, const char *name, ui_elem_type_t type, const uistyle_t *styles)
{
    ui_elem_t newelem = {
        .type = type
    };

    switch(type)
    {
        case UI_BUTTON: {
            newelem.data = calloc(1, sizeof(uibutton_t ));
            assert(newelem.data);
            uibutton_t button = __uibutton_init(gui, name, styles);
            memcpy(newelem.data, &button, sizeof(uibutton_t ));
        } break;
        case UI_LABEL: {
            newelem.data = calloc(1, sizeof(uilabel_t ));
            assert(newelem.data);
            uilabel_t label = __uilabel_init(gui, name, styles);
            memcpy(newelem.data, &label, sizeof(uilabel_t ));
        } break;
        default: eprint("unknown ui element type\n");
    }
    
    list_append(&gui->ui_elements, newelem);
}

void gui_render(gui_t *self)
{
    // Both batch queues are fixed to size 10 for now,
    // TODO: find a better way to do this
    assert(self->ui_elements.len < 10);

    glrenderer2d_draw_from_batch(
            &(glrenderer2d_t ) {
                .shader = &self->gfx.shader,
                .texture = NULL
            }, 
            &self->gfx.quads);
    glfreetypefont_draw(&self->gfx.font, &self->gfx.texts);

    // For now we clear before every render 
    // we need to design so as to only render when any changes happen to it
    glbatch_clear(&self->gfx.quads);
    glbatch_clear(&self->gfx.texts);
}

void __ui_elem_button_update(gui_t *gui, uibutton_t *button)
{
    const quadf_t buttonquad = quadf(
            (vec3f_t ) { 
                button->style.button.position.x + button->style.button.margin, 
                button->style.button.position.y - button->style.button.margin, 
                0.0f 
            }, 
            button->style.button.width, button->style.button.height);

    // functionality
    if (quadf_is_point_in_quad(
            buttonquad, window_mouse_get_norm_position(global_window))) {
        button->status.is_hover = true;
        button->style.button.color.a = 0.7f;
        button->style.text.color.a = 0.7f;
    } else {
        button->status.is_hover = false;
        button->style.button.color.a = 1.0f;
        button->style.text.color.a = 1.0f;
    }

    if (button->status.is_hover) {
        if (window_mouse_button_is_pressed(global_window, SDL_MOUSEBUTTON_LEFT))
            button->status.is_clicked = true;
        else
            button->status.is_clicked = false;
    }

    //quad
    const glquad_t glbt = glquad(buttonquad, button->style.button.color, QUAD_UV_DEFAULT);
    glbatch_put(&gui->gfx.quads, glbt);

    //text
    glfreetypefont_add_text_to_batch(
            &gui->gfx.font, 
            &gui->gfx.texts, 
            button->style.text.value, 
            button->style.text.position, 
            button->style.text.color);

}

void __ui_elem_label_update(gui_t *gui, uilabel_t *label)
{
    //quad
    const quadf_t labelquad = quadf(
            (vec3f_t ) { 
                label->style.label.position.x + label->style.label.margin, 
                label->style.label.position.y - label->style.label.margin, 
                0.0f 
            }, 
            label->style.label.width, label->style.label.height);

    const glquad_t gllb = glquad(labelquad, label->style.label.color, QUAD_UV_DEFAULT);
    glbatch_put(&gui->gfx.quads, gllb);

    //text
    glfreetypefont_add_text_to_batch(
            &gui->gfx.font, 
            &gui->gfx.texts, 
            label->style.text.value, 
            label->style.text.position, 
            label->style.text.color);
}

void gui_update(gui_t *self)
{
    list_iterator(&self->ui_elements, iter)
    {
        ui_elem_t *elem = (ui_elem_t *)iter;
        switch(elem->type)
        {
            case UI_BUTTON: {
                __ui_elem_button_update(self, (uibutton_t *)elem->data);
            } break;
            case UI_LABEL: {
                __ui_elem_label_update(self, (uilabel_t *)elem->data);
            } break;
            default: eprint("unkown ui elem type");
        }
    }
}

void gui_destroy(gui_t *self)
{
    glshader_destroy(&self->gfx.shader);
    glfreetypefont_destroy(&self->gfx.font);
    glbatch_destroy(&self->gfx.quads);
    glbatch_destroy(&self->gfx.texts);

    list_iterator(&self->ui_elements, iter)
        free(((ui_elem_t *)iter)->data);
    list_destroy(&self->ui_elements);
}

#endif //IGNORE_POGLIB_GUI_IMPLEMENTATION
