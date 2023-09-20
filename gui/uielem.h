#pragma once
#include <poglib/basic.h>
#include <poglib/math.h>

typedef struct {

    struct {
        struct {
            vec2f_t     position;
            f32         height, width;
            vec4f_t     color;
            f32         padding, margin;
        } button;
        struct {
            vec2f_t     position;
            f32         height, width;
            vec4f_t     color;
            char        value[32];
        } text;
    } style;

    struct {
        bool is_hover;
        bool is_clicked;
    } status;

} uibutton_t ;

typedef struct {

    struct {
        struct {
            vec2f_t     position;
            f32         height, width;
            vec4f_t     color;
            f32         padding, margin;
        } label;
        struct {
            vec2f_t     position;
            f32         height, width;
            vec4f_t     color;
            char        value[32];
        } text;
    } style;

} uilabel_t;
