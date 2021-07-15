#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>

#include "../simple_window.h"

#define uint32 u_int32_t 


void gl_draw_triangle(void)
{
    Shader shader = simple_use_default_shader();

    float vertices[] = {    
    -0.5f, -0.5f, 0.0f, 
     0.5f, -0.5f, 0.0f, 
     0.0f,  0.5f, 0.0f  
    };                      

    GLuint VAO;
    glGenVertexArrays(1, &VAO); 
    glBindVertexArray(VAO); 

    GLuint VBO;
    glGenBuffers(1, &VBO); 
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    /*GLuint EBO;*/
    /*glGenBuffers(1, &EBO); glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);*/
    /*glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_size, indices, GL_STATIC_DRAW);*/

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0); 

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glBindVertexArray(0);


}

int main(void)
{
    uint32 FLAGS = SDL_INIT_VIDEO;
    SimpleWindow window = window_init(FLAGS);
    window_render_init(&window, gl_draw_triangle); 
    return 0;
}
