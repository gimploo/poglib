#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>

#include "../simple_opengl.h"
#include "../simple_window.h"

void loop(void *arg)
{
    gl_object_t *obj = arg;
    gl_object_draw(obj, GL_TRIANGLES);
}

int main(void)
{
    SimpleWindow window = window_init(1080, 920, SDL_INIT_VIDEO);

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

    

    Shader shader = shader_init("./wood.vs", "./wood.fs");

    Texture2D texture = texture_init("./wall.jpg");
    
    gl_object_t obj = gl_object_init(vertices, sizeof(vertices), indices, 6, &shader, &texture);

    vao_set_attributes(&obj.vao, 3, GL_FLOAT, false, 8 * sizeof(float), 0);
    vao_set_attributes(&obj.vao, 3, GL_FLOAT, false, 8 * sizeof(float), 3 * sizeof(float));
    vao_set_attributes(&obj.vao, 2, GL_FLOAT, false, 8 * sizeof(float), 6 * sizeof(float));


    while (window.is_open)
    {
        window_process_user_input(&window);
        window_render(&window, loop, &obj); 
    }

    gl_object_destroy(&obj);

    window_destroy(&window);
    return 0;
}
