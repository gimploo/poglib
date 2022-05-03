#pragma once
#include <poglib/application.h>
#include <poglib/ds.h>

/*=============================================================================
// CRAPGUI ( IMMEDIATE UI INSPIRED GUI LIB )
=============================================================================*/

typedef struct crapgui_t crapgui_t ;
typedef struct frame_t  frame_t ;
typedef struct ui_t ui_t ;

typedef enum uitype {

    UI_BUTTON,
    UI_LABEL,
    //
    //..
    //
    
    // NOTE: DONT MOVE FRAME, have it stay here 
    // (it needs to be one step above UITYPE_COUNT always!)
    UI_FRAME,                       
    UITYPE_COUNT

} uitype;

typedef struct ui_t {

    //name
    const char          *title;

    // ui type (like button ,label, ...)
    uitype              type;

    //positoning
    vec2f_t             pos;
    vec2f_t             dim;
    vec2f_t             margin;

    // color
    vec4f_t             textcolor;
    vec4f_t             basecolor;
    vec4f_t             hovercolor;

    // mouse related things ..
    bool                is_hot;
    bool                is_active;

    // opengl specifics
    bool                __update_cache_now;
    quadf_t             __vertices;
    glquad_t            __glvertices;
    glbatch_t           __textbatch;

} ui_t ;


typedef struct crapgui_t {

    window_t            *win;
    glfreetypefont_t    fonts[UITYPE_COUNT];

    glshader_t          shaders[UITYPE_COUNT];
    map_t               frames;

    void (*update)(struct crapgui_t *);
    void (*render)(struct crapgui_t *);

} crapgui_t ;

#define         crapgui_get_font(PGUI, UITYPE)      (glfreetypefont_t *)&(PGUI)->fonts[UITYPE]

/*=============================================================================
 // FRAME 
=============================================================================*/

#define MAX_FRAMES_ALLOWED                  10 
#define MAX_UI_TYPE_ALLOWED_IN_FRAME        UITYPE_COUNT - 1

//NOTE: write now 10 here for the batch stuff, better to give the 10 a name
#define MAX_UI_CAPACITY_PER_FRAME           10 * MAX_UI_TYPE_ALLOWED_IN_FRAME

#define DEFAULT_UI_TEXT_COLOR               COLOR_WHITE
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
    vec2f_t             margin;
    slot_t              uis;

    bool                __update_cache_now;
    glframebuffer_t     __texture;
    quadf_t             __vertices;
    glbatch_t           __uibatch[MAX_UI_TYPE_ALLOWED_IN_FRAME];
    glbatch_t           __txtbatch[MAX_UI_TYPE_ALLOWED_IN_FRAME];

    void (*update)(struct frame_t * self, const crapgui_t *gui);
    void (*render)(const struct frame_t * self, const crapgui_t *gui);

} frame_t ;

frame_t     frame_init(const char *label, vec2f_t pos, vec4f_t color, vec2f_t dim);
vec2f_t     frame_get_mouse_position(const frame_t *frame);
void        frame_destroy(frame_t *self);


/*=============================================================================
 // BUTTON 
=============================================================================*/

#define DEFAULT_BUTTON_FONT_PATH        "lib/poglib/res/ttf_fonts/Roboto-Medium.ttf"
#define DEFAULT_BUTTON_FONT_SIZE        28 
#define DEFAULT_BUTTON_COLOR            COLOR_BLUE
#define DEFAULT_BUTTON_HOVER_COLOR      COLOR_GRAY
#define DEFAULT_BUTTON_DIMENSIONS       (vec2f_t ){0.4f, 0.2f}


/*=============================================================================
 // LABEL 
=============================================================================*/

#define DEFAULT_LABEL_FONT_PATH        "lib/poglib/res/ttf_fonts/Roboto-Medium.ttf"
#define DEFAULT_LABEL_FONT_SIZE        38
#define DEFAULT_LABEL_DIMENSIONS       (vec2f_t ){0.9f, 0.2f}
#define DEFAULT_LABEL_COLOR            COLOR_BLACK


