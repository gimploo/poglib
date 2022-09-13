#pragma once
#if defined(WINDOW_SDL) 
#error SDL already included (can only run either one)
#endif
#include <poglib/basic.h>
#include <poglib/math.h>
#include <GL/glew.h>
#include "../gfx/gl/common.h"
#include <GLFW/glfw3.h>

#define DEFAULT_BACKGROUND_COLOR    (vec4f_t ){ 0.0f, 0.0f, 0.8f, 1.0f}
#define GLFW_NUM_SCANCODES          (GLFW_KEY_LAST + 1)

typedef struct window_t {

    bool                is_open;
    const char          *title;
    u64                 width; 
    u64                 height;
    vec4f_t             background_color;

    struct {

        vec2f_t         norm_position;
        vec2f_t         position;

    } mouse;

    struct {

        bool     pressed[GLFW_NUM_SCANCODES]; 
        bool     just_pressed[GLFW_NUM_SCANCODES]; 
        bool     is_held[GLFW_NUM_SCANCODES];

    } input;

    GLFWwindow          *__glfw_window;      // initializes the window 

} window_t ;

global window_t     *global_window = NULL;

window_t *          window_init(const char *title, const u64 width, const u64 height, const u32 GLFWflags);
#define             window_get_current_active_window() global_window

void                window_update_user_input(window_t *window);
void                window_set_background(window_t *window, vec4f_t color);
void                window_update_title(window_t *window, const char *title);

bool                window_keyboard_is_key_just_pressed(window_t *window, const u32 key);
bool                window_keyboard_is_key_held(window_t *window, const u32 key);
bool                window_keyboard_is_key_pressed(window_t *window, const u32 key);

bool                window_mouse_button_just_pressed(window_t *window, const u32 button);
bool                window_mouse_button_is_pressed(window_t *window, const u32  button);
bool                window_mouse_button_is_held(window_t *window, const u32 button);

#define             window_mouse_get_norm_position(PWINDOW)                     (PWINDOW)->mouse.norm_position
#define             window_mouse_get_position(PWINDOW)                          (PWINDOW)->mouse.position

#define             window_gl_render_begin(PWINDOW)                             __impl_window_gl_render_begin(PWINDOW)
#define             window_gl_render_end(PWINDOW)                               glfwSwapBuffers((PWINDOW)->__glfw_window)

void                window_destroy(void);

#ifndef IGNORE_GLFW_WINDOW_IMPLEMENTATION

void __glfw_error_callback(int error, const char *description) 
{  
    eprint("%s", description);
}

void __glfw_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) 
{
    window_t *win = window_get_current_active_window();

    switch(action)
    {
        case GLFW_PRESS: 
            /*printf("%i is pressed\n", key);*/
            if (win->input.pressed[key]) {
                win->input.is_held[key] = true;
                win->input.just_pressed[key] = false;
            } else 
                win->input.pressed[key] = true;
        break;

        case GLFW_RELEASE: 
            /*printf("%i is released\n", key);*/
            win->input.pressed[key] = false;
            win->input.is_held[key] = false;
            win->input.just_pressed[key] = false;
        break;

        case GLFW_REPEAT:
            /*printf("%i is pressed repeatedly\n", key);*/
            win->input.pressed[key] = true;
            win->input.is_held[key] = true;
            win->input.just_pressed[key] = false;
        break;
    }

    {
        // Mouse position
        
        f64 xpos = 0.0f , ypos = 0.0f;
        glfwGetCursorPos(window, &xpos, &ypos);
        const f32 nx = -1.0 + 2.0 *  (f32) xpos / win->width;
        const f32 ny = (1.0 - 2.0 * (f32) ypos / win->height);
        win->mouse.position         = (vec2f_t ){ (f32)xpos, (f32)ypos };
        win->mouse.norm_position    = (vec2f_t ){ nx, ny };
    }

}

void __glfw_mouse_button_callback(GLFWwindow* window, int key, int action, int mods)
{
    window_t *win = window_get_current_active_window();

    switch(action)
    {
        case GLFW_PRESS: 
            if (win->input.pressed[key]) {
                win->input.is_held[key] = true;
                win->input.just_pressed[key] = false;
            } else 
                win->input.pressed[key] = true;
        break;

        case GLFW_RELEASE: 
            win->input.pressed[key] = false;
            win->input.is_held[key] = false;
            win->input.just_pressed[key] = false;
        break;

        case GLFW_REPEAT:
            win->input.pressed[key] = true;
            win->input.is_held[key] = true;
            win->input.just_pressed[key] = false;
        break;

        default: eprint("unknown action");
    }
}

void __glfw_window_close_callback(GLFWwindow *glfwin)
{
    window_t *win = window_get_current_active_window();
    win->is_open = false;
}

void __glfw_framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    GL_CHECK(glViewport(0, 0, width, height));
    window_t *win = window_get_current_active_window();
    win->width = width;
    win->height = height;
}

window_t * window_init(const char *title, const u64 width, const u64 height, const u32 GLFWflags)
{
    glfwSetErrorCallback(__glfw_error_callback);

    if (!glfwInit())
        eprint("glfw failed to initialize");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    GLFWwindow * window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!window) {
        glfwTerminate();
        eprint("glfw failed to create window");
    }
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) 
        eprint("GLEW Error: %s\n", glewGetErrorString(glewError));

    printf("[OUTPUT] Using OpenGL renderer version (%s)\n", glGetString(GL_VERSION));

    glfwSetFramebufferSizeCallback(window, __glfw_framebuffer_size_callback);
    glfwSetKeyCallback(window, __glfw_key_callback);
    glfwSetMouseButtonCallback(window, __glfw_mouse_button_callback);
    if (glfwRawMouseMotionSupported())
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    glfwSetWindowCloseCallback(window, __glfw_window_close_callback);

    window_t win = {0};
    win.is_open = true;
    win.background_color = DEFAULT_BACKGROUND_COLOR;
    win.__glfw_window = window;

    global_window = (window_t *)calloc(1, sizeof(window_t ));
    assert(global_window);

    *global_window = win;

    return global_window;
}

void window_destroy(void)
{
    if (!global_window) eprint("global window is null");

    window_t *window = global_window;

    glfwDestroyWindow(window->__glfw_window);
    glfwTerminate();

    window->__glfw_window = NULL;
    window = NULL;

    free(global_window);
    window = NULL;
}

#define __impl_window_gl_render_begin(PWINDOW) do {\
\
    GL_CHECK(glEnable(GL_BLEND));\
    GL_CHECK(glEnable(GL_DEPTH_TEST));\
    GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));\
    GL_CHECK(glClearColor(\
            (PWINDOW)->background_color.r,\
            (PWINDOW)->background_color.g,\
            (PWINDOW)->background_color.b,\
            (PWINDOW)->background_color.a\
    ));\
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));\
\
} while(0)


void window_update_user_input(window_t *window)
{
    glfwPollEvents();
}

void window_set_background(window_t *window, vec4f_t color)
{
    window->background_color = color;
}

void window_update_title(window_t *window, const char *title)
{
    window->title = title;
    glfwSetWindowTitle(window->__glfw_window, title);
}

bool window_keyboard_is_key_just_pressed(window_t *window, const u32 key)
{
    bool output = window->input.just_pressed[key];
    window->input.just_pressed[key] = false;
    return output;
}

bool window_keyboard_is_key_held(window_t *window, const u32 key)
{
    bool output = window->input.is_held[(key)];
    window->input.is_held[(key)] = false;
    return output;
}

bool window_keyboard_is_key_pressed(window_t *window, const u32 key)
{
    bool output = window->input.is_held[(key)] 
                    || window->input.just_pressed[(key)];

    window->input.just_pressed[(key)] = false;
    window->input.is_held[(key)] = false;

    return output;
}

bool window_mouse_button_just_pressed(window_t *window, const u32 button)
{
    return window->input.just_pressed[button];
}

bool window_mouse_button_is_pressed(window_t *window, const u32 button)
{
    return window->input.pressed[button];
}

bool window_mouse_button_is_held(window_t *window, const u32 button)
{
    return window->input.is_held[button];
}

bool window_mouse_button_is_released(window_t *window, const u32 button)
{
    return !window->input.pressed[button];
}

#endif 

