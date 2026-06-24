/* clay_renderer_poglib.h -- clay UI renderer for poglib (OpenGL 3.3+)
 *
 * Usage:
 *   1. #define CLAY_IMPLEMENTATION and #include "clay.h" in one file
 *   2. #include "clay_renderer_poglib.h"
 *   3. Create font / image arrays
 *   4. clay_poglib_renderer_t r = clay_poglib_init(screenW, screenH);
 *   5. Clay_SetMeasureTextFunction(clay_poglib_measure_text, fonts);
 *   6. Each frame: clay_poglib_render(&r, cmds, fonts, nfonts, images, nimgs);
 *   7. clay_poglib_destroy(&r);
 *
 * Coordinates: top-left origin, y-down (matches clay's convention).
 * The renderer uses an orthographic projection so positions in pixels
 * map directly to clip space.
 */

#ifndef CLAY_RENDERER_POGLIB_H
#define CLAY_RENDERER_POGLIB_H

/* Adjust this include path if clay.h lives elsewhere in your project */
#include <poglib/external/clay.h>

/* -- poglib headers needed for rendering -------------------------------- */
#include <poglib/basic.h>
#include <poglib/math.h>
#include <poglib/gfx/gl/shader.h>
#include <poglib/gfx/gl/texture2d.h>
#include <poglib/gfx/gl/objects.h>
#include <poglib/gfx/gl/types.h>
#include <poglib/font/glfreetypefont.h>


/* Convert clay's 0-255 color to poglib's 0-1 normalized vec4f_t */
static inline vec4f_t clay__color_to_vec4f(Clay_Color c) {
    return (vec4f_t){ c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f };
}

/* -------------------------------------------------------------------------
 * Embedded GLSL shaders
 * ------------------------------------------------------------------------- */

/* Vertex shader -- transforms pixel coords to clip space */
static const char *CLAY_POGLIB_VS =
    "#version 330 core\n"
    "layout (location = 0) in vec2 aPos;\n"
    "layout (location = 1) in vec4 aColor;\n"
    "layout (location = 2) in vec2 aUV;\n"
    "uniform mat4 uProjection;\n"
    "out vec4 vColor;\n"
    "out vec2 vUV;\n"
    "void main() {\n"
    "    gl_Position = uProjection * vec4(aPos, 0.0, 1.0);\n"
    "    vColor = aColor;\n"
    "    vUV = aUV;\n"
    "}\n";

/* Fragment shader -- rounded-rectangle SDF */
static const char *CLAY_POGLIB_RECT_FS =
    "#version 330 core\n"
    "in vec4 vColor;\n"
    "in vec2 vUV;\n"
    "uniform vec4 uCornerRadius;\n"
    "uniform vec2 uSize;\n"
    "out vec4 FragColor;\n"
    "void main() {\n"
    "    vec2 p = vUV * uSize;\n"
    "    vec4 r = uCornerRadius;\n"
    "    float rx = min(p.x, uSize.x - p.x);\n"
    "    float ry = min(p.y, uSize.y - p.y);\n"
    "    float qx = min(rx, r.x);\n"
    "    qx = min(qx, min(r.y, min(r.z, r.w)));\n"
    "    float qy = min(ry, qx);\n"
    "    float d = length(max(vec2(rx - qx, ry - qy), 0.0)) - qx;\n"
    "    float alpha = 1.0 - smoothstep(0.0, 1.0, d);\n"
    "    FragColor = vec4(vColor.rgb, vColor.a * alpha);\n"
    "}\n";

/* Fragment shader -- texture * color (text / images) */
static const char *CLAY_POGLIB_TEX_FS =
    "#version 330 core\n"
    "in vec4 vColor;\n"
    "in vec2 vUV;\n"
    "uniform sampler2D uTexture;\n"
    "out vec4 FragColor;\n"
    "void main() {\n"
    "    FragColor = vColor * texture(uTexture, vUV);\n"
    "}\n";

/* Fragment shader -- glyph (red-channel mask, used with freetype atlas) */
static const char *CLAY_POGLIB_GLYPH_FS =
    "#version 330 core\n"
    "in vec4 vColor;\n"
    "in vec2 vUV;\n"
    "uniform sampler2D uTexture;\n"
    "out vec4 FragColor;\n"
    "void main() {\n"
    "    FragColor = vColor * texture(uTexture, vUV).r;\n"
    "}\n";

/* -------------------------------------------------------------------------
 *  Renderer state
 * ------------------------------------------------------------------------- */

typedef struct {
    f32 width;
    f32 height;
    matrix4f_t projection;
    glshader_t rect_shader;
    glshader_t tex_shader;
    glshader_t glyph_shader;
    bool initialized;
} clay_poglib_renderer_t;

/* -------------------------------------------------------------------------
 *  Public API
 * ------------------------------------------------------------------------- */

/* Initialise renderer with screen dimensions. Creates embedded shaders. */
clay_poglib_renderer_t clay_poglib_init(f32 screen_width, f32 screen_height);

/* Update projection on window resize */
void clay_poglib_resize(clay_poglib_renderer_t *r, f32 screen_width, f32 screen_height);

/* Main render function. Processes a Clay_RenderCommandArray. */
void clay_poglib_render(
    clay_poglib_renderer_t *r,
    Clay_RenderCommandArray commands,
    glfreetypefont_t *fonts,       /* indexed by fontId */
    u32 font_count,
    gltexture2d_t *images,          /* indexed by pointer identity lookup */
    u32 image_count
);

/* Text measurement callback for Clay_SetMeasureTextFunction */
Clay_Dimensions clay_poglib_measure_text(
    Clay_StringSlice text,
    Clay_TextElementConfig *config,
    void *userData                  /* should be glfreetypefont_t* array */
);

/* Destroy shader resources */
void clay_poglib_destroy(clay_poglib_renderer_t *r);

/* Helper: draw a filled rectangle (with optional rounded corners) */
void clay_poglib_draw_rect(
    clay_poglib_renderer_t *r,
    f32 x, f32 y, f32 w, f32 h,
    vec4f_t color,
    Clay_CornerRadius corner_radius
);

/* Helper: draw a textured quad (images and glyphs) */
void clay_poglib_draw_textured_quad(
    clay_poglib_renderer_t *r,
    f32 x, f32 y, f32 w, f32 h,
    vec4f_t color,
    GLuint texture_id,
    bool is_glyph
);

/* Helper: draw a border */
void clay_poglib_draw_border(
    clay_poglib_renderer_t *r,
    f32 x, f32 y, f32 w, f32 h,
    vec4f_t color,
    Clay_BorderWidth width,
    Clay_CornerRadius corner_radius
);


/* =========================================================================
 *  IMPLEMENTATION
 * ========================================================================= */

#ifndef IGNORE_CLAY_POGLIB_IMPLEMENTATION


/* -------------------------------------------------------------------------
 *  Shader creation helpers
 * ------------------------------------------------------------------------- */

static glshader_t clay__compile_shader(const char *vs_src, const char *fs_src) {
    return glshader_from_cstr_init(vs_src, fs_src);
}

/* -------------------------------------------------------------------------
 *  Draw helpers
 * ------------------------------------------------------------------------- */

void clay_poglib_draw_rect(
    clay_poglib_renderer_t *r,
    f32 x, f32 y, f32 w, f32 h,
    vec4f_t color,
    Clay_CornerRadius corner_radius
)
{
    /* Build a full-screen quad in window coordinates */
    glvertex2d_t verts[4] = {
        { { x,     y,     0.0f }, color, { 0.0f, 0.0f } },
        { { x + w, y,     0.0f }, color, { 1.0f, 0.0f } },
        { { x + w, y + h, 0.0f }, color, { 1.0f, 1.0f } },
        { { x,     y + h, 0.0f }, color, { 0.0f, 1.0f } },
    };
    u32 indices[6] = { 0, 1, 2, 2, 3, 0 };

    vao_t vao = vao_init();
    vbo_t vbo = vbo_static_init(verts, sizeof(verts), 4);
    ebo_t ebo = ebo_init(&vbo, indices, 6);

    vao_bind(&vao);
    vao_set_attributes(&vao, &vbo, 2, GL_FLOAT, GL_FALSE, sizeof(glvertex2d_t),
        offsetof(glvertex2d_t, position), 0, VBO_STREAM_TYPE_GEOMETRY);
    vao_set_attributes(&vao, &vbo, 4, GL_FLOAT, GL_FALSE, sizeof(glvertex2d_t),
        offsetof(glvertex2d_t, color), 0, VBO_STREAM_TYPE_GEOMETRY);
    vao_set_attributes(&vao, &vbo, 2, GL_FLOAT, GL_FALSE, sizeof(glvertex2d_t),
        offsetof(glvertex2d_t, uv), 0, VBO_STREAM_TYPE_GEOMETRY);

    glshader_bind(&r->rect_shader);
    glshader_send_uniform_matrix4f(&r->rect_shader, "uProjection", r->projection);
    glshader_send_uniform_vec4f(&r->rect_shader, "uCornerRadius",
        (vec4f_t){ corner_radius.topLeft, corner_radius.topRight,
                   corner_radius.bottomRight, corner_radius.bottomLeft });
    glshader_send_uniform_vec2f(&r->rect_shader, "uSize", (vec2f_t){ w, h });

    vao_draw_with_ebo(&vao, &ebo);

    vao_unbind();
    ebo_destroy(&ebo);
    vbo_destroy(&vbo);
    vao_destroy(&vao);
}

void clay_poglib_draw_textured_quad(
    clay_poglib_renderer_t *r,
    f32 x, f32 y, f32 w, f32 h,
    vec4f_t color,
    GLuint texture_id,
    bool is_glyph
)
{
    glvertex2d_t verts[4] = {
        { { x,     y,     0.0f }, color, { 0.0f, 0.0f } },
        { { x + w, y,     0.0f }, color, { 1.0f, 0.0f } },
        { { x + w, y + h, 0.0f }, color, { 1.0f, 1.0f } },
        { { x,     y + h, 0.0f }, color, { 0.0f, 1.0f } },
    };
    u32 indices[6] = { 0, 1, 2, 2, 3, 0 };

    vao_t vao = vao_init();
    vbo_t vbo = vbo_static_init(verts, sizeof(verts), 4);
    ebo_t ebo = ebo_init(&vbo, indices, 6);

    vao_bind(&vao);
    vao_set_attributes(&vao, &vbo, 2, GL_FLOAT, GL_FALSE, sizeof(glvertex2d_t),
        offsetof(glvertex2d_t, position), 0, VBO_STREAM_TYPE_GEOMETRY);
    vao_set_attributes(&vao, &vbo, 4, GL_FLOAT, GL_FALSE, sizeof(glvertex2d_t),
        offsetof(glvertex2d_t, color), 0, VBO_STREAM_TYPE_GEOMETRY);
    vao_set_attributes(&vao, &vbo, 2, GL_FLOAT, GL_FALSE, sizeof(glvertex2d_t),
        offsetof(glvertex2d_t, uv), 0, VBO_STREAM_TYPE_GEOMETRY);

    glshader_t *const shader = is_glyph ? &r->glyph_shader : &r->tex_shader;
    glshader_bind(shader);
    glshader_send_uniform_matrix4f(shader, "uProjection", r->projection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glshader_send_uniform_ival(shader, "uTexture", 0);

    vao_draw_with_ebo(&vao, &ebo);

    vao_unbind();
    ebo_destroy(&ebo);
    vbo_destroy(&vbo);
    vao_destroy(&vao);
}

void clay_poglib_draw_border(
    clay_poglib_renderer_t *r,
    f32 x, f32 y, f32 w, f32 h,
    vec4f_t color,
    Clay_BorderWidth border_width,
    Clay_CornerRadius corner_radius
)
{
    (void)corner_radius;
    f32 lt = (f32)border_width.left;
    f32 rt = (f32)border_width.right;
    f32 tp = (f32)border_width.top;
    f32 bt = (f32)border_width.bottom;

    /* Draw border as four inset edge rects (square corners for now) */
    if (tp > 0.0f)     clay_poglib_draw_rect(r, x, y, w, tp, color, (Clay_CornerRadius){0});
    if (bt > 0.0f)     clay_poglib_draw_rect(r, x, y + h - bt, w, bt, color, (Clay_CornerRadius){0});
    if (lt > 0.0f)     clay_poglib_draw_rect(r, x, y + tp, lt, h - tp - bt, color, (Clay_CornerRadius){0});
    if (rt > 0.0f)     clay_poglib_draw_rect(r, x + w - rt, y + tp, rt, h - tp - bt, color, (Clay_CornerRadius){0});
}

/* -------------------------------------------------------------------------
 *  API implementation
 * ------------------------------------------------------------------------- */

clay_poglib_renderer_t clay_poglib_init(f32 screen_width, f32 screen_height)
{
    clay_poglib_renderer_t r = {0};
    r.width = screen_width;
    r.height = screen_height;
    r.projection = glms_ortho(0.0f, screen_width, screen_height, 0.0f, -100.0f, 100.0f);

    /* Compile embedded shaders */
    r.rect_shader   = clay__compile_shader(CLAY_POGLIB_VS, CLAY_POGLIB_RECT_FS);
    r.tex_shader    = clay__compile_shader(CLAY_POGLIB_VS, CLAY_POGLIB_TEX_FS);
    r.glyph_shader  = clay__compile_shader(CLAY_POGLIB_VS, CLAY_POGLIB_GLYPH_FS);
    r.initialized = true;

    return r;
}

void clay_poglib_resize(clay_poglib_renderer_t *r, f32 screen_width, f32 screen_height)
{
    r->width  = screen_width;
    r->height = screen_height;
    r->projection = glms_ortho(0.0f, screen_width, screen_height, 0.0f, -100.0f, 100.0f);
}

void clay_poglib_render(
    clay_poglib_renderer_t *r,
    Clay_RenderCommandArray commands,
    glfreetypefont_t *fonts,
    u32 font_count,
    gltexture2d_t *images,
    u32 image_count
)
{
    (void)images;
    (void)image_count;

    GLboolean scissor_was_enabled = glIsEnabled(GL_SCISSOR_TEST);
    if (!scissor_was_enabled)
        glEnable(GL_SCISSOR_TEST);

    /* Enable blending for alpha */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    for (i32 i = 0; i < commands.length; i++) {
        Clay_RenderCommand *cmd = Clay_RenderCommandArray_Get(&commands, i);
        Clay_BoundingBox bb = cmd->boundingBox;

        switch (cmd->commandType) {

        case CLAY_RENDER_COMMAND_TYPE_RECTANGLE: {
            Clay_RectangleRenderData *d = &cmd->renderData.rectangle;
            clay_poglib_draw_rect(r, bb.x, bb.y, bb.width, bb.height,
                clay__color_to_vec4f(d->backgroundColor), d->cornerRadius);
            break;
        }

        case CLAY_RENDER_COMMAND_TYPE_BORDER: {
            Clay_BorderRenderData *d = &cmd->renderData.border;
            clay_poglib_draw_border(r, bb.x, bb.y, bb.width, bb.height,
                clay__color_to_vec4f(d->color), d->width, d->cornerRadius);
            break;
        }

        case CLAY_RENDER_COMMAND_TYPE_TEXT: {
            Clay_TextRenderData *d = &cmd->renderData.text;
            if (d->fontId >= font_count) break;
            const glfreetypefont_t *font = &fonts[d->fontId];
            vec4f_t col = clay__color_to_vec4f(d->textColor);
            f32 x = bb.x;
            f32 y = bb.y;

            /* Render each character using the freetype font atlas */
            for (i32 j = 0; j < d->stringContents.length; j++) {
                u8 c = (u8)d->stringContents.chars[j];
                if (c < 32 || c >= 128) continue;
                if (c == '\n') {
                    x = bb.x;
                    y += (d->lineHeight > 0) ? (f32)d->lineHeight : (f32)font->fontsize;
                    continue;
                }

                f32 ax = font->fontatlas[c].ax;
                f32 bw = font->fontatlas[c].bw;
                f32 bh = font->fontatlas[c].bh;
                f32 bl = font->fontatlas[c].bl;
                f32 bt = font->fontatlas[c].bt;
                f32 tx = font->fontatlas[c].tx;
                f32 ty = font->fontatlas[c].ty;

                f32 glyph_x = x + bl;
                f32 glyph_y = y + bt - bh;

                if (bw > 0.0f && bh > 0.0f) {
                    /* Build UV quad: tl, tr, br, bl */
                    f32 tx2 = tx + bw / (f32)font->width;
                    f32 ty2 = ty + bh / (f32)font->height;

                    glvertex2d_t verts[4] = {
                        { { glyph_x,        glyph_y,        0.0f }, col, { tx,  ty  } },
                        { { glyph_x + bw,   glyph_y,        0.0f }, col, { tx2, ty  } },
                        { { glyph_x + bw,   glyph_y + bh,   0.0f }, col, { tx2, ty2 } },
                        { { glyph_x,        glyph_y + bh,   0.0f }, col, { tx,  ty2 } },
                    };
                    u32 idx[6] = { 0, 1, 2, 2, 3, 0 };

                    vao_t vao = vao_init();
                    vbo_t vbo = vbo_static_init(verts, sizeof(verts), 4);
                    ebo_t ebo = ebo_init(&vbo, idx, 6);

                    vao_bind(&vao);
                    vao_set_attributes(&vao, &vbo, 2, GL_FLOAT, GL_FALSE,
                        sizeof(glvertex2d_t), offsetof(glvertex2d_t, position),
                        0, VBO_STREAM_TYPE_GEOMETRY);
                    vao_set_attributes(&vao, &vbo, 4, GL_FLOAT, GL_FALSE,
                        sizeof(glvertex2d_t), offsetof(glvertex2d_t, color),
                        0, VBO_STREAM_TYPE_GEOMETRY);
                    vao_set_attributes(&vao, &vbo, 2, GL_FLOAT, GL_FALSE,
                        sizeof(glvertex2d_t), offsetof(glvertex2d_t, uv),
                        0, VBO_STREAM_TYPE_GEOMETRY);

                    glshader_bind(&r->glyph_shader);
                    glshader_send_uniform_matrix4f(&r->glyph_shader, "uProjection",
                        r->projection);
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, font->texture.id);
                    glshader_send_uniform_ival(&r->glyph_shader, "uTexture", 0);

                    vao_draw_with_ebo(&vao, &ebo);

                    vao_unbind();
                    ebo_destroy(&ebo);
                    vbo_destroy(&vbo);
                    vao_destroy(&vao);
                }

                x += ax;
            }
            break;
        }

        case CLAY_RENDER_COMMAND_TYPE_IMAGE: {
            Clay_ImageRenderData *d = &cmd->renderData.image;
            if (d->imageData) {
                gltexture2d_t *tex = (gltexture2d_t *)d->imageData;
                clay_poglib_draw_textured_quad(r,
                    bb.x, bb.y, bb.width, bb.height,
                    (vec4f_t){1,1,1,1}, tex->id, false);
            }
            break;
        }

        case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START: {
            glScissor((GLint)bb.x, (GLint)(r->height - bb.y - bb.height),
                      (GLsizei)bb.width, (GLsizei)bb.height);
            break;
        }

        case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END: {
            /* Reset scissor to full viewport */
            glScissor(0, 0, (GLsizei)r->width, (GLsizei)r->height);
            break;
        }

        case CLAY_RENDER_COMMAND_TYPE_CUSTOM: {
            /* Custom elements - user handles via cmd->renderData.custom.customData */
            break;
        }

        default: break;
        }
    }

    if (!scissor_was_enabled)
        glDisable(GL_SCISSOR_TEST);
}

Clay_Dimensions clay_poglib_measure_text(
    Clay_StringSlice text,
    Clay_TextElementConfig *config,
    void *userData
)
{
    glfreetypefont_t *fonts = (glfreetypefont_t *)userData;
    if (!fonts) return (Clay_Dimensions){0, 0};

    u32 fid = config->fontId;
    const glfreetypefont_t *font = &fonts[fid];

    f32 width = 0.0f;
    f32 height = (f32)font->fontsize;
    f32 line_width = 0.0f;
    f32 max_width = 0.0f;

    for (i32 i = 0; i < text.length; i++) {
        u8 c = (u8)text.chars[i];
        if (c >= 128) continue;
        if (c == '\n') {
            max_width = fmaxf(max_width, line_width);
            line_width = 0.0f;
            height += (f32)font->fontsize;
            continue;
        }
        f32 ax = font->fontatlas[c].ax;
        line_width += ax + (f32)config->letterSpacing;
    }
    max_width = fmaxf(max_width, line_width);
    width = max_width;

    return (Clay_Dimensions){ width, height };
}

void clay_poglib_destroy(clay_poglib_renderer_t *r)
{
    if (!r->initialized) return;
    glshader_destroy(&r->rect_shader);
    glshader_destroy(&r->tex_shader);
    glshader_destroy(&r->glyph_shader);
    r->initialized = false;
}

#endif /* IGNORE_CLAY_POGLIB_IMPLEMENTATION */

#endif /* CLAY_RENDERER_POGLIB_H */
