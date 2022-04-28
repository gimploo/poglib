#pragma once
#include <poglib/application.h>


typedef enum uitype {

    UI_BUTTON,
    UI_LABEL,
    //
    //..
    //

    // dont move frame, have it stay here
    UI_FRAME,
    UITYPE_COUNT

} uitype;

typedef struct uielem_t {

    uitype              type;
    void                *value;

    vec2f_t             pos;
    vec2f_t             dim;
    vec4f_t             color;

    glfreetypefont_t    *__font;
    bool                __is_changed;

    glquad_t            __uivertices;
    glbatch_t           __textbatch;

    void (*update)(struct uielem_t *);

} uielem_t ;



/*=============================================================================
// CRAPGUI ( IMMEDIATE UI INSPIRED GUI LIB )
=============================================================================*/

typedef struct crapgui_t {

    window_t            *win;
    glfreetypefont_t    fonts[UITYPE_COUNT];

    glshader_t          shaders[UITYPE_COUNT];
    map_t               frames;

    void (*update)(struct crapgui_t *);
    void (*render)(struct crapgui_t *);

} crapgui_t ;


/*=============================================================================
 // FRAME 
=============================================================================*/

#define MAX_FRAMES_ALLOWED                  10 
#define MAX_UI_TYPE_ALLOWED_IN_FRAME        UITYPE_COUNT - 1

//NOTE: write now 10 here for the batch stuff, better to give the 10 a name
#define MAX_UI_CAPACITY_PER_FRAME           10 * MAX_UI_TYPE_ALLOWED_IN_FRAME

#define DEFAULT_UI_MARGIN                   (vec2f_t ){0.08f, 0.08f}

#define DEFAULT_FRAME_FONT_PATH             "lib/poglib/res/ttf_fonts/Roboto-Medium.ttf"
#define DEFAULT_FRAME_FONT_SIZE             20 
#define DEFAULT_FRAME_DIMENSIONS            (vec2f_t ){ 0.8f, 0.8f }
#define DEFAULT_FRAME_BACKGROUNDCOLOR       COLOR_RED
#define DEFAULT_FRAME_MARGIN                (vec2f_t ){0.04f, 0.04f}

typedef struct frame_t {

    const char          *label;
    vec2f_t             pos;
    vec2f_t             dim;
    vec4f_t             color;
    list_t              uielems;

    crapgui_t           *__gui;
    bool                __is_changed;       // flag to check whether to rebatch the batch
    glframebuffer_t     __texture;
    glquad_t            __vertices;
    glbatch_t           __uibatch[MAX_UI_TYPE_ALLOWED_IN_FRAME];
    glbatch_t           __txtbatch[MAX_UI_TYPE_ALLOWED_IN_FRAME];

    void (*update)(struct frame_t * self);
    void (*render)(struct frame_t * self);

} frame_t ;


/*=============================================================================
 // BUTTON 
=============================================================================*/

#define DEFAULT_BUTTON_FONT_PATH        "lib/poglib/res/ttf_fonts/Roboto-Medium.ttf"
#define DEFAULT_BUTTON_FONT_SIZE        28 
#define DEFAULT_BUTTON_COLOR            COLOR_BLUE
#define DEFAULT_BUTTON_HOVER_COLOR      COLOR_GRAY
#define DEFAULT_BUTTON_DIMENSIONS       (vec2f_t ){0.4f, 0.2f}


typedef struct button_t {

    const char  *label;
    bool        is_active;
    bool        is_hot;
    vec4f_t     hover_color;

} button_t ;

/*=============================================================================
 // LABEL 
=============================================================================*/

#define DEFAULT_LABEL_FONT_PATH        "lib/poglib/res/ttf_fonts/Roboto-Medium.ttf"
#define DEFAULT_LABEL_FONT_SIZE        38
#define DEFAULT_LABEL_DIMENSIONS       (vec2f_t ){0.9f, 0.2f}
#define DEFAULT_LABEL_COLOR            COLOR_BLACK

typedef struct label_t {

    const char *label;

} label_t ;

