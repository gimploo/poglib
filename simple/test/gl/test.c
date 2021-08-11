#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>

#include "../gl_renderer.h"
#include "../font/gl_ascii_font.h"

#include "../window.h"

#define uint32 u_int32_t 

static gl_shader_t shader;
static gl_texture2d_t texture;
static gl_texture2d_t *texture_ref = NULL;

static gl_ascii_font_handler_t font;

static vbo_t VBO;
static ebo_t EBO;
static vao_t VAO;

void print_VBO(void *arg)
{
    GLuint id; 
    u32  attribute_count;

    vbo_t *v = (vbo_t *)arg;

    printf("VBO id = %i", v->id);
    printf(" |  attribute_index = %i", v->attribute_index);
    printf(" |  indices_count = %i", v->indices_count);
}

void gl_setup_rectangle(void)
{
    shader = shader_init("./wood.vs", "./wood.fs");

    texture = texture_init("./ascii_font.png");

    font = gl_ascii_font_init("./glyph_atlas.png", 16, 6);
    texture_ref = &font.texture;

    /*float vertices[] = {    */
              /*// positions          // colors           // texture coords*/
         /*0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f, 0.437500,0.166667, // 1.0f, 1.0f, // top right*/
         /*0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f, 0.437500,0.000000, // 1.0f, 0.0f, // bottom right*/
        /*-0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f, 0.000000,0.0f, // 0.0f, 0.0f, // bottom left*/
        /*-0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f, 0.000000,0.166667  // 0.0f, 1.0f  // top left*/
    /*};                      */
    float vertices[] = {    
              // positions          // colors           // texture coords
         0.5f,  0.5f, 0.0f,   // top right
         0.5f, -0.5f, 0.0f,   // bottom right
        -0.5f, -0.5f, 0.0f,   // bottom left
        -0.5f,  0.5f, 0.0f,   // top left

            //color
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 0.0f,

        // textures
          1.0f, 1.0f,
          1.0f, 0.0f,
          0.0f, 0.0f,
          0.0f, 1.0f 
    };                      

    unsigned int indices[] = {  
            3, 1, 2, // first triangle
            1, 3, 0  // second triangle
    };

    VAO = vao_init(1); // NOTE: ALWAYS Define vao before vbos


    VBO = vbo_init(vertices, sizeof(vertices));
    vao_push(&VAO, &VBO); // this pushes a VBO to the vao stack

    EBO = ebo_init(&VBO, indices, 6); // NOTE: Always define ebos after vbo

    vao_set_attributes(&VAO, 0, 3, GL_FLOAT, false, 3 * sizeof(float), 0);
    vao_set_attributes(&VAO, 0, 3, GL_FLOAT, false, 3 * sizeof(float), 12 * sizeof(float));
    vao_set_attributes(&VAO, 0, 2, GL_FLOAT, false, 2 * sizeof(float), 24 * sizeof(float));

    /*vao_set_attributes(&VAO, 0, 3, GL_FLOAT, false, 8 * sizeof(float), 0);*/
    /*vao_set_attributes(&VAO, 0, 3, GL_FLOAT, false, 8 * sizeof(float), 3 * sizeof(float));*/
    /*vao_set_attributes(&VAO, 0, 2, GL_FLOAT, false, 8 * sizeof(float), 6 * sizeof(float));*/


}


void gl_draw_rectangle(void * arg)
{
    (void)arg;

    /*texture_bind(&texture, 0);*/
    texture_bind(texture_ref, 0);
    shader_bind(&shader);

    vao_draw(&VAO);
        
}

int main(void)
{
    uint32 FLAGS = SDL_INIT_VIDEO;
    Window window = window_init(1080, 920, FLAGS);
    gl_setup_rectangle();
    while (window.is_open)
    {
        window_process_user_input(&window);
        window_render(&window, gl_draw_rectangle, NULL); 
    }

    ebo_destroy(&EBO);
    vao_destroy(&VAO);

    shader_destroy(&shader);
    texture_destroy(&texture);

    window_destroy(&window);

    return 0;
}
