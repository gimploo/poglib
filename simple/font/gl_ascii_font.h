#ifndef __MY__FONT__H__
#define __MY__FONT__H__

/*======================================================
 // OpenGL font rendering with ascii style spritesheets
======================================================*/

#include "../gl_renderer.h"

#define ASCII_TILE_WIDTH_COUNT     16
#define ASCII_TILE_HEIGHT_COUNT    6
#define MAX_ASCII_TILES_COUNT    (ASCII_TILE_HEIGHT_COUNT * ASCII_TILE_WIDTH_COUNT)

typedef struct character_info_t  {

    char                character;
    vec2f               texture_coord[4];

    f32                 font_width;
    f32                 font_height;

} character_info_t;

typedef struct gl_ascii_font_handler_t {
    
    character_info_t    font_atlas[MAX_ASCII_TILES_COUNT];
    gl_shader_t         shader;

    gl_texture2d_t      texture;
    u32                 sprite_count_width;
    u32                 sprite_count_height;


} gl_ascii_font_handler_t;


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
    "   gl_Position = vec4(aPos, 0.0f);\n"
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
    "}";

/*----------------------------------------------------------------
 // Declarations
----------------------------------------------------------------*/

gl_ascii_font_handler_t gl_ascii_font_init(const char *file_path, u32 tile_count_width, u32 tile_count_height);
void                    gl_ascii_font_render_text(gl_ascii_font_handler_t *handler, const char *text, vec2f position);
void                    gl_ascii_font_destroy(gl_ascii_font_handler_t *);

/*----------------------------------------------------------------
 // Implementation
----------------------------------------------------------------*/


//NOTE: this function only handles a ascii style spritesheets
gl_ascii_font_handler_t gl_ascii_font_init(const char *file_path, u32 tile_count_width, u32 tile_count_height)
{
    if (file_path == NULL) eprint("file_path argument is null");


    gl_ascii_font_handler_t output;
    output.shader = shader_load_code(ascii_font_vs, ascii_font_fs);
    output.texture = texture_init(file_path);

    const f32 tiles_U = tile_count_width;
    const f32 tiles_V = tile_count_height;

    u32 tile_index = 0;
    char character = ' ';
    const u32 font_width = output.texture.width / tiles_U;
    const u32 font_height = output.texture.height / tiles_V;

    for (u32 u = 0; u < tiles_U; u++)
    {
        for (u32 v = 0; v < tiles_V; v++)
        {
            f32 left_U      = u        / tiles_U;
            f32 right_U     = (u+1.0f) / tiles_U; 
            f32 top_V       = (tiles_V - v)        / tiles_V;
            f32 bottom_V    = (tiles_V - v - 1.0f) / tiles_V;

            vec2f tex_coord[4] = {
                {left_U, top_V},
                {right_U, top_V},
                {right_U, bottom_V},
                {left_U, bottom_V},
            };

            output.font_atlas[tile_index].character = (character + tile_index);
            output.font_atlas[tile_index].font_height = font_height;
            output.font_atlas[tile_index].font_width =  font_width;
            matrix4f_copy(
                    output.font_atlas[tile_index].texture_coord,
                    tex_coord
            );

            /*
            printf("Character = %c \n", output.font_atlas[tile_index].character);
            printf("font height = %f\n", output.font_atlas[tile_index].font_height);
            printf("font height = %f\n", output.font_atlas[tile_index].font_width);
            printf("TexCoord: " VEC2F_FMT "\n",VEC2F_ARG(tex_coord[0])); 
            printf("TexCoord: " VEC2F_FMT "\n",VEC2F_ARG(tex_coord[1])); 
            printf("TexCoord: " VEC2F_FMT "\n",VEC2F_ARG(tex_coord[2])); 
            printf("TexCoord: " VEC2F_FMT "\n",VEC2F_ARG(tex_coord[3])); 
            */


            ++tile_index;

        }
    }

    return output;
    
}

void gl_ascii_font_render_text(gl_ascii_font_handler_t *handler, const char *text, vec2f position)
{
    if (handler == NULL)    eprint("handler argument is null");
    if (text == NULL)       eprint("text argument is null");

    unsigned int indices[] = {          
        0, 1, 2, // first triangle  
        2, 3, 0  // second triangle 
    };                                  

    u32 tile_index = ' ';

    character_info_t *atlas = handler->font_atlas;

    gl_ascii_font_handler_t *output = handler; // NOTE: delete this line later

    f32 font_width = handler->font_atlas[0].font_width;
    f32 font_height = handler->font_atlas[0].font_height;
    
    for (int i = 0; text[i] != '\0'; i++) 
    {
        tile_index = (text[i] - ' ') + i;

        printf("Tile index %i\n", tile_index);
        printf("Character = %c \n", output->font_atlas[tile_index].character);
        printf("font height = %f\n", output->font_atlas[tile_index].font_height);
        printf("font height = %f\n", output->font_atlas[tile_index].font_width);
        printf("TexCoord: " VEC2F_FMT "\n",VEC2F_ARG(output->font_atlas[tile_index].texture_coord[0])); 
        printf("TexCoord: " VEC2F_FMT "\n",VEC2F_ARG(output->font_atlas[tile_index].texture_coord[1])); 
        printf("TexCoord: " VEC2F_FMT "\n",VEC2F_ARG(output->font_atlas[tile_index].texture_coord[2])); 
        printf("TexCoord: " VEC2F_FMT "\n",VEC2F_ARG(output->font_atlas[tile_index].texture_coord[3])); 

        

        float vertices[] = {                           

            position.cmp[X] , position.cmp[Y], 0.0f,                                atlas[tile_index].texture_coord[0].cmp[X], atlas[tile_index].texture_coord[0].cmp[Y],
            (position.cmp[X] + font_width), position.cmp[Y], 0.0f,                  atlas[tile_index].texture_coord[1].cmp[X], atlas[tile_index].texture_coord[1].cmp[Y],
            (position.cmp[X] + font_width), (position.cmp[Y] - font_height), 0.0f,  atlas[tile_index].texture_coord[2].cmp[X], atlas[tile_index].texture_coord[2].cmp[Y],
            position.cmp[X], (position.cmp[Y] - font_height), 0.0f,                 atlas[tile_index].texture_coord[3].cmp[X], atlas[tile_index].texture_coord[3].cmp[Y],

        };                                             

        vao_t vao = vao_init(1);
        vbo_t vbo = vbo_init(vertices, sizeof(vertices)); 
        vao_push(&vao, &vbo);

        ebo_t ebo = ebo_init(&vbo, indices, 6);

        vao_set_attributes(&vao, 0, 3, GL_FLOAT, false, 5 * sizeof(float), 0);      
        vao_set_attributes(&vao, 0, 2, GL_FLOAT, false, 5 * sizeof(float), 3 * sizeof(float));

        texture_bind(&handler->texture, 0);
        shader_bind(&handler->shader);

        vao_draw(&vao); 

        ebo_destroy(&ebo);
        vbo_destroy(&vbo);
        vao_destroy(&vao);

    }
}

void gl_ascii_font_destroy(gl_ascii_font_handler_t *self)
{
    if (self == NULL) eprint("argument is null");

    shader_destroy(&self->shader);
}

#endif //__MY__FONT__H__
