#include "./common.h"

#define label_update_value(plabel, text) do {\
    (plabel)->string = text;\
    (plabel)->string_len = strlen(text);\
} while(0)


label_t label_init(const char *value, vec3f_t norm_position, f32 norm_font_size)
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
    glfreetypefont_set_text(gui->font_handler, label->string, vec2f(label->norm_position), COLOR_WHITE);
}
