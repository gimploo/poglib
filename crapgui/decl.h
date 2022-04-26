#pragma once
#include <poglib/application.h>


typedef enum uitype {

    UIELEM_BUTTON,
    //
    //..
    //

    // dont move frame, have it stay here
    UIELEM_FRAME,
    UIELEM_COUNT

} uitype;

typedef struct uielem_t {

    const uitype    type;
    void            *value;

    vec2f_t         pos;
    vec2f_t         dim;
    vec4f_t         color;
    bool            __is_changed;
    glquad_t        __vertices;

    void (*update)(struct uielem_t *);

} uielem_t ;



/*=============================================================================
// CRAPGUI ( IMMEDIATE UI INSPIRED GUI LIB )
=============================================================================*/

#define MAX_FRAMES_ALLOWED_IN_CRAPGUI   10 
#define DEFAULT_FONT_PATH               "lib/poglib/res/ttf_fonts/Roboto-Medium.ttf"
#define DEFAULT_FONT_SIZE               15

typedef struct crapgui_t {

    window_t            *win;
    glfreetypefont_t    font;

    glshader_t          shaders[UIELEM_COUNT];
    map_t               frames;

    void (*update)(struct crapgui_t *);
    void (*render)(struct crapgui_t *);

} crapgui_t ;


/*=============================================================================
 // FRAME 
=============================================================================*/

#define MAX_UIELEM_TYPE_ALLOWED_IN_FRAME    UIELEM_COUNT - 1
#define MAX_UIELEM_CAPACITY_PER_FRAME       10
#define DEFAULT_UIELEM_MARGIN               (vec2f_t ){0.08f, 0.08f}

#define DEFAULT_FRAME_DIMENSIONS            (vec2f_t ){ 0.8f, 0.8f }
#define DEFAULT_FRAME_BACKGROUNDCOLOR       COLOR_RED
#define DEFAULT_FRAME_MARGIN                (vec2f_t ){0.04f, 0.04f}

typedef struct frame_t {

    const char      *label;
    vec2f_t         pos;
    vec2f_t         dim;
    vec4f_t         color;
    list_t          uielems;

    bool            __is_changed;       // flag to check whether to rebatch the batch
    glframebuffer_t __texture;
    glquad_t        __vertices;
    glbatch_t       __batch[MAX_UIELEM_TYPE_ALLOWED_IN_FRAME];

    void (*update)(struct frame_t * self, crapgui_t *gui);
    void (*render)(struct frame_t * self, crapgui_t *gui);

} frame_t ;


/*=============================================================================
 // BUTTON 
=============================================================================*/

#define DEFAULT_BUTTON_COLOR        COLOR_BLUE
#define DEFAULT_BUTTON_HOVER_COLOR  COLOR_GRAY
#define DEFAULT_BUTTON_DIMENSIONS   (vec2f_t ){0.4f, 0.2f}

typedef struct button_t {

    const char  *label;
    bool        is_active;
    bool        is_hot;
    vec4f_t     hover_color;

    void (*update)(struct button_t * self, uielem_t *);

} button_t ;
