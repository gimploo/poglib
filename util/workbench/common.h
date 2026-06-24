#pragma once
#include <poglib/basic.h>
#include <poglib/gui.h>
#include <poglib/util/glcamera.h>
#include <poglib/input/commandqueue.h>
#include <poglib/font/glfreetypefont.h>
#include "./workbench-debug-renderer.h"
#include "../../clay_renderer_poglib.h"

typedef enum workbench_action_type {
    WORKBENCH_ACTION_TYPE_MOUSE_LEFT_CLICK_DRAG         = 0,
    WORKBENCH_ACTION_TYPE_MOUSE_LEFT_CLICK_JUST_CLICKED = 1,
    WORKBENCH_ACTION_TYPE_CAMERA_ZOOM_IN                = 2,
    WORKBENCH_ACTION_TYPE_CAMERA_ZOOM_OUT               = 3,
    WORKBENCH_ACTION_TYPE_COUNT
} workbench_action_type;

typedef struct {

    bool is_active;
    bool enable_collider;
    bool disable_grid;

    struct {
        bool wireframe_mode;
    } render_config;

    struct {
        gui_t handle;
        bool enable;
    } gui;

    struct {
        u32 atlas_id;
        u32 shader_id;
    } primitives;

    workbench_debug_renderer_t debug_renderer;

    glshader_t shader;
    vec3f_t player_camera_position;

    commandregistry_t commandregistry;

    u32 selected_entity_id;

    struct {
        u32 entity_id;
        glcamera_t *handle;
    } world_camera;

    struct {
        clay_poglib_renderer_t renderer;
        glfreetypefont_t font;
        bool initialized;
        char text_buf[2048];
        i32 text_offset;
    } clay;

} workbench_t;

global workbench_t *global_workbench = NULL;
