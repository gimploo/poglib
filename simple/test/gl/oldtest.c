#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>

#include "../gl_renderer.h"
#include "../window.h"

#define uint32 u_int32_t 

static Shader shader;
static Texture2D texture;

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

    texture = texture_init("./wall.jpg");

    float vertices[] = {    
              // positions          // colors           // texture coords
         0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
         0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
        -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left
    };                      

    unsigned int indices[] = {  
            0, 1, 3, // first triangle
            1, 2, 3  // second triangle
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
    vao_set_attributes(&VAO, 0, 3, GL_FLOAT, false, 8 * sizeof(float), 0);
    vao_set_attributes(&VAO, 0, 3, GL_FLOAT, false, 8 * sizeof(float), 3 * sizeof(float));
    vao_set_attributes(&VAO, 0, 2, GL_FLOAT, false, 8 * sizeof(float), 6 * sizeof(float));

    
    

    vao_unbind();

}

void gl_draw_rectangle(void * arg)
{
    (void)arg;

    texture_bind(&texture, 0);
    shader_bind(&shader);

    /*stack_print(&VAO.vbos, print_VBO);*/

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
