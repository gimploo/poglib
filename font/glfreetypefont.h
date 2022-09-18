#pragma once
#include <poglib/basic.h>
#include "../gfx/glrenderer2d.h"
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

typedef struct glfreetypefont_t {

    u32 width;
    u32 height;
    u32 fontsize;

    struct {

        f32 ax;	// advance.x
		f32 ay;	// advance.y

		f32 bw;	// bitmap.width;
		f32 bh;	// bitmap.height;

		f32 bl;	// bitmap_left;
		f32 bt;	// bitmap_top;

		f32 tx;	// x offset of glyph in texture coordinates
		f32 ty;	// y offset of glyph in texture coordinates

    } fontatlas[MAX_CHARACTERS_IN_FREETYPE];

    glshader_t      shader;
    gltexture2d_t   texture;

} glfreetypefont_t ;


glfreetypefont_t    glfreetypefont_init(const char *filepath, const u32 fontsize);
void                glfreetypefont_add_text_to_batch(const glfreetypefont_t *self, glbatch_t *batch, const char *text, vec2f_t pos, vec4f_t color);
void                glfreetypefont_draw(const glfreetypefont_t *self, const glbatch_t *batch);
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
        "FragColor = ourColor * texture(texture1, TexCoord).a;\n"
    "}";




glfreetypefont_t glfreetypefont_init(const char *filepath, const u32 fontsize)
{
    assert(filepath);

    glfreetypefont_t o = {
        .width = 0,
        .height = 0,
        .fontsize = fontsize,
        .fontatlas = {0},
    };

    gltexture2d_t tex = {
        .file_path = filepath,
        .buf = NULL,
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

    for(u32 i = 0; i < 128; i++) 
    {
        if(FT_Load_Char(face, i, FT_LOAD_RENDER)) 
            eprint("Loading character %c failed!", i);

        if (roww + g->bitmap.width + 1 >= tex.width) {
				o.width = MAX(o.width, roww);
				o.height += rowh;
				roww = 0;
				rowh = 0;
        }
        roww += g->bitmap.width + 1;
        rowh = MAX(rowh, g->bitmap.rows);
    }

    tex.width  = o.width   = MAX(o.width, roww); 
    tex.height = o.height += rowh; 

    // generate texture
    GL_CHECK(glActiveTexture(GL_TEXTURE0));
    GL_CHECK(glGenTextures(1, &tex.id));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, tex.id));
    GL_CHECK(glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_ALPHA,
        tex.width,
        tex.height,
        0,
        GL_ALPHA,
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

			glTexSubImage2D(GL_TEXTURE_2D, 0, ox, oy, g->bitmap.width, g->bitmap.rows, GL_ALPHA, GL_UNSIGNED_BYTE, g->bitmap.buffer);
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

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    //Shader
    o.shader = glshader_from_cstr_init(freetype_vs, freetype_fs);
    o.texture = tex;

    return o;
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

        quadf_t quad = quadf((vec3f_t ){x2, -y2, 0.0f}, w, h);

        quadf_t uv = {
          self->fontatlas[c].tx, self->fontatlas[c].ty, 0.0f,
          self->fontatlas[c].tx + self->fontatlas[c].bw / self->width, self->fontatlas[c].ty, 0.0f,
          self->fontatlas[c].tx + self->fontatlas[c].bw / self->width, self->fontatlas[c].ty + self->fontatlas[c].bh / self->height, 0.0f,
          self->fontatlas[c].tx, self->fontatlas[c].ty + self->fontatlas[c].bh / self->height, 0.0f,
        };

        glquad_t stuff = glquad(quad, color, uv, 0);

        glbatch_put(batch, stuff);
    }
}


void glfreetypefont_draw(const glfreetypefont_t *self, const glbatch_t *batch)
{
    glrenderer2d_t rd2d = {
        .shader = &self->shader,
        .texture = &self->texture
    };
    glrenderer2d_draw_from_batch(&rd2d, batch);
}

void glfreetypefont_destroy(glfreetypefont_t *self)
{
    glshader_destroy(&self->shader);
    gltexture2d_destroy(&self->texture);
}

#endif
