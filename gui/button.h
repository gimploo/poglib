#ifndef __MY__BUTTON__H__
#define __MY__BUTTON__H__

#include "../basic.h"
#include "../math/la.h"
#include "../simple/gl_renderer.h"
#include "../simple/window.h"

#include <unistd.h>


typedef struct button_t button_t;
struct button_t {

    const char  *label;
    bool        is_hot;
    bool        is_active;

    vec2f       position; 
    f32         width, height;
    vec4f       color;

    vao_t       vao;
    vbo_t       vbo;
    ebo_t       ebo;
    vec3f       vertices[4];
    Shader      shader;
};

const u32 RECT_INDICES[] = {          
    0, 1, 3, // first triangle  
    1, 2, 3  // second triangle 
};                                  

const char *vertex_source = 
"#version 430 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"    gl_Position = vec4(aPos, 1.0f);\n"
"}";

const char *fragment_source = 
"#version 430 core\n"
"out vec4 FragColor;\n"
"uniform vec4 u_color;\n"
"void main(void)\n"
"{\n"
"    FragColor = u_color;\n"
"}\n";

                             



static button_t     button_init(const char *label, vec4f color, vec2f position, f32 width, f32 height);
static void         button_draw(button_t *button);


static button_t button_init(const char *label, vec4f color, vec2f position, f32 width, f32 height)
{
    button_t button = {
        .label = label,
        .is_hot = false, 
        .is_active = false,
        .position = position,
        .width = width,
        .height = height,
        .color = color,
        .vao    = vao_init(1),
        .shader = shader_load_code(vertex_source, fragment_source),
    };


    vec3f vertices[4]; 

    // top left
    memcpy(&vertices[0], &button.position, sizeof(button.position)); 
    vertices[0].cmp[Z] = 0.0f; 
    
    // top right
    vertices[1] = (vec3f) {     
        button.position.cmp[X] + button.width,
        button.position.cmp[Y],
        0.0f
    }; 

    // bottom right
    vertices[2] = (vec3f) {     
        button.position.cmp[X] + button.width,
        button.position.cmp[Y] - button.height,
        0.0f
    }; 


    // bottom left
    vertices[3] = (vec3f) {     
        button.position.cmp[X],
        button.position.cmp[Y] - button.height,
        0.0f
    }; 

    memcpy(button.vertices, vertices, sizeof(vertices));

    for (int i = 0; i < 4; i++)
    {
        printf("(%f, %f, %f) \n", 
                button.vertices[i].cmp[X], 
                button.vertices[i].cmp[Y], 
                button.vertices[i].cmp[Z]); 
    }
    printf("\n");

    shader_set_farr(&button.shader, "u_color", (float *)&button.color);
    

    vao_bind(&button.vao);
    {
        button.vbo = vbo_init(button.vertices, sizeof(button.vertices));
        button.ebo = ebo_init(&button.vbo, RECT_INDICES, 6);

        GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0));
        GL_CHECK(glEnableVertexAttribArray(0));


    }
    vao_unbind();


    return button;
} 

static void button_draw(button_t * button)
{
    if (button == NULL) eprint("button argument is null");

    u32 indices[] = {          
        0, 1, 2, // first triangle  
        2, 3, 0  // second triangle 
    };                                  

    vao_bind(&button->vao);
    {
        shader_bind(&button->shader);
        GL_CHECK(glDrawElements(GL_TRIANGLES, button->vbo.indices_count, GL_UNSIGNED_INT, 0));
    }
    vao_unbind();
}

#endif //__MY__BUTTON__H__
