#pragma once
#include <poglib/basic.h>
#include <poglib/gui.h>
#include <poglib/util/glcamera.h>
#include "./workbench-debug-renderer.h"
#include "poglib/ecs/component/types.h"

typedef enum WORKBENCH_RESERVED_ENTITY_ID {

    WORKBENCH_RESERVED_ENTITY_ID_WORLDCAMERA    = 1,

    WORKBENCH_RESERVED_ENTITY_ID_COUNT,

} WORKBENCH_RESERVED_ENTITY_ID;

typedef enum workbench_action_type {
    WORKBENCH_ACTION_TYPE_MOUSE_LEFT_CLICK_DRAG = 0,
    WORKBENCH_ACTION_TYPE_MOUSE_MIDDLE_CLICK_DRAG,
    WORKBENCH_ACTION_TYPE_MOUSE_KEYBOARD_UNSELECT_ENTITY,
    WORKBENCH_ACTION_TYPE_MOUSE_LEFT_CLICK_JUST_CLICKED,
    WORKBENCH_ACTION_TYPE_CAMERA_ZOOM_IN,
    WORKBENCH_ACTION_TYPE_CAMERA_ZOOM_IN_FAST,
    WORKBENCH_ACTION_TYPE_CAMERA_ZOOM_OUT_FAST,
    WORKBENCH_ACTION_TYPE_CAMERA_ZOOM_OUT,
    WORKBENCH_ACTION_TYPE_TOGGLE_WIREFRAME,
    WORKBENCH_ACTION_TYPE_MOUSE_ENTITY_SELECTION,
    WORKBENCH_ACTION_TYPE_KEYBOARD_COPYPASTE_ENTITY,
    WORKBENCH_ACTION_TYPE_KEYBOARD_DELETE_ENTITY,
    WORKBENCH_ACTION_TYPE_UNDO,
    WORKBENCH_ACTION_TYPE_SAVE,
    WORKBENCH_ACTION_TYPE_KEYBOARD_SELECT_PLAYER,
    WORKBENCH_ACTION_TYPE_COUNT
} workbench_action_type;

typedef struct {
    u32                     entity_id;
    ecs_componentbundle_t   cmp_data;
    bool                    is_created;
    bool                    is_deleted;
} workbench_editor_ecs_action_t;

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
        u32 prev_selected_entity_id;
        u32 current_selected_entity_id;
        ecs_component_transform_t *selected_entity_transform;

        struct {
            u32 entity_id;
            ecs_component_transform_t transform;
        } workbench_editor_action_snapshot;

        stack_t workbench_editor_action_history;

    } editor;

} workbench_t;

global workbench_t *global_workbench = NULL;
