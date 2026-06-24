#pragma once
#include <poglib/basic.h>
#include "../gfx/glrenderer2d.h"
#include "../gfx/glrenderer3d.h"
#if defined(WINDOW_GLFW)
#include <poglib/application/window/glfw_window.h>
#elif defined(WINDOW_SDL)
#include <poglib/application/window/sdl_window.h>
#else
#include <poglib/application/window/sdl_window.h>
#endif
#include <ft2build.h>
#include FT_FREETYPE_H 

/*=============================================================================
                    - TEXT RENDERER (FreeTypeFont) -
=============================================================================*/
//NOTE: text texture positioning origin starts at the bottom left of the 
//quad, when passing the position

#define MAX_CHARACTERS_IN_FREETYPE 128
#define DEFAULT_FONT_ROBOTO_MEDIUM_FILEPATH "lib/poglib/res/ttf_fonts/Roboto-Medium.ttf"

typedef struct glfreetypefont_t {

    u32 width;
    u32 height;
    u32 fontsize;

    struct {

        f32 ax;// advance.x
        f32 ay;// advance.y
        f32 bw;// bitmap.width;
        f32 bh;// bitmap.height;
        f32 bl;// bitmap_left;
        f32 bt;// bitmap_top;
        f32 tx;// x offset of glyph in texture coordinates
        f32 ty;// y offset of glyph in texture coordinates

    } fontatlas[MAX_CHARACTERS_IN_FREETYPE];

    glshader_t      shader;
    gltexture2d_t   texture;

    struct {
        bool shader_initialized;
    } internals;

} glfreetypefont_t ;


glfreetypefont_t    glfreetypefont_init(const char *filepath, const u32 fontsize, bool ignore_shader);

glquad_t            glfreetypefont_generate_glquad_for_char(const glfreetypefont_t *self, const char c, const vec3f_t pos, const vec4f_t color);
void                glfreetypefont_add_text_to_batch(
                        const glfreetypefont_t *self, 
                        glbatch_t *batch, 
                        const char *text, 
                        vec2f_t pos, 
                        vec4f_t color);
void                glfreetypefont_draw_with_r2d(const glfreetypefont_t *self, const glbatch_t *batch);
void                glfreetypefont_destroy(glfreetypefont_t *self);


/*-----------------------------------------------------------------------------
                            IMPLEMENTATION
-----------------------------------------------------------------------------*/


#ifndef IGNORE_GLFREETYPEFONT_IMPLEMENTATION

const char * freetype_vs = 
    "#version 330 core\n"
    "\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec4 aColor;\n"
    "layout (location = 2) in vec2 aTexCoord;\n"
    "\n"
    "out vec4 ourColor;\n"
    "out vec2 TexCoord;\n"
    "\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos, 1.0f);\n"
    "   ourColor = aColor;\n"
    "   TexCoord = aTexCoord;\n"
    "}";

const char * freetype_fs = 
    "#version 330 core\n"
    "\n"
    "in vec2 TexCoord;\n"
    "in vec4 ourColor;\n"
    "\n"
    "out vec4 FragColor;\n"
    "\n"
    "uniform sampler2D texture1;\n"
    "\n"
    "void main()\n"
    "{\n"
        "FragColor = ourColor * texture(texture1, TexCoord).r;\n"
    "}";



glfreetypefont_t glfreetypefont_init(const char *filepath, const u32 fontsize, bool ignore_shader)
{
    assert(filepath);

    glfreetypefont_t o = {
        .width = 0,
        .height = 0,
        .fontsize = fontsize,
        .fontatlas = {0},
        .internals = {
            .shader_initialized = !ignore_shader
        }
    };

    gltexture2d_t tex = {
        .buf = NULL,
        .filepath = str__from_cstr(filepath, strlen(filepath))
    };

    FT_Library  ft;
    FT_Face     face;

    if (FT_Init_FreeType(&ft)) 
        eprint("could not init freetype library");

    if (FT_New_Face(ft, filepath, 0, &face))
        eprint("Failed to load font");

    FT_Set_Pixel_Sizes(face, 0, fontsize);

    u32 roww = 0;
    u32 rowh = 0;
    FT_GlyphSlot g = face->glyph;
    u32 atlas_fixed_width = 512;

    for(u32 i = 32; i < 128; i++) 
    {
        if(FT_Load_Char(face, i, FT_LOAD_RENDER)) 
            eprint("Loading character %c failed!", i);

        if (roww + g->bitmap.width + 1 >= atlas_fixed_width) {
            o.width = MAX(o.width, roww);
            o.height += rowh;
            roww = 0;
            rowh = 0;
        }
        roww += g->bitmap.width + 1;
        rowh = MAX(rowh, g->bitmap.rows);
    }

    tex.width  = o.width   = atlas_fixed_width; 
    tex.height = o.height += rowh; 

    // generate texture
    GL_CHECK(glActiveTexture(GL_TEXTURE0));
    GL_CHECK(glGenTextures(1, &tex.id));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, tex.id));
    GL_CHECK(glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_R8,
        tex.width,
        tex.height,
        0,
        GL_RED,
        GL_UNSIGNED_BYTE,
        0
    ));
    GL_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, 1)); // disable byte-alignment restriction
    // set texture options
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    i32 ox = 0;
    i32 oy = 0;
    rowh = 0;

    // load first 128 characters of ASCII set
    for (u32 c = 32; c < 128; c++)
    {
        // Load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) 
            eprint("ERROR::FREETYTPE: Failed to load Glyph");

        if (ox + g->bitmap.width + 1 >= o.width) {
            oy += rowh;
            rowh = 0;
            ox = 0;
        }

        GL_CHECK(glTexSubImage2D(GL_TEXTURE_2D, 0, ox, oy, g->bitmap.width, g->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer));
        o.fontatlas[c].ax = g->advance.x >> 6;
        o.fontatlas[c].ay = g->advance.y >> 6;

        o.fontatlas[c].bw = g->bitmap.width;
        o.fontatlas[c].bh = g->bitmap.rows;

        o.fontatlas[c].bl = g->bitmap_left;
        o.fontatlas[c].bt = g->bitmap_top;

        o.fontatlas[c].tx = ox / (float)o.width;
        o.fontatlas[c].ty = oy / (float)o.height;

        rowh = MAX(rowh, g->bitmap.rows);
        ox += g->bitmap.width + 1;
    }
    GL_CHECK(glActiveTexture(GL_TEXTURE0));

    memcpy(&o.texture, &tex, sizeof(tex));

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    //Shader
    if (!ignore_shader) {
        o.shader = glshader_from_cstr_init(freetype_vs, freetype_fs);
    }

    return o;
}

//NOTE: Pos is expected to be for origin set at Top left corner (Window Width x Window Height in pixels)
glquad_t glfreetypefont_generate_glquad_for_char(const glfreetypefont_t *self, const char c, const vec3f_t pos, const vec4f_t color)
{
    ASSERT(self);

    const f32 glyph_width = self->fontatlas[(u8)c].bw;
    const f32 glyph_height = self->fontatlas[(u8)c].bh;
    const f32 x = pos.x + self->fontatlas[(u8)c].bl;
    const f32 y = pos.y + self->fontatlas[(u8)c].bt - glyph_height;

    if(!glyph_width || !glyph_height) eprint("Glyph has no pixels");

    const rect_t uv = {
        self->fontatlas[(u8)c].tx, self->fontatlas[(u8)c].ty,
        self->fontatlas[(u8)c].tx + self->fontatlas[(u8)c].bw / self->width, self->fontatlas[(u8)c].ty,
        self->fontatlas[(u8)c].tx + self->fontatlas[(u8)c].bw / self->width, self->fontatlas[(u8)c].ty + self->fontatlas[(u8)c].bh / self->height,
        self->fontatlas[(u8)c].tx, self->fontatlas[(u8)c].ty + self->fontatlas[(u8)c].bh / self->height,
    };
    return glquad(
        quadf_for_window_coordinates((vec3f_t ){ x, y, pos.z }, glyph_width, glyph_height),
        color, 
        uv
    );
}

rect_t glfreetypefont_generate_uv_for_char__old(const glfreetypefont_t *self, const char c, const vec3f_t pos, const vec4f_t color)
{
    ASSERT(self);

    const f32 glyph_width = self->fontatlas[(u8)c].bw;
    const f32 glyph_height = self->fontatlas[(u8)c].bh;

    if(!glyph_width || !glyph_height) eprint("Glyph has no pixels");

    const rect_t uv = {
        self->fontatlas[(u8)c].tx, self->fontatlas[(u8)c].ty,
        self->fontatlas[(u8)c].tx + self->fontatlas[(u8)c].bw / self->width, self->fontatlas[(u8)c].ty,
        self->fontatlas[(u8)c].tx + self->fontatlas[(u8)c].bw / self->width, self->fontatlas[(u8)c].ty + self->fontatlas[(u8)c].bh / self->height,
        self->fontatlas[(u8)c].tx, self->fontatlas[(u8)c].ty + self->fontatlas[(u8)c].bh / self->height,
    };
    return uv;
}

typedef struct {
    box_t uv;
    u32 xoffset;
    u32 yoffset;
} font_character_t;

box_t glfreetypefont_generate_uv_for_char(const glfreetypefont_t *self, const char c)
{
    ASSERT(self);
    // Directly return the pre-calculated normalized values from the struct
    return (box_t){ 
        .x = self->fontatlas[(u8)c].tx, 
        .y = self->fontatlas[(u8)c].ty, 
        .width = self->fontatlas[(u8)c].bw / (f32)self->width, 
        .height = self->fontatlas[(u8)c].bh / (f32)self->height 
    };
}
void glfreetypefont_add_text_to_batch(const glfreetypefont_t *self, glbatch_t *batch, const char *text, vec2f_t pos, vec4f_t color)
{
    assert(self);
    assert(text);
    assert(batch);

    f32 x = pos.x;
    f32 y = pos.y;

    f32 sx = 2.0f / global_window->width;
    f32 sy = 2.0f / global_window->height;
    
    u32 textlength = strlen(text);
    for(u32 i = 0; i < textlength; i++) 
    { 
        char c = text[i];
        /* Calculate the vertex and texture coordinates */
        float x2 = x + self->fontatlas[c].bl * sx;
        float y2 = -y - self->fontatlas[c].bt * sy;
        float w = self->fontatlas[c].bw * sx;
        float h = self->fontatlas[c].bh * sy;

        /* Advance the cursor to the start of the next character */
        x += self->fontatlas[c].ax * sx;
        y += self->fontatlas[c].ay * sy;

        /* Skip glyphs that have no pixels */
        if(!w || !h) continue;

        const quadf_t quad = quadf((vec3f_t ){x2, -y2, -1.0f}, w, h);

        const rect_t uv = {
          self->fontatlas[c].tx, self->fontatlas[c].ty,
          self->fontatlas[c].tx + self->fontatlas[c].bw / self->width, self->fontatlas[c].ty,
          self->fontatlas[c].tx + self->fontatlas[c].bw / self->width, self->fontatlas[c].ty + self->fontatlas[c].bh / self->height,
          self->fontatlas[c].tx, self->fontatlas[c].ty + self->fontatlas[c].bh / self->height,
        };

        const glquad_t stuff = glquad(quad, color, uv);

        glbatch_put(batch, stuff);
    }
}


void glfreetypefont_draw_with_r2d(const glfreetypefont_t *self, const glbatch_t *batch)
{
    glrenderer2d_t rd2d = {
        .shader = &self->shader,
        .texture = &self->texture
    };
    glrenderer2d_draw_from_batch(&rd2d, batch);
}

void glfreetypefont_destroy(glfreetypefont_t *self)
{
    if (self->internals.shader_initialized) {
        glshader_destroy(&self->shader);
    }
    gltexture2d_destroy(&self->texture);
}

glrendercall_t glfreetypefont_get_renderconfig(glfreetypefont_t *self, buffer_t geomerty_vtx_buffer, buffer_t geometry_idx_buffer)
{
    return (glrendercall_t) {
        .draw_mode = GL_TRIANGLES,
        .vtx = {
            [VBO_STREAM_TYPE_GEOMETRY] = geomerty_vtx_buffer
        },
        .idx = {
            .data = geometry_idx_buffer.raw_data,
            .nmemb = geometry_idx_buffer.size / sizeof(u32)
        },
        .attrs = {
            .count = 3,
            .attr = {
                [0] = {
                    .type = GL_FLOAT,
                    .ncmp = 3,
                    .interleaved = {0}
                },
                [1] = {
                    .type = GL_FLOAT,
                    .ncmp = 4,
                    .interleaved = {0},
                },
                [2] = {
                    .type = GL_FLOAT,
                    .ncmp = 2,
                    .interleaved = {0},
                }
            }
        },
        .textures = {
            .count = 1,
            .items = {
                [0] = {
                    .type = GL_TEXTURE_TYPE_NORMAL,
                    .source.normal_texture = &self->texture 
                },
            }
        }, 
        .shader_config = {
            .shader = &self->shader,
        },
    };
}

#endif
