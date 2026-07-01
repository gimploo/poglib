#include <poglib/ecs.h>
#include <poglib/math/la.h>
#include <poglib/basic/dbg.h>
#include <poglib/basic/runtime-ctx.h>
#include <poglib/basic/concurrency.h>
#include <poglib/poggen.h>
#include <assert.h>
#include <stdio.h>

#define TEST_FILE "./test_save.ecs"

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_ASSERT(cond, msg) do { \
    if (!(cond)) { printf("  FAIL: %s\n", msg); tests_failed++; } \
    else { printf("  PASS: %s\n", msg); tests_passed++; } \
} while(0)

static void ecs_setup(void) { global_ecs = NULL; remove(TEST_FILE); }
static void ecs_teardown(ecs_t *ecs) { if (ecs) { ecs_destroy(ecs); global_ecs = NULL; } remove(TEST_FILE); }
#define SAVE_AND_DESTROY(ecs) do { ecs_save_to_file((ecs), str(TEST_FILE)); ecs_destroy(ecs); global_ecs = NULL; } while(0)

static bgtask_manager_t g_bgtask_mgr = {0};
static arena_t           g_eng_arena;

static void engine_setup(void)
{
    global_bgtask_manager = &g_bgtask_mgr;
    g_eng_arena = arena_init(NULL, 1 * MB);
    global_engine = arena_store(&g_eng_arena,
        &(poggen_t){ .systems.assets = assetmanager_init(&g_bgtask_mgr) }, sizeof(poggen_t));
}

static void engine_teardown(void)
{
    arena_destroy(&g_eng_arena);
    global_engine = NULL;
    global_bgtask_manager = NULL;
}

/* ================================================================
   PER-COMPONENT ROUND-TRIP TESTS  (all 7 components in one save/load)
   ================================================================ */

static void test_all_component_types(void)
{
    printf("\n--- test_all_component_types ---\n");
    ecs_setup();
    ecs_t *ecs = ecs_init();

    /* entity 1: transform */
    ecs_entity_add(ecs, (ecs_componentbundle_t){
        .signature = ECS_CMP_TRANSFORM,
        .component = { [ECS_CMP_TRANSFORM_IDX].transform = (ecs_component_transform_t){
            .position = { 1.5f, 2.5f, 3.5f }, .scale = { 2.0f, 2.0f, 2.0f },
            .velocity = { 0.5f, 0.0f, -0.5f }, .source = ECS_CMP_TRANSFORM_SOURCE_INPUT,
        }}
    });
    /* entity 2: mesh + material */
    ecs_entity_add(ecs, (ecs_componentbundle_t){
        .signature = ECS_CMP_TRANSFORM | ECS_CMP_MESH | ECS_CMP_MATERIAL,
        .component = {
            [ECS_CMP_TRANSFORM_IDX].transform = (ecs_component_transform_t){ .scale = {1,1,1}, .source = ECS_CMP_TRANSFORM_SOURCE_NONE },
            [ECS_CMP_MESH_IDX].mesh = (ecs_component_mesh_t){ .asset_id = 2, .prototype_sprite_type = 20 },
            [ECS_CMP_MATERIAL_IDX].material = (ecs_component_material_t){ .texture_asset_id = 6, .shader_asset_id = 7 },
        }
    });
    /* entity 3: input */
    ecs_entity_add(ecs, (ecs_componentbundle_t){
        .signature = ECS_CMP_TRANSFORM | ECS_CMP_INPUT,
        .component = {
            [ECS_CMP_TRANSFORM_IDX].transform = (ecs_component_transform_t){
                .position = { 3,1,-7 }, .orientation = {0,0,0,1}, .source = ECS_CMP_TRANSFORM_SOURCE_INPUT,
            },
            [ECS_CMP_INPUT_IDX].input = (ecs_component_input_t){
                .direction_source = 1,
                .internal.state.current_position = {3,1,-7}, .internal.state.current_orientation = {0,0,0,1},
                .internal.state.front = { 0.5f, 0.3f, -0.8f }, .internal.state.right = { 0.8f, 0.0f, 0.5f },
            },
        }
    });
    /* entity 4: camera */
    ecs_entity_add(ecs, (ecs_componentbundle_t){
        .signature = ECS_CMP_TRANSFORM | ECS_CMP_CAMERA | ECS_CMP_INPUT,
        .component = {
            [ECS_CMP_TRANSFORM_IDX].transform = (ecs_component_transform_t){ .orientation = {0,0,0,1}, .source = ECS_CMP_TRANSFORM_SOURCE_INPUT },
            [ECS_CMP_INPUT_IDX].input = (ecs_component_input_t){ .direction_source = 0 },
            [ECS_CMP_CAMERA_IDX].camera = (ecs_component_camera_t){
                .camera.position = {10,2,-5}, .camera.euler_angle = {0.5f,-1.2f},
                .camera.direction.front = {0.3f,0.1f,-0.9f}, .camera.direction.up = {-0.1f,0.9f,0.1f}, .camera.direction.right = {0.9f,0,0.3f},
                .mode = ECS_CMP_CAMERA_MODE_ORBIT_FOLLOW, .follow.orbit_radius = 8.0f,
                .follow.center_offset = {0,1.5f,0}, .follow.track_entity_id = 5,
            },
        }
    });
    /* entity 5: collider cube */
    ecs_entity_add(ecs, (ecs_componentbundle_t){
        .signature = ECS_CMP_TRANSFORM | ECS_CMP_COLLIDER,
        .component = {
            [ECS_CMP_TRANSFORM_IDX].transform = (ecs_component_transform_t){
                .position = {0,-1,0}, .scale = {10,0.5f,10}, .source = ECS_CMP_TRANSFORM_SOURCE_NONE,
            },
            [ECS_CMP_COLLIDER_IDX].collider = (ecs_component_collider_t){
                .shape_type = COLLIDER_SHAPE_TYPE_CUBE, .motion_type = 0, .object_layer_type = 2,
                .dim.cube = { .half_width = 10, .half_height = 0.5f, .half_depth = 10 },
            },
        }
    });
    /* entity 6: model */
    ecs_entity_add(ecs, (ecs_componentbundle_t){
        .signature = ECS_CMP_TRANSFORM | ECS_CMP_MODEL | ECS_CMP_MATERIAL,
        .component = {
            [ECS_CMP_TRANSFORM_IDX].transform = (ecs_component_transform_t){ .scale = {1,1,1}, .source = ECS_CMP_TRANSFORM_SOURCE_NONE },
            [ECS_CMP_MODEL_IDX].model = (ecs_component_model_t){ .asset_id = 9 },
            [ECS_CMP_MATERIAL_IDX].material = (ecs_component_material_t){ .texture_asset_id = 0, .shader_asset_id = 10 },
        }
    });
    /* entities 7-9: multiple same type for multi-entity test */
    ecs_entity_add(ecs, (ecs_componentbundle_t){
        .signature = ECS_CMP_TRANSFORM,
        .component = { [ECS_CMP_TRANSFORM_IDX].transform = (ecs_component_transform_t){ .position={77,0,0}, .scale={1,1,1} }}
    });
    ecs_entity_add(ecs, (ecs_componentbundle_t){
        .signature = ECS_CMP_TRANSFORM,
        .component = { [ECS_CMP_TRANSFORM_IDX].transform = (ecs_component_transform_t){ .position={78,0,0}, .scale={1,1,1} }}
    });
    ecs_entity_add(ecs, (ecs_componentbundle_t){
        .signature = ECS_CMP_TRANSFORM,
        .component = { [ECS_CMP_TRANSFORM_IDX].transform = (ecs_component_transform_t){ .position={79,0,0}, .scale={1,1,1} }}
    });

    SAVE_AND_DESTROY(ecs);
    ecs_t *ecs2 = ecs_init();
    ecs_load_savefile(ecs2, str(TEST_FILE));

    /* verify entity 1: transform */
    { const ecs_entity_query_t q = ecs_entity_query_components(ecs2, 1, ECS_CMP_TRANSFORM);
      ecs_component_transform_t *t = q.entity_cmp_data[ECS_CMP_TRANSFORM_IDX];
      TEST_ASSERT(t, "transform: present");
      TEST_ASSERT(t->position.x == 1.5f && t->position.y == 2.5f && t->position.z == 3.5f, "transform: position");
      TEST_ASSERT(t->scale.x == 2.0f, "transform: scale");
      TEST_ASSERT(t->velocity.z == -0.5f, "transform: velocity");
      TEST_ASSERT(t->source == ECS_CMP_TRANSFORM_SOURCE_INPUT, "transform: source");
      TEST_ASSERT(t->orientation.w > 0.8f, "transform: orientation"); }

    /* entity 2: mesh + material */
    { const ecs_entity_query_t q = ecs_entity_query_components(ecs2, 2, ECS_CMP_MESH | ECS_CMP_MATERIAL);
      ecs_component_mesh_t *m = q.entity_cmp_data[ECS_CMP_MESH_IDX];
      ecs_component_material_t *mat = q.entity_cmp_data[ECS_CMP_MATERIAL_IDX];
      TEST_ASSERT(m, "mesh: present");
      TEST_ASSERT(m->asset_id == 2, "mesh: asset_id");
      TEST_ASSERT(m->prototype_sprite_type == 20, "mesh: sprite_type");
      TEST_ASSERT(mat, "material: present");
      TEST_ASSERT(mat->texture_asset_id == 6, "material: texture_id");
      TEST_ASSERT(mat->shader_asset_id == 7, "material: shader_id"); }

    /* entity 3: input */
    { const ecs_entity_query_t q = ecs_entity_query_components(ecs2, 3, ECS_CMP_INPUT);
      ecs_component_input_t *in = q.entity_cmp_data[ECS_CMP_INPUT_IDX];
      TEST_ASSERT(in, "input: present");
      TEST_ASSERT(in->direction_source == 1, "input: direction");
      TEST_ASSERT(in->internal.state.current_position.x == 3.0f, "input: pos.x");
      TEST_ASSERT(in->internal.state.front.x == 0.5f && in->internal.state.front.z == -0.8f, "input: front");
      TEST_ASSERT(in->internal.state.right.x == 0.8f, "input: right");
      TEST_ASSERT(in->input_behavior == NULL, "input: behavior NULL"); }

    /* entity 4: camera */
    { const ecs_entity_query_t q = ecs_entity_query_components(ecs2, 4, ECS_CMP_CAMERA);
      ecs_component_camera_t *cam = q.entity_cmp_data[ECS_CMP_CAMERA_IDX];
      TEST_ASSERT(cam, "camera: present");
      TEST_ASSERT(cam->camera.position.z == -5.0f, "camera: pos.z");
      TEST_ASSERT(cam->camera.euler_angle.x == 0.5f, "camera: euler.x");
      TEST_ASSERT(cam->camera.euler_angle.y == -1.2f, "camera: euler.y");
      TEST_ASSERT(cam->camera.direction.front.z == -0.9f, "camera: dir.front.z");
      TEST_ASSERT(cam->camera.direction.up.y == 0.9f, "camera: dir.up.y");
      TEST_ASSERT(cam->mode == ECS_CMP_CAMERA_MODE_ORBIT_FOLLOW, "camera: mode");
      TEST_ASSERT(cam->follow.orbit_radius == 8.0f, "camera: orbit_radius");
      TEST_ASSERT(cam->follow.center_offset.y == 1.5f, "camera: center_offset");
      TEST_ASSERT(cam->follow.track_entity_id == 5, "camera: track_id"); }

    /* entity 5: collider cube */
    { const ecs_entity_query_t q = ecs_entity_query_components(ecs2, 5, ECS_CMP_COLLIDER);
      ecs_component_collider_t *c = q.entity_cmp_data[ECS_CMP_COLLIDER_IDX];
      TEST_ASSERT(c, "collider(cube): present");
      TEST_ASSERT(c->shape_type == COLLIDER_SHAPE_TYPE_CUBE, "collider(cube): shape");
      TEST_ASSERT(c->object_layer_type == 2, "collider(cube): layer");
      TEST_ASSERT(c->dim.cube.half_width == 10.0f, "collider(cube): dim.w");
      TEST_ASSERT(c->dim.cube.half_height == 0.5f, "collider(cube): dim.h");
      TEST_ASSERT(c->dim.cube.half_depth == 10.0f, "collider(cube): dim.d"); }

    /* entity 6: model */
    { const ecs_entity_query_t q = ecs_entity_query_components(ecs2, 6, ECS_CMP_MODEL);
      ecs_component_model_t *m = q.entity_cmp_data[ECS_CMP_MODEL_IDX];
      TEST_ASSERT(m, "model: present");
      TEST_ASSERT(m->asset_id == 9, "model: asset_id"); }

    /* entities 7-9: multiple same type */
    { ecs_component_transform_t *t7 = ecs_entity_query_components(ecs2, 7, ECS_CMP_TRANSFORM).entity_cmp_data[ECS_CMP_TRANSFORM_IDX];
      ecs_component_transform_t *t8 = ecs_entity_query_components(ecs2, 8, ECS_CMP_TRANSFORM).entity_cmp_data[ECS_CMP_TRANSFORM_IDX];
      ecs_component_transform_t *t9 = ecs_entity_query_components(ecs2, 9, ECS_CMP_TRANSFORM).entity_cmp_data[ECS_CMP_TRANSFORM_IDX];
      TEST_ASSERT(t7 && t8 && t9, "multi: 3 entities present");
      TEST_ASSERT(t7->position.x == 77 && t8->position.x == 78 && t9->position.x == 79, "multi: positions match"); }

    /* counter: new entity after load */
    { u32 new_id = ecs_entity_add(ecs2, (ecs_componentbundle_t){
        .signature = ECS_CMP_TRANSFORM,
        .component = { [ECS_CMP_TRANSFORM_IDX].transform = (ecs_component_transform_t){ .scale={1,1,1} }}
      });
      TEST_ASSERT(new_id == 10, "counter: new entity gets id 10"); }

    ecs_teardown(ecs2);
}

/* ================================================================ */

int main(void)
{
    dbg_init();
    runtimectx_init();
    engine_setup();
    setbuf(stdout, NULL);
    printf("ecs save/load tests\n===================\n");

    test_all_component_types();

    printf("\nResults: %d passed, %d failed\n", tests_passed, tests_failed);
    engine_teardown();
    return tests_failed ? 1 : 0;
}
