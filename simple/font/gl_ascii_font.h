#ifndef __MY__FONT__H__
#define __MY__FONT__H__

/*======================================================
 // OpenGL font rendering with ascii style spritesheets
======================================================*/

#include "../../math/la.h"
#include "../../math/shapes.h"
#include "../gl_renderer.h"

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
    
    character_info_t    font_atlas[MAX_ASCII_TILES_COUNT];
    gl_shader_t         shader;

    gl_texture2d_t      texture;

    u32                 font_count_width;
    u32                 font_count_height;


} gl_ascii_font_t;


const char *ascii_font_vs = 
    "#version 460 core\n"
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

const char *ascii_font_fs = 
    "#version 460 core\n"
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
        //"FragColor = (1.0f, 0.0f, 0.0f, 1.0f);\n"
    "}";


/*----------------------------------------------------------------
 // Declarations
----------------------------------------------------------------*/

gl_ascii_font_t gl_ascii_font_init(const char *file_path, u32 tile_count_width, u32 tile_count_height);
void            gl_ascii_font_render_text(const gl_ascii_font_t *handler, const char *text, const vec2f_t position, const f32 norm_font_size); 
void            gl_ascii_font_destroy(gl_ascii_font_t *);

/*----------------------------------------------------------------
 // Implementation
----------------------------------------------------------------*/



//NOTE: this function only handles a ascii style spritesheets
gl_ascii_font_t gl_ascii_font_init(const char *file_path, u32 tile_count_width, u32 tile_count_height)
{
    if (file_path == NULL) eprint("file_path argument is null");

    gl_ascii_font_t output = {
        .shader     = shader_load_code(ascii_font_vs, ascii_font_fs),
        .texture    = texture_init(file_path),
    };

    const u32 font_width    = output.texture.width / tile_count_width;
    const u32 font_height   = output.texture.height / tile_count_height;

    const f32 norm_sprite_height  = norm(font_height, 0.0f, output.texture.height); 
    const f32 norm_sprite_width   = norm(font_width, 0.0f, output.texture.width);

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
        const gl_ascii_font_t   *handler, 
        const char              *text, 
        const vec2f_t             position, 
        const f32               norm_font_size)
{
    if (handler == NULL)    eprint("handler argument is null");
    if (text == NULL)       eprint("text argument is null");


    const character_info_t    *atlas   = handler->font_atlas;

    quadf_t quad                = {0};
    u32     tile_index          = ' ';
    f32     norm_font_width     = norm_font_size;
    f32     norm_font_height    = norm_font_size;

    unsigned int indices[] = {          
        0, 1, 2, // first triangle  
        2, 3, 0  // second triangle 
    };                                  

    vao_t vao = vao_init(1);
    vec2f_t x_offset = {0};

    for (int i = 0; text[i] != '\0'; i++) 
    {
        tile_index = text[i] - ' ';

        quad = quadf_init(
                vec2f_add(position, x_offset), 
                norm_font_width,
                norm_font_height
        );


#if 0
        gl_ascii_font_t *output = handler; // NOTE: delete this line later
        printf("Tile index %i\n", tile_index);
        printf("Character = %c \n", output->font_atlas[tile_index].character);
        printf("norm font width = %f\n", output->font_atlas[tile_index].norm_font_width);
        printf("norm font height = %f\n", output->font_atlas[tile_index].norm_font_height);
        printf("font height = %f\n", output->font_atlas[tile_index].font_height);
        printf("font width = %f\n", output->font_atlas[tile_index].font_width);
        printf("TexCoord: " VEC2F_FMT "\n",VEC2F_ARG(output->font_atlas[tile_index].texture_coord[0])); 
        printf("TexCoord: " VEC2F_FMT "\n",VEC2F_ARG(output->font_atlas[tile_index].texture_coord[1])); 
        printf("TexCoord: " VEC2F_FMT "\n",VEC2F_ARG(output->font_atlas[tile_index].texture_coord[2])); 
        printf("TexCoord: " VEC2F_FMT "\n",VEC2F_ARG(output->font_atlas[tile_index].texture_coord[3])); 
#endif 

        

        float vertices[] = {                           

            quad.cmp[0].cmp[X], quad.cmp[0].cmp[Y], 0.0f,
            quad.cmp[1].cmp[X], quad.cmp[1].cmp[Y], 0.0f,
            quad.cmp[2].cmp[X], quad.cmp[2].cmp[Y], 0.0f,
            quad.cmp[3].cmp[X], quad.cmp[3].cmp[Y], 0.0f,

            atlas[tile_index].texture_coord[0].cmp[X], atlas[tile_index].texture_coord[0].cmp[Y],
            atlas[tile_index].texture_coord[1].cmp[X], atlas[tile_index].texture_coord[1].cmp[Y],
            atlas[tile_index].texture_coord[2].cmp[X], atlas[tile_index].texture_coord[2].cmp[Y],
            atlas[tile_index].texture_coord[3].cmp[X], atlas[tile_index].texture_coord[3].cmp[Y],

        };                                             

#if 0
        printf("Position: " VEC2F_FMT"\n", vertices[0], vertices[1]);
        printf("--------- " VEC2F_FMT"\n", vertices[3], vertices[4]); 
        printf("          " VEC2F_FMT"\n", vertices[6], vertices[7]); 
        printf("          " VEC2F_FMT"\n", vertices[9], vertices[10]); 
#endif


        vbo_t vbo = vbo_init(vertices, sizeof(vertices)); 
        ebo_t ebo = ebo_init(&vbo, indices, 6);

        vao_push(&vao, &vbo);
            vao_set_attributes(&vao, 0, 3, GL_FLOAT, false, 3 * sizeof(float), 0);      
            vao_set_attributes(&vao, 0, 2, GL_FLOAT, false, 2 * sizeof(float), 12 * sizeof(float));
            texture_bind(&handler->texture, 0);
            shader_bind(&handler->shader);
            vao_draw(&vao); 
        vao_pop(&vao);

        ebo_destroy(&ebo);
        vbo_destroy(&vbo);

        x_offset = vec2f_add(
                x_offset, 
                (vec2f_t) {norm_font_width, 0.0f}
        );


    }

    vao_destroy(&vao);
}

void gl_ascii_font_destroy(gl_ascii_font_t *self)
{
    if (self == NULL) eprint("argument is null");

    shader_destroy(&self->shader);
}

#endif //__MY__FONT__H__
