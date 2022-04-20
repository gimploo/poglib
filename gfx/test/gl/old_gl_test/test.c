#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>

#include "../../../gl_renderer.h"
#include "../../../window.h"

#define uint32 u_int32_t 

static gl_shader_t shader;
static gl_texture2d_t texture;

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
    shader = gl_shader_from_file_init("./wood.vs", "./wood.fs");

    texture = texture_init("./wall.jpg");

    gl_vertex_t vertices[4] = {    
              // positions          // colors           // texture coords
        -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f,  // top left
         0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
         0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
    };                      

    u32 indices[] = {  
            0, 1, 2, // first triangle
            2, 3, 0  // second triangle
    };


    /*glGenVertexArrays(1, &VAO); */
    /*glBindVertexArray(VAO); */

    //               | 
    //               | 
    //               V

    VAO = vao_init(1);

    /*GLuint VBO;*/
    /*glGenBuffers(1, &VBO); */
    /*glBindBuffer(GL_ARRAY_BUFFER, VBO);*/
    /*glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);*/

    //               | 
    //               | 
    //               V
    
    VBO = vbo_init(vertices, sizeof(vertices));

    vao_push(&VAO, &VBO); // this pushes a VBO to the vao stack



    /*glGenBuffers(1, &EBO);*/
    /*glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);*/
    /*glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);*/

    //               | 
    //               | 
    //               V

    EBO = ebo_init(&VBO, indices, 6);


    // position attribute
    /*glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);*/
    /*glEnableVertexAttribArray(0);*/
    /*// color attribute*/
    /*glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));*/
    /*glEnableVertexAttribArray(1);*/
    /*// texture coord attribute*/
    /*glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));*/
    /*glEnableVertexAttribArray(2);*/
    //               | 
    //               | 
    //               V

    //                 | vao |cmp| type   | norm | offset           |  start ptr
    vao_set_attributes(&VAO, 0, 3, GL_FLOAT, false, sizeof(gl_vertex_t), offsetof(gl_vertex_t, position));
    vao_set_attributes(&VAO, 0, 3, GL_FLOAT, false, sizeof(gl_vertex_t), offsetof(gl_vertex_t, color));
    vao_set_attributes(&VAO, 0, 2, GL_FLOAT, false, sizeof(gl_vertex_t), offsetof(gl_vertex_t, texture_coord));

    
    

    vao_unbind();

}

void gl_draw_rectangle(void * arg)
{
    (void)arg;

    gl_shader_bind(&shader);
    texture_bind(&texture, 0);

    /*stack_print(&VAO.vbos, print_VBO);*/

    vao_draw(&VAO);
        
}

int main(void)
{
    unsigned int FLAGS = SDL_INIT_VIDEO;
    window_t window = window_init(1080, 920, FLAGS);
    gl_setup_rectangle();
    while (window.is_open)
    {
        if (window.keyboard_handler.key == SDLK_q)
            window.is_open = false;
        /*window_render(&window, gl_draw_rectangle, NULL); */
        window_gl_render_begin(&window);
            gl_draw_rectangle(NULL);
        window_gl_render_end(&window);
    }

    ebo_destroy(&EBO);
    
    vao_destroy(&VAO);

    gl_shader_destroy(&shader);
    texture_destroy(&texture);

    window_destroy(&window);

    return 0;
}
