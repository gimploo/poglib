#pragma once
/*======================================================
 // OpenGL font rendering with ascii style spritesheets
======================================================*/

#include "../math/la.h"
#include "../simple/glrenderer2d.h"


typedef struct glbitmapfont_t glbitmapfont_t;


#define             glbitmapfont_default_init()  glbitmapfont_init("./lib/res/ascii_fonts/glyph_atlas.png", 16, 6)
glbitmapfont_t      glbitmapfont_init(const char *file_path, const u32 tile_count_width, const u32 tile_count_height);
void                glbitmapfont_set_text(glbitmapfont_t *handler, const char *text, vec2f_t norm_position, f32 fontsize); 
void                glbitmapfont_draw(glbitmapfont_t *font);
void                glbitmapfont_destroy(glbitmapfont_t *);






#ifndef IGNORE_GLBITMAPFONT_IMPLEMENTATION

#define MAX_ASCII_TILES_COUNT 200

const char * const ascii_font_vs = 
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

const char * const ascii_font_fs = 
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
        "FragColor = texture(texture1, TexCoord) ;\n"
    "}";



struct glbitmapfont_t {
    
    glrenderer2d_t renderer;

    struct {

        char          character;
        quadf_t       texture_coord;

    } font_atlas[MAX_ASCII_TILES_COUNT];

    glbatch_t   batches;
};



gltexture2d_t __ascii_load_texture(const char *file_path)
{
    if (file_path == NULL) eprint("file argument is null");

    GLuint id;
    int width = 0, height = 0, bpp = 0;
    u8 *buf = NULL;

    stbi_set_flip_vertically_on_load(1);                                
    buf = stbi_load(file_path, &width, &height, &bpp, 0); //RGB
    if (buf == NULL) eprint("Failed to load texture");

    GL_CHECK(glGenTextures(1, &id));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, id));

    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));	
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));


    GL_CHECK(glTexImage2D(
        GL_TEXTURE_2D, 
        0, 
        GL_RGBA8, // The format opengl will store the pixel data in
        width,
        height,
        0,
        GL_RGB, // The format the buf variable is in
        GL_UNSIGNED_BYTE,
        buf
     ));

    GL_CHECK(glGenerateMipmap(GL_TEXTURE_2D));

    GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
    
    GL_LOG("Texture `%i` successfully created", id);

    return (gltexture2d_t) {
        .id        = id,
        .file_path = file_path,
        .buf       = buf,
        .width     = width,
        .height    = height,
        .bpp       = bpp,
    };
}


//NOTE: this function only handles a ascii style spritesheets
glbitmapfont_t glbitmapfont_init(const char *file_path, const u32 tile_count_width, const u32 tile_count_height)
{
    if (file_path == NULL) eprint("file_path argument is null");

    //Shader
    glshader_t shader = glshader_from_cstr_init(ascii_font_vs, ascii_font_fs);
    glshader_t *pshader = (glshader_t *)calloc(1, sizeof(shader));
    assert(pshader);
    memcpy(pshader, &shader, sizeof(shader));

    //Texture
    gltexture2d_t texture = __ascii_load_texture(file_path);
    gltexture2d_t *ptexture = (gltexture2d_t *)calloc(1, sizeof(texture));
    assert(ptexture);
    memcpy(ptexture, &texture, sizeof(texture));


    glbitmapfont_t output = {
        .renderer           = glrenderer2d_init(pshader, ptexture),
        .batches            = glbatch_init(KB, glquad_t )
    };

    f32 norm_font_width    = normalize(texture.width / tile_count_width, 0.0f, texture.width);
    f32 norm_font_height   = normalize(texture.height / tile_count_height, 0.0f, texture.height);

    for (u32 v = 0, tile_index = 0; v < tile_count_height; v++)
    {
        for (u32 u = 0; u < tile_count_width; u++)
        {
            
            f32 left_U      = u * norm_font_width;
            f32 right_U     = left_U + norm_font_width; 
            f32 top_V       = -(v * norm_font_height);
            f32 bottom_V    = top_V - norm_font_height ;

            output.font_atlas[tile_index].character     = (' ' + tile_index);
            output.font_atlas[tile_index].texture_coord = (quadf_t ){
                left_U,     top_V,      0.0f,   
                right_U,    top_V,      0.0f,
                right_U,    bottom_V,   0.0f,
                left_U,     bottom_V,   0.0f
            };



#if 0
            quadf_t tex_coord = output.font_atlas[tile_index].texture_coord;
            printf("Character = %c \n", output.font_atlas[tile_index].character);
            printf("norm font height = %f\n", output.norm_font_height);
            printf("norm font width = %f\n", output.norm_font_width);
            printf("TexCoord: " VEC2F_FMT "\n",VEC2F_ARG(&tex_coord.vertex[0])); 
            printf("TexCoord: " VEC2F_FMT "\n",VEC2F_ARG(&tex_coord.vertex[1])); 
            printf("TexCoord: " VEC2F_FMT "\n",VEC2F_ARG(&tex_coord.vertex[2])); 
            printf("TexCoord: " VEC2F_FMT "\n",VEC2F_ARG(&tex_coord.vertex[3])); 
#endif

            ++tile_index;


        }
    }



    return output;
    
}

void glbitmapfont_set_text(
        glbitmapfont_t  *font, 
        const char      *text, 
        vec2f_t   position,
        f32 fontsize)
{
    if (font == NULL)    eprint("font argument is null");
    if (text == NULL)    eprint("text argument is null");


    f32     norm_font_width     = fontsize;
    f32     norm_font_height    = fontsize;


    vec2f_t x_offset = {0};

    for (int i = 0; text[i] != '\0'; i++) 
    {
        u32 tile_index = text[i] - ' ';

        quadf_t quad = quadf(
                vec3f_add(vec3f(position), vec3f(x_offset)), 
                norm_font_width, 
                norm_font_height);

#if 0
        glbitmapfont_t *output = font; // NOTE: delete this line later
        printf("Tile index %i\n", tile_index);
        printf("Character = %c \n", output->font_atlas[tile_index].character);
        printf("norm font width = %f\n", output->norm_font_width);
        printf("norm font height = %f\n", output->norm_font_height);
        printf("TexCoord: " VEC2F_FMT "\n",VEC2F_ARG(&output->font_atlas[tile_index].texture_coord.vertex[0])); 
        printf("TexCoord: " VEC2F_FMT "\n",VEC2F_ARG(&output->font_atlas[tile_index].texture_coord.vertex[1])); 
        printf("TexCoord: " VEC2F_FMT "\n",VEC2F_ARG(&output->font_atlas[tile_index].texture_coord.vertex[2])); 
        printf("TexCoord: " VEC2F_FMT "\n",VEC2F_ARG(&output->font_atlas[tile_index].texture_coord.vertex[3])); 
#endif 


        glquad_t tmp = glquad(quad, (vec4f_t ){0.0f, 0.0f, 0.0f, 1.0f}, font->font_atlas[tile_index].texture_coord, 0);

        glbatch_put(&font->batches, tmp);

#if 0
        printf("Position: " VEC2F_FMT"\n", vertices[0], vertices[1]);
        printf("--------- " VEC2F_FMT"\n", vertices[3], vertices[4]); 
        printf("          " VEC2F_FMT"\n", vertices[6], vertices[7]); 
        printf("          " VEC2F_FMT"\n", vertices[9], vertices[10]); 
#endif


        x_offset = vec2f_add(
                x_offset, 
                (vec2f_t) {norm_font_width, 0.0f}
        );


    }
}

void glbitmapfont_draw(glbitmapfont_t *font)
{
    assert(font);

    glrenderer2d_draw_from_batch(&font->renderer, &font->batches);

}

void glbitmapfont_destroy(glbitmapfont_t *self)
{
    if (self == NULL) eprint("argument is null");

    glrenderer2d_destroy(&self->renderer);
    glshader_destroy(self->renderer.__shader);
    gltexture2d_destroy(self->renderer.__texture);
    glbatch_destroy(&self->batches);
    free(self->renderer.__shader);
    free(self->renderer.__texture);


}

#endif
