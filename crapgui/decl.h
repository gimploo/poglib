#pragma once
#include <poglib/application.h>
#include <poglib/ds.h>

/*=============================================================================
// CRAPGUI ( IMMEDIATE UI INSPIRED GUI LIB )
=============================================================================*/

typedef struct crapgui_t    crapgui_t ;
typedef struct frame_t      frame_t ;
typedef struct ui_t         ui_t ;

typedef enum uitype {

    UI_BUTTON,
    UI_LABEL,
    //
    //..
    //
    UITYPE_COUNT

} uitype;

typedef struct uistyle_t {

    f32         width;
    f32         height;
    vec2f_t     padding;
    vec2f_t     margin;
    vec4f_t     color;
    vec4f_t     hovercolor;
    vec4f_t     textcolor;

} uistyle_t ;

typedef struct ui_t {

    const char          *title;
    uitype              type;
    uistyle_t           styles;
    vec2f_t             pos;
    bool                is_hot;
    bool                is_active;
    struct {
        bool            cache_again;
        quadf_t         quad;
        glquad_t        glquad;
        gltext_t        texts;
    } __cache;

} ui_t ;

#define __crapgui_recache_only_frame(PFRAME) do {\
    (PFRAME)->__cache.self.cache_again = true;\
} while (0)

#define __crapgui_recache_both(PFRAME) do {\
    (PFRAME)->__cache.uis.cache_again = true;\
    (PFRAME)->__cache.self.cache_again = true;\
} while(0)

#define __crapgui_recache_only_uis(PFRAME) do {\
    (PFRAME)->__cache.uis.cache_again = true;\
} while (0)

#define __crapgui_recache_ui(PUI) do {\
    (PUI)->__cache.cache_again = true;\
} while(0)

typedef struct crapgui_t {

    window_t                *win;
    map_t                   frames;
    struct {
        glfreetypefont_t    font;
        glshader_t          shader;
    } frame_assets ;
    struct {
        glfreetypefont_t    fonts[UITYPE_COUNT];
        glshader_t          shaders[UITYPE_COUNT];
    } ui_assets;
    glshader_t              __common_shader;
    struct {
        bool                is_on;

        struct {
            bool            frame;
            bool            ui;
        } focused;

        // Clicking on the middle mouse allows to the user to select from 
        // different options to do edit the gui
        struct {
            vec2f_t         pos;
            bool            is_active;
        } optionwheel;

    } editmode;
    struct {
        frame_t             *frame;
        ui_t                *ui;
    } currently_active;

    void (*update)(struct crapgui_t *);
    void (*render)(struct crapgui_t *);

} crapgui_t ;

crapgui_t __crapgui_init(void);

#define COMMON_VS   "lib/poglib/crapgui/res/common.vs"
#define COMMON_FS   "lib/poglib/crapgui/res/common.fs"
#define FRAME_FS    "lib/poglib/crapgui/res/frame.fs"
#define BUTTON_FS   "lib/poglib/crapgui/res/button.fs"
#define LABEL_FS    "lib/poglib/crapgui/res/label.fs"
#define DEFAULT_CRAPGUI_CONFIG_PATH "bin/crapgui.config"

/*=============================================================================
 // FRAME 
=============================================================================*/

#define MAX_FRAMES_ALLOWED                  10 

//NOTE: write now 10 here for the batch stuff, better to give the 10 a name
#define MAX_UI_CAPACITY_PER_FRAME           10 * UITYPE_COUNT

#define DEFAULT_UI_TEXT_COLOR               COLOR_WHITE
#define DEFAULT_UI_MARGIN                   (vec2f_t ){0.08f, 0.08f}

#define FRAME_HEADER_HEIGHT                 0.2f
#define DEFAULT_FRAME_FONT_PATH             "lib/poglib/res/ttf_fonts/Roboto-Medium.ttf"
#define DEFAULT_FRAME_FONT_SIZE             20 
#define DEFAULT_FRAME_WIDTH                 0.8f
#define DEFAULT_FRAME_HEIGHT                0.8f
#define DEFAULT_FRAME_COLOR                 COLOR_RED
#define DEFAULT_FRAME_HOVER_COLOR           COLOR_BLUE
#define DEFAULT_FRAME_MARGIN                (vec2f_t ){0.04f, 0.04f}

#define DEFAULT_FRAME_STYLE\
    (uistyle_t ) {          \
        .width          = DEFAULT_FRAME_WIDTH,\
        .height         = DEFAULT_FRAME_HEIGHT,\
        .padding        = 0.0f,\
        .margin         = DEFAULT_FRAME_MARGIN,\
        .color          = DEFAULT_FRAME_COLOR,\
        .hovercolor     = DEFAULT_FRAME_HOVER_COLOR,\
        .textcolor      = DEFAULT_UI_TEXT_COLOR,\
    }

typedef struct frame_t {

    const char              *label;
    vec2f_t                 pos;
    uistyle_t               styles;
    slot_t                  uis;
    struct {
        struct {
            bool            cache_again;
            quadf_t         quad;
            gltext_t        texts;
            glbatch_t       glquads;
        } self;
        struct {
            bool            cache_again;
            glframebuffer_t texture;
            glbatch_t       uibatch[UITYPE_COUNT];
            gltext_t        txtbatch[UITYPE_COUNT];
        } uis;
    } __cache;

    void (*update)(struct frame_t * self, crapgui_t *gui);
    void (*render)(const struct frame_t * self, const crapgui_t *gui);

} frame_t ;

frame_t     frame_init(const char *label, vec2f_t pos, uistyle_t styles);
vec2f_t     frame_get_mouse_position(const frame_t *frame);
void        frame_destroy(frame_t *self);


/*=============================================================================
 // BUTTON 
=============================================================================*/

#define DEFAULT_BUTTON_FONT_PATH        "lib/poglib/res/ttf_fonts/Roboto-Medium.ttf"
#define DEFAULT_BUTTON_FONT_SIZE        28 
#define DEFAULT_BUTTON_COLOR            COLOR_BLUE
#define DEFAULT_BUTTON_HOVER_COLOR      (vec4f_t ){0.0f, 0.0f, 1.0f, 0.7f}
#define DEFAULT_BUTTON_WIDTH            0.4f
#define DEFAULT_BUTTON_HEIGHT           0.2f

#define DEFAULT_BUTTON_STYLE\
    (uistyle_t ) {          \
        .width          = DEFAULT_BUTTON_WIDTH,\
        .height         = DEFAULT_BUTTON_HEIGHT,\
        .padding        = 0.0f,\
        .margin         = DEFAULT_UI_MARGIN,\
        .color          = DEFAULT_BUTTON_COLOR,\
        .hovercolor     = DEFAULT_BUTTON_HOVER_COLOR,\
        .textcolor      = DEFAULT_UI_TEXT_COLOR,\
    }


/*=============================================================================
 // LABEL 
=============================================================================*/

#define DEFAULT_LABEL_FONT_PATH        "lib/poglib/res/ttf_fonts/Roboto-Medium.ttf"
#define DEFAULT_LABEL_FONT_SIZE        38
#define DEFAULT_LABEL_WIDTH            0.9f
#define DEFAULT_LABEL_HEIGHT           0.2f
#define DEFAULT_LABEL_COLOR            COLOR_BLACK

#define DEFAULT_LABEL_STYLE\
    (uistyle_t ) {          \
        .width          = DEFAULT_LABEL_WIDTH,\
        .height         = DEFAULT_LABEL_HEIGHT,\
        .padding        = 0.0f,\
        .margin         = DEFAULT_UI_MARGIN,\
        .color          = DEFAULT_LABEL_COLOR,\
        .hovercolor     = vec4f(0.0f),\
        .textcolor      = DEFAULT_UI_TEXT_COLOR,\
    }

