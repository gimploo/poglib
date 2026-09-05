#pragma once
#include <poglib/basic.h>
#include <poglib/gui.h>
#include <poglib/util/glcamera.h>
#include "poglib/ecs/component/types.h"
#include "poglib/input/commandregistry.h"
#include "poglib/pipeline/render/render_queue.h"

typedef struct {
    glshader_t          *lineshader;
    JPH_DebugRenderer   *handle;
    JPH_DrawSettings    settings;
    renderqueue_t       *renderqueue;
    struct {
        matrix4f_t  view;
        matrix4f_t  projection;
        f32         aspect_ratio;
    } frame;
} jolt_debugrenderer_t;


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
    WORKBENCH_ACTION_TYPE_CAMERA_ZOOM_OUT,
    WORKBENCH_ACTION_TYPE_MOUSE_HIGHER_SENS,
    WORKBENCH_ACTION_TYPE_TOGGLE_WIREFRAME,
    WORKBENCH_ACTION_TYPE_MOUSE_ENTITY_SELECTION,
    WORKBENCH_ACTION_TYPE_KEYBOARD_COPY_ENTITY,
    WORKBENCH_ACTION_TYPE_KEYBOARD_PASTE_ENTITY,
    WORKBENCH_ACTION_TYPE_KEYBOARD_DELETE_ENTITY,
    WORKBENCH_ACTION_TYPE_UNDO,
    WORKBENCH_ACTION_TYPE_SAVE,
    WORKBENCH_ACTION_TYPE_EXPORT_GLB,
    WORKBENCH_ACTION_TYPE_KEYBOARD_SELECT_PLAYER,
    WORKBENCH_ACTION_TYPE_TOGGLE_JOLT_RENDERER,
    WORKBENCH_ACTION_TYPE_CLEAR,
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
    bool disable_joltrenderer;
    bool disable_grid;

    struct {
        f32 original_volume;
        bool mute_sound;
    } audio;

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

    glshader_t shader;
    vec3f_t player_camera_position;

    commandregistry_t commandregistry;

    //NOTE: brought this to have some commands persist longer than a single frame for debug purposes
    list_t persist_rendercommands;

    struct {
        u32 entity_id;
        glcamera_t *handle;
    } world_camera;

    struct {

        //NOTE: these are used for entity selection
        u32 mouse_closest_to_entity_id;
        u32 prev_selected_entity_id;
        u32 current_selected_entity_id;
        u32 copied_entity_id;
        ecs_component_transform_t *selected_entity_transform;

        struct {
            u32 entity_id;
            ecs_component_transform_t transform;
        } workbench_editor_action_snapshot;

        ds_stack_t workbench_editor_action_history;

    } editor;

    jolt_debugrenderer_t *joltrenderer;

} workbench_t;


global workbench_t *global_workbench = NULL;


