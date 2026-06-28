#pragma once
#include <poglib/basic.h>
#include <poglib/gui.h>
#include <poglib/util/glcamera.h>
#include "./workbench-debug-renderer.h"

typedef enum gizmo_mode {
    GIZMO_MODE_TRANSLATE = 0,
    GIZMO_MODE_ROTATE,
    GIZMO_MODE_SCALE,
} gizmo_mode_t;

typedef enum workbench_action_type {
    WORKBENCH_ACTION_TYPE_MOUSE_LEFT_CLICK_DRAG             = 0,
    WORKBENCH_ACTION_TYPE_MOUSE_LEFT_CLICK_JUST_CLICKED     = 1,
    WORKBENCH_ACTION_TYPE_CAMERA_ZOOM_IN                    = 2,
    WORKBENCH_ACTION_TYPE_CAMERA_ZOOM_OUT                   = 3,
    WORKBENCH_ACTION_TYPE_EDITOR_CANCEL_EDIT                = 4,
    WORKBENCH_ACTION_TYPE_GIZMO_CYCLE_MODE                  = 5,
    WORKBENCH_ACTION_TYPE_TOGGLE_WIREFRAME                  = 6,
    WORKBENCH_ACTION_TYPE_MOUSE_ENTITY_SELECTION            = 7,
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
        u32 mesh_shader_id;
        u32 line_shader_id;
    } primitives;

    workbench_debug_renderer_t debug_renderer;

    glshader_t shader;
    vec3f_t player_camera_position;

    commandregistry_t commandregistry;

    struct {
        u32 entity_id;
        glcamera_t *handle;
    } world_camera;


    struct {

        //NOTE: these are used for entity selection
        u32 mouse_closest_to_entity_id;
        u32 current_selected_entity_id;
        u32 prev_selected_entity_id;
        u32 mouseclick_counter;

        gizmo_mode_t gizmo_mode;

        #define EDITOR_SELECTION_BOUNDS_MAX 256
        struct {
            u32     entity_id;
            vec3f_t half_extents;
        } selection_bounds[EDITOR_SELECTION_BOUNDS_MAX];
        u32 selection_bounds_count;
    } editor;

} workbench_t;

global workbench_t *global_workbench = NULL;
