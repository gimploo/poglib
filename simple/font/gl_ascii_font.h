#ifndef __MY__FONT__H__
#define __MY__FONT__H__

/*======================================================
 // OpenGL font rendering with ascii style spritesheets
======================================================*/

#include "../../math/la.h"
#include "../../math/shapes.h"
#include "../gl_renderer2d.h"

#define MAX_ASCII_TILES_COUNT 200

typedef struct character_info_t  {

    char                character;
    vec2f_t             texture_coord[4];

    f32                 font_width;
    f32                 font_height;

    f32                 norm_font_width;
    f32                 norm_font_height;

} character_info_t;

typedef struct gl_ascii_font_t {
    
    gl_shader_t         shader;
    gl_texture2d_t      texture;

    u32                 font_count_width;
    u32                 font_count_height;
    character_info_t    font_atlas[MAX_ASCII_TILES_COUNT];


} gl_ascii_font_t;


const char * const ascii_font_vs = 
    "#version 330 core\n"
    "\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec2 aTexCoord;\n"
    "\n"
    "out vec2 TexCoord;\n"
    "\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos, 1.0f);\n"
    "   TexCoord    = aTexCoord;\n"
    "}";

const char * const ascii_font_fs = 
    "#version 330 core\n"
    "\n"
    "in vec2 TexCoord;\n"
    "\n"
    "out vec4 FragColor;\n"
    "\n"
    "uniform sampler2D texture1;\n"
    "\n"
    "void main()\n"
    "{\n"
        "FragColor = texture(texture1, TexCoord);\n"
    "}";


/*----------------------------------------------------------------
 // Declarations
----------------------------------------------------------------*/

gl_ascii_font_t gl_ascii_font_init(const char *file_path, u32 tile_count_width, u32 tile_count_height);
void            gl_ascii_font_render_text(gl_ascii_font_t *handler, const char *text, const vec2f_t norm_position, const f32 norm_font_size); 
void            gl_ascii_font_destroy(gl_ascii_font_t *);

/*----------------------------------------------------------------
 // Implementation
----------------------------------------------------------------*/

static inline gl_texture2d_t __ascii_load_texture(const char *file_path)
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

    return (gl_texture2d_t) {
        .id        = id,
        .file_path = file_path,
        .buf       = buf,
        .width     = width,
        .height    = height,
        .bpp       = bpp,
    };
}


//NOTE: this function only handles a ascii style spritesheets
gl_ascii_font_t gl_ascii_font_init(const char *file_path, u32 tile_count_width, u32 tile_count_height)
{
    if (file_path == NULL) eprint("file_path argument is null");

    gl_ascii_font_t output = {

        .shader     = gl_shader_from_cstr_init(ascii_font_vs, ascii_font_fs),
        .texture    = __ascii_load_texture(file_path),
    };

    const u32 font_width    = output.texture.width / tile_count_width;
    const u32 font_height   = output.texture.height / tile_count_height;

    const f32 norm_sprite_height  = normalize(font_height, 0.0f, output.texture.height); 
    const f32 norm_sprite_width   = normalize(font_width, 0.0f, output.texture.width);

    for (u32 v = 0, tile_index = 0; v < tile_count_height; v++)
    {
        for (u32 u = 0; u < tile_count_width; u++)
        {
            
            f32 left_U      = u * norm_sprite_width;
            f32 right_U     = left_U + norm_sprite_width; 
            f32 top_V       = -(v * norm_sprite_height);
            f32 bottom_V    = top_V - norm_sprite_height ;

            vec2f_t tex_coord[4] = {

                {left_U, top_V},
                {right_U, top_V},
                {right_U, bottom_V},
                {left_U, bottom_V},
            };

            output.font_atlas[tile_index].character = (' ' + tile_index);
            output.font_atlas[tile_index].font_height = font_height;
            output.font_atlas[tile_index].font_width =  font_width;
            output.font_atlas[tile_index].norm_font_width = norm_sprite_width;
            output.font_atlas[tile_index].norm_font_height = norm_sprite_height;


            matrix4f_copy(
                    output.font_atlas[tile_index].texture_coord,
                    tex_coord
            );

#if 0
            printf("Character = %c \n", output.font_atlas[tile_index].character);
            printf("font height = %f\n", output.font_atlas[tile_index].font_height);
            printf("font width = %f\n", output.font_atlas[tile_index].font_width);
            printf("norm font height = %f\n", output.font_atlas[tile_index].norm_font_height);
            printf("norm font width = %f\n", output.font_atlas[tile_index].norm_font_width);
            printf("TexCoord: " VEC2F_FMT "\n",VEC2F_ARG(tex_coord[0])); 
            printf("TexCoord: " VEC2F_FMT "\n",VEC2F_ARG(tex_coord[1])); 
            printf("TexCoord: " VEC2F_FMT "\n",VEC2F_ARG(tex_coord[2])); 
            printf("TexCoord: " VEC2F_FMT "\n",VEC2F_ARG(tex_coord[3])); 
#endif


            ++tile_index;

        }
    }


    return output;
    
}

void gl_ascii_font_render_text(
        gl_ascii_font_t   *handler, 
        const char        *text, 
        const vec2f_t     position, 
        const f32         norm_font_size)
{
    if (handler == NULL)    eprint("handler argument is null");
    if (text == NULL)       eprint("text argument is null");
    if(strlen(text) > KB) eprint("text is too big exceeded 1024 bytes");


    const character_info_t    *atlas   = handler->font_atlas;

    u32     tile_index          = ' ';
    f32     norm_font_width     = norm_font_size;
    f32     norm_font_height    = norm_font_size;
    vec2f_t x_offset            = {0};

    glquad_t vertices[KB];
    stack_t stack = stack_static_array_init(vertices, KB);

    for (int i = 0; text[i] != '\0'; i++) 
    {
        tile_index = text[i] - ' ';

        quadf_t quad = quadf_init(
                vec2f_add(position, x_offset), 
                norm_font_width,
                norm_font_height
        );

        quadf_t tex_coord = {
            atlas[tile_index].texture_coord[0].cmp[X], atlas[tile_index].texture_coord[0].cmp[Y],
            atlas[tile_index].texture_coord[1].cmp[X], atlas[tile_index].texture_coord[1].cmp[Y],
            atlas[tile_index].texture_coord[2].cmp[X], atlas[tile_index].texture_coord[2].cmp[Y],
            atlas[tile_index].texture_coord[3].cmp[X], atlas[tile_index].texture_coord[3].cmp[Y],
        };

        glquad_t textquad = glquad_init(quad, vec3f(0.0f), tex_coord, 0);

        stack_push_by_value(&stack, textquad);

        x_offset = vec2f_add(
                x_offset, 
                (vec2f_t) {norm_font_width, 0.0f}
        );
    }

    glrenderer2d_t textrenderer    = glrenderer2d_init(&handler->shader, &handler->texture);
    glbatch_t batch                = glbatch_init(vertices, glquad_t );

    glrenderer2d_draw_from_batch(&textrenderer, &batch);

    glbatch_destroy(&batch);
    glrenderer2d_destroy(&textrenderer);
}

void gl_ascii_font_destroy(gl_ascii_font_t *self)
{
    if (self == NULL) eprint("argument is null");

    gl_shader_destroy(&self->shader);
    gl_texture2d_destroy(&self->texture);
}

#endif //__MY__FONT__H__
