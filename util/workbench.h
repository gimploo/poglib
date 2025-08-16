#pragma once
#include <poglib/poggen.h>
#include <poglib/gui.h>
#include "./glcamera.h"
#include <poglib/gfx/glrenderer2d.h>
#include <poglib/gfx/glrenderer3d.h>
#include "./workbench/workbench-constants.h"
#include "./gllight.h"

typedef struct {

    vec3f_t start;
    vec3f_t end;

} line_t;

typedef struct {

    struct {
        bool wireframe_mode;
    } render_config;

    gui_t *gui;

    glshader_t shader;
    glcamera_t world_camera;
    vec3f_t player_camera_position;

    // Lines that draws - this clears up after every render
    list_t draw_lines;

    list_t lightsources;

    //FIXME: Need to get rid of this maybe with introducing arenas here will be able to
    struct {
        str_t vshader;
        str_t fshader;
    } filepaths;

} workbench_t;

#define WORKBENCH_CAMERA_DEFAULT_POSITION (vec3f_t){-125.f, 40.0f, 200.0f}
#define WORKBENCH_CAMERA_DEFAULT_ROTATION (vec2f_t){-0.3f, -0.9f}

workbench_t workbench_init(const application_t *app)
{
    str_t vshader = application_get_absolute_filepath(app, "lib/poglib/util/workbench/workbench-shader.vs");
    str_t fshader = application_get_absolute_filepath(app, "lib/poglib/util/workbench/workbench-shader.fs");

    workbench_t o = {
        .shader = glshader_from_file_init(
            vshader.data, 
            fshader.data),
        .world_camera = glcamera_perspective(
            WORKBENCH_CAMERA_DEFAULT_POSITION,
            WORKBENCH_CAMERA_DEFAULT_ROTATION 
        ),
        .player_camera_position = vec3f(0.f),
        .draw_lines = list_init(line_t),
        .lightsources = list_init(gllight_t *),
        .filepaths = {
            .vshader = vshader,
            .fshader = fshader
        },
        .render_config = {
            .wireframe_mode = false
        },
        .gui = gui_init()
    };

    return o;
}

void workbench_update_player_camera_position(workbench_t *self, const vec3f_t pos)
{
    self->player_camera_position = pos;
}

void workbench_pass_line(workbench_t *self, const line_t line) 
{
    list_append(&self->draw_lines, line);
}

void workbench_track_lightsource(workbench_t *self, const gllight_t *light)
{
    list_append_ptr(&self->lightsources, light);
}

void workbench_toggle_wireframe_mode(workbench_t *self)
{
    self->render_config.wireframe_mode = !self->render_config.wireframe_mode;
    gui_set_wireframe_mode(self->gui, self->render_config.wireframe_mode);
}

void __workbench_render_ui(workbench_t *self)
{
    GUI(self->gui) {
        UI_PANEL(panel, ((style_t){
            .color = COLOR_RED,
            .margin = {10.f, 10.f, 0.f, 0.f},
            .padding = {10, 10}, 
            .dim = {
                .width = 200,
                .height = 300
            },
            .layout = UI_LAYOUT_VERTICAL
        })) {
            UI_BUTTON(button1, 
                ((style_t){
                    .color = COLOR_WHITE, 
                }))
            if (button1->state.is_clicked) {
                printf("Button1 is clicked\n");
            }
            UI_LABEL(label1, 
                ((style_t){
                    .color = COLOR_BLUE, 
                }));
            UI_BUTTON(button3, 
                ((style_t ){
                    .color = COLOR_GREEN, 
                })) {
                if (button3->state.is_clicked) {
                    printf("Button3 is clicked\n");
                }
            };
            UI_CHECKBOX(
                checkbox, 
                ((style_t){0})
            );
            UI_SLIDER(
                slider,
                ((ui_config_t){.range={.min = 10, .max = 100}}),
                ((style_t){
                    .dim = {10, 10},
                })
            ) {
                //printf("value %f\n", slider->state.value);
            }
            UI_BUTTON(button4, 
                ((style_t){
                    .color = COLOR_GREEN, 
                }))
            if (button4->state.is_clicked) {
                printf("Button4 is clicked\n");
            }
        }
    }
}

void __workbench_render_lightsources(workbench_t *self)
{
    if (self->lightsources.len == 0) 
        return;

    const list_t *lights = &self->lightsources;
    list_iterator(lights, iter) {
        glrenderer3d_draw((glrendererconfig_t) {
            .calls = {
                .count = 1,
                .call = {
                    [0] = {
                        .is_wireframe = self->render_config.wireframe_mode,
                        .textures = {0},
                        .vtx = {
                            .data = (u8 *)DEFAULT_CUBE_VERTICES_8,
                            .size = sizeof(DEFAULT_CUBE_VERTICES_8)
                        },
                        .idx = {
                            .data = (u8 *)DEFAULT_CUBE_INDICES_8,
                            .nmemb = ARRAY_LEN(DEFAULT_CUBE_INDICES_8)
                        },
                        .attrs = {
                            .count = 1,
                            .attr = {
                                [0] = {
                                    .ncmp = 3,
                                    .interleaved = {0},
                                },
                            }
                        },
                        .shader_config = {
                            .shader = &self->shader,
                            .uniforms = {
                                .count = 4,
                                .uniform = {
                                    [0] = {
                                        .name = "view",
                                        .type = "matrix4f_t",
                                        .value = glcamera_getview(&self->world_camera)
                                    },
                                    [1] = {
                                        .name = "projection",
                                        .type = "matrix4f_t",
                                        .value = glms_perspective(
                                            radians(45), global_poggen->handle.app->window.aspect_ratio, 1.0f, 1000.0f)
                                    },
                                    [2] = {
                                        .name = "color",
                                        .type = "vec4f_t",
                                        .value.vec4 = ((gllight_t *)iter)->color
                                    },
                                    [3] = {
                                        .name = "transform",
                                        .type = "matrix4f_t",
                                        .value = glms_mat4_mul(
                                            glms_translate_make(((gllight_t *)iter)->position),
                                            glms_scale_make((vec3f_t){10.f, 10.f, 10.f})
                                        ),
                                    },
                                }
                            }
                        }
                    }
                }
            } 
        });

    }
}

void workbench_render(workbench_t *self)
{
    if(self->draw_lines.len == 0) eprint("No lines to render!");

    glrenderer3d_draw((glrendererconfig_t){
        .calls = {
            .count = 3,
            .call = {
                //camera
                [0] = {
                    .is_wireframe = true || self->render_config.wireframe_mode,
                    .textures = {0},
                    .idx = {
                        .data = (u8 *)&CAMERA_INDICES,
                        .nmemb = ARRAY_LEN(CAMERA_INDICES)
                    },
                    .vtx = {
                        .data = (u8 *)&CAMERA_VERTICES,
                        .size = sizeof(CAMERA_VERTICES)
                    },
                    .textures = {0},
                    .attrs = {
                        .count = 1,
                        .attr = {
                            [0] = {
                                .ncmp = 3, 
                                .interleaved = {0}
                            }
                        },
                    },
                    .shader_config = {
                        .shader = &self->shader,
                        .uniforms = {
                            .count = 4,
                            .uniform = {
                                [0] = {
                                    .name = "view",
                                    .type = "matrix4f_t",
                                    .value = glcamera_getview(&self->world_camera)
                                },
                                [1] = {
                                    .name = "projection",
                                    .type = "matrix4f_t",
                                    .value = glms_perspective(
                                        radians(45), global_poggen->handle.app->window.aspect_ratio, 1.0f, 1000.0f)
                                },
                                [2] = {
                                    .name = "color",
                                    .type = "vec4f_t",
                                    .value.vec4 = COLOR_BLACK
                                },
                                [3] = {
                                    .name = "transform",
                                    .type = "matrix4f_t",
                                    .value = glms_mat4_mul(
                                        glms_translate_make(self->player_camera_position),
                                        glms_scale_make((vec3f_t){10.f, 10.f, 10.f})
                                    ),
                                },
                            }
                        },
                    }
                },
                //platform
                [1] = {
                    .is_wireframe = self->render_config.wireframe_mode,
                    .textures = {0},
                    .idx = {
                        .data = (u8 *)&DEFAULT_CUBE_INDICES_8,
                        .nmemb = ARRAY_LEN(DEFAULT_CUBE_INDICES_8)
                    },
                    .vtx = {
                        .data = (u8 *)&DEFAULT_CUBE_VERTICES_8,
                        .size = sizeof(DEFAULT_CUBE_VERTICES_8)
                    },
                    .textures = {0},
                    .attrs = {
                        .count = 1,
                        .attr = {
                            [0] = {
                                .ncmp = 3, 
                                .interleaved = {0}
                            }
                        },
                    },
                    .shader_config = {
                        .shader = &self->shader,
                        .uniforms = {
                            .count = 4,
                            .uniform = {
                                [0] = {
                                    .name = "view",
                                    .type = "matrix4f_t",
                                    .value = glcamera_getview(&self->world_camera)
                                },
                                [1] = {
                                    .name = "projection",
                                    .type = "matrix4f_t",
                                    .value = glms_perspective(
                                        radians(45), 
                                        global_poggen->handle.app->window.aspect_ratio, 
                                        1.0f, 1000.0f
                                    )
                                },
                                [2] = {
                                    .name = "transform",
                                    .type = "matrix4f_t",
                                    .value = glms_scale(MATRIX4F_IDENTITY, (vec3f_t){1000.0f, 1.0f, 1000.0f}),
                                },
                                [3] = {
                                    .name = "color",
                                    .type = "vec4f_t",
                                    .value.vec4 = COLOR_DARK_GRAY
                                },
                            }
                        } 
                    }
                },
                // lines
                [2] = {
                    .draw_mode = GL_LINE,
                    .is_wireframe = self->render_config.wireframe_mode,
                    .textures = {0},
                    .shader_config = {
                        .shader = &self->shader,
                        .uniforms = {
                            .count = 4,
                            .uniform = {
                                [0] = {
                                    .name = "view",
                                    .type = "matrix4f_t",
                                    .value = glcamera_getview(&self->world_camera)
                                },
                                [1] = {
                                    .name = "projection",
                                    .type = "matrix4f_t",
                                    .value = glms_perspective(
                                        radians(45), 
                                        global_poggen->handle.app->window.aspect_ratio, 
                                        1.0f, 1000.0f
                                    )
                                },
                                [2] = {
                                    .name = "transform",
                                    .type = "matrix4f_t",
                                    .value = MATRIX4F_IDENTITY
                                },
                                [3] = {
                                    .name = "color",
                                    .type = "vec4f_t",
                                    .value.vec4 = COLOR_BLUE
                                },
                            }
                        } 
                    },
                    .idx = {0},
                    .vtx = {
                        .data = list_get_buffer(&self->draw_lines),
                        .size = list_get_size(&self->draw_lines),
                    },
                    .attrs = {
                        .count = 1,
                        .attr = {
                            // position
                            [0] = {
                                .ncmp = 3,
                                .interleaved = {0},
                            },
                        }
                    }
                }
            },
        },
    });

    __workbench_render_lightsources(self);
    __workbench_render_ui(self);

    list_clear(&self->draw_lines);
}

void workbench_destroy(workbench_t *self)
{
    str_free(&self->filepaths.fshader);
    str_free(&self->filepaths.vshader);
    glshader_destroy(&self->shader);
    list_destroy(&self->draw_lines);
    list_destroy(&self->lightsources);
    gui_destroy(self->gui);
}



