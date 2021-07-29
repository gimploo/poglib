#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>

#include "../simple_opengl.h"
#include "../simple_window.h"

#define uint32 u_int32_t 

static Shader shader;
static Texture2D texture;

static gl_vbo_t vbo;
static gl_ebo_t ebo;
static gl_vao_t vao;

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

    vao = vao_init();

    /*GLuint VBO;*/
    /*glGenBuffers(1, &VBO); */
    /*glBindBuffer(GL_ARRAY_BUFFER, VBO);*/
    /*glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);*/

    //               | 
    //               | 
    //               V
    
    vbo = vbo_init(vertices, sizeof(vertices));


    /*glGenBuffers(1, &EBO);*/
    /*glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);*/
    /*glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);*/

    //               | 
    //               | 
    //               V

    ebo = ebo_init(indices, 6);



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

    vao_set_attributes(&vao, 3, GL_FLOAT, false, 8 * sizeof(float), 0);
    vao_set_attributes(&vao, 3, GL_FLOAT, false, 8 * sizeof(float), 3 * sizeof(float));
    vao_set_attributes(&vao, 2, GL_FLOAT, false, 8 * sizeof(float), 6 * sizeof(float));

    vbo_unbind();
    vao_unbind();

}

void gl_draw_rectangle(void)
{
    texture_bind(&texture, 0);
    shader_bind(&shader);

    vao_bind(&vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

int main(void)
{
    uint32 FLAGS = SDL_INIT_VIDEO;
    SimpleWindow window = window_init(1080, 920, FLAGS);
    gl_setup_rectangle();
    while (window.is_open)
    {
        window_process_user_input(&window);
        window_render(&window, gl_draw_rectangle); 
    }

    ebo_destroy(&ebo);
    vbo_destroy(&vbo);
    vao_destroy(&vao);

    shader_destroy(&shader);
    texture_destroy(&texture);

    window_destroy(&window);
    return 0;
}
