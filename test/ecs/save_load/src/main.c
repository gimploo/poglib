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
    /* entity 6: collider capsule */
    ecs_entity_add(ecs, (ecs_componentbundle_t){
        .signature = ECS_CMP_TRANSFORM | ECS_CMP_COLLIDER,
        .component = {
            [ECS_CMP_TRANSFORM_IDX].transform = (ecs_component_transform_t){ .scale = {1,1,1}, .source = ECS_CMP_TRANSFORM_SOURCE_INPUT },
            [ECS_CMP_COLLIDER_IDX].collider = (ecs_component_collider_t){
                .shape_type = COLLIDER_SHAPE_TYPE_CAPSULE, .motion_type = 1, .object_layer_type = 0,
                .dim.capsule = { .radius = 0.4f, .half_height = 0.75f },
            },
        }
    });
    /* entity 7: model */
    ecs_entity_add(ecs, (ecs_componentbundle_t){
        .signature = ECS_CMP_TRANSFORM | ECS_CMP_MODEL | ECS_CMP_MATERIAL,
        .component = {
            [ECS_CMP_TRANSFORM_IDX].transform = (ecs_component_transform_t){ .scale = {1,1,1}, .source = ECS_CMP_TRANSFORM_SOURCE_NONE },
            [ECS_CMP_MODEL_IDX].model = (ecs_component_model_t){ .asset_id = 9 },
            [ECS_CMP_MATERIAL_IDX].material = (ecs_component_material_t){ .texture_asset_id = 0, .shader_asset_id = 10 },
        }
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

    /* entity 6: collider capsule */
    { const ecs_entity_query_t q = ecs_entity_query_components(ecs2, 6, ECS_CMP_COLLIDER);
      ecs_component_collider_t *c = q.entity_cmp_data[ECS_CMP_COLLIDER_IDX];
      TEST_ASSERT(c, "collider(capsule): present");
      TEST_ASSERT(c->shape_type == COLLIDER_SHAPE_TYPE_CAPSULE, "collider(capsule): shape");
      TEST_ASSERT(c->motion_type == 1, "collider(capsule): motion");
      TEST_ASSERT(c->dim.capsule.radius == 0.4f, "collider(capsule): radius");
      TEST_ASSERT(c->dim.capsule.half_height == 0.75f, "collider(capsule): half_h"); }

    /* entity 7: model */
    { const ecs_entity_query_t q = ecs_entity_query_components(ecs2, 7, ECS_CMP_MODEL);
      ecs_component_model_t *m = q.entity_cmp_data[ECS_CMP_MODEL_IDX];
      TEST_ASSERT(m, "model: present");
      TEST_ASSERT(m->asset_id == 9, "model: asset_id"); }

    ecs_teardown(ecs2);
}

/* ================================================================
   ALL-COMPONENTS SINGLE ENTITY
   ================================================================ */

static void populate_all7(ecs_t *ecs)
{
    ecs_entity_add(ecs, (ecs_componentbundle_t){
        .signature = ECS_CMP_TRANSFORM | ECS_CMP_MODEL | ECS_CMP_INPUT |
                     ECS_CMP_MATERIAL | ECS_CMP_CAMERA | ECS_CMP_COLLIDER | ECS_CMP_MESH,
        .component = {
            [ECS_CMP_TRANSFORM_IDX].transform  = (ecs_component_transform_t){
                .position = {1,2,3}, .orientation = {0,0,0,1}, .scale = {1,1,1}, .source = ECS_CMP_TRANSFORM_SOURCE_INPUT },
            [ECS_CMP_MODEL_IDX].model     = (ecs_component_model_t){ .asset_id = 9 },
            [ECS_CMP_INPUT_IDX].input     = (ecs_component_input_t){ .direction_source = 1,
                .internal.state.current_position = {1,2,3}, .internal.state.current_orientation = {0,0,0,1} },
            [ECS_CMP_MATERIAL_IDX].material = (ecs_component_material_t){ .texture_asset_id = 6, .shader_asset_id = 7 },
            [ECS_CMP_CAMERA_IDX].camera   = (ecs_component_camera_t){ .mode = ECS_CMP_CAMERA_MODE_FREE_FLY, .follow.track_entity_id = 0 },
            [ECS_CMP_COLLIDER_IDX].collider = (ecs_component_collider_t){
                .shape_type = COLLIDER_SHAPE_TYPE_CUBE, .object_layer_type = 2, .dim.cube = {1,1,1} },
            [ECS_CMP_MESH_IDX].mesh       = (ecs_component_mesh_t){ .asset_id = 2, .prototype_sprite_type = 1 },
        }
    });
}

static void test_all_components_single_entity(void)
{
    printf("\n--- test_all_components_single_entity ---\n");
    ecs_setup();
    ecs_t *ecs = ecs_init();
    populate_all7(ecs);
    SAVE_AND_DESTROY(ecs);

    ecs_t *ecs2 = ecs_init();
    ecs_load_savefile(ecs2, str(TEST_FILE));

    const ecs_entity_query_t q = ecs_entity_query_components(ecs2, 1, ECS_CMP_TRANSFORM | ECS_CMP_MODEL |
        ECS_CMP_INPUT | ECS_CMP_MATERIAL | ECS_CMP_CAMERA | ECS_CMP_COLLIDER | ECS_CMP_MESH);
    TEST_ASSERT(q.entity_cmp_data[ECS_CMP_TRANSFORM_IDX], "all7: transform");
    TEST_ASSERT(q.entity_cmp_data[ECS_CMP_MODEL_IDX],     "all7: model");
    TEST_ASSERT(q.entity_cmp_data[ECS_CMP_INPUT_IDX],     "all7: input");
    TEST_ASSERT(q.entity_cmp_data[ECS_CMP_MATERIAL_IDX],  "all7: material");
    TEST_ASSERT(q.entity_cmp_data[ECS_CMP_CAMERA_IDX],    "all7: camera");
    TEST_ASSERT(q.entity_cmp_data[ECS_CMP_COLLIDER_IDX],  "all7: collider");
    TEST_ASSERT(q.entity_cmp_data[ECS_CMP_MESH_IDX],      "all7: mesh");

    ecs_component_transform_t *t = q.entity_cmp_data[ECS_CMP_TRANSFORM_IDX];
    ecs_component_collider_t  *c = q.entity_cmp_data[ECS_CMP_COLLIDER_IDX];
    TEST_ASSERT(t->position.x == 1 && t->position.y == 2 && t->position.z == 3, "all7: position");
    TEST_ASSERT(c->shape_type == COLLIDER_SHAPE_TYPE_CUBE, "all7: collider shape");
    ecs_teardown(ecs2);
}

/* ================================================================
   EDGE CASES
   ================================================================ */

static void populate_multi_transform(ecs_t *ecs)
{
    ecs_entity_add(ecs, (ecs_componentbundle_t){
        .signature = ECS_CMP_TRANSFORM,
        .component = { [ECS_CMP_TRANSFORM_IDX].transform = (ecs_component_transform_t){ .position = {1,0,0}, .scale={1,1,1} }}
    });
    ecs_entity_add(ecs, (ecs_componentbundle_t){
        .signature = ECS_CMP_TRANSFORM,
        .component = { [ECS_CMP_TRANSFORM_IDX].transform = (ecs_component_transform_t){ .position = {2,0,0}, .scale={1,1,1} }}
    });
    ecs_entity_add(ecs, (ecs_componentbundle_t){
        .signature = ECS_CMP_TRANSFORM,
        .component = { [ECS_CMP_TRANSFORM_IDX].transform = (ecs_component_transform_t){ .position = {3,0,0}, .scale={1,1,1} }}
    });
}

static void test_multiple_same_type(void)
{
    printf("\n--- test_multiple_same_type ---\n");
    ecs_setup();
    ecs_t *ecs = ecs_init();
    populate_multi_transform(ecs);
    SAVE_AND_DESTROY(ecs);

    ecs_t *ecs2 = ecs_init();
    ecs_load_savefile(ecs2, str(TEST_FILE));
    ecs_component_transform_t *t1 = ecs_entity_query_components(ecs2, 1, ECS_CMP_TRANSFORM).entity_cmp_data[ECS_CMP_TRANSFORM_IDX];
    ecs_component_transform_t *t2 = ecs_entity_query_components(ecs2, 2, ECS_CMP_TRANSFORM).entity_cmp_data[ECS_CMP_TRANSFORM_IDX];
    ecs_component_transform_t *t3 = ecs_entity_query_components(ecs2, 3, ECS_CMP_TRANSFORM).entity_cmp_data[ECS_CMP_TRANSFORM_IDX];
    TEST_ASSERT(t1 && t2 && t3, "multi: 3 entities");
    TEST_ASSERT(t1->position.x == 1 && t2->position.x == 2 && t3->position.x == 3, "multi: positions");
    ecs_teardown(ecs2);
}

static void populate_single_transform(ecs_t *ecs)
{
    ecs_entity_add(ecs, (ecs_componentbundle_t){
        .signature = ECS_CMP_TRANSFORM,
        .component = { [ECS_CMP_TRANSFORM_IDX].transform = (ecs_component_transform_t){ .scale={1,1,1} }}
    });
}

static void test_entity_id_counter(void)
{
    printf("\n--- test_entity_id_counter ---\n");
    ecs_setup();
    ecs_t *ecs = ecs_init();
    populate_single_transform(ecs);
    SAVE_AND_DESTROY(ecs);

    ecs_t *ecs2 = ecs_init();
    ecs_load_savefile(ecs2, str(TEST_FILE));
    u32 new_id = ecs_entity_add(ecs2, (ecs_componentbundle_t){
        .signature = ECS_CMP_TRANSFORM,
        .component = { [ECS_CMP_TRANSFORM_IDX].transform = (ecs_component_transform_t){ .scale={1,1,1} }}
    });
    TEST_ASSERT(new_id == 2, "counter: new entity gets id 2 after load");
    ecs_teardown(ecs2);
}

static void test_asset_manager_integration(void)
{
    printf("\n--- test_asset_manager_integration ---\n");
    ecs_setup();
    bgtask_manager_t tmp_mgr = {0};
    global_bgtask_manager = &tmp_mgr;

    arena_t eng_arena = arena_init(NULL, 1 * MB);
    global_engine = arena_store(&eng_arena,
        &(poggen_t){ .systems.assets = assetmanager_init(&tmp_mgr) }, sizeof(poggen_t));

    ecs_t *ecs = ecs_init();
    ecs_entity_add(ecs, (ecs_componentbundle_t){
        .signature = ECS_CMP_TRANSFORM | ECS_CMP_MESH | ECS_CMP_MATERIAL,
        .component = {
            [ECS_CMP_TRANSFORM_IDX].transform = (ecs_component_transform_t){ .scale={1,1,1}, .source=ECS_CMP_TRANSFORM_SOURCE_NONE },
            [ECS_CMP_MESH_IDX].mesh = (ecs_component_mesh_t){ .asset_id = 2 },
            [ECS_CMP_MATERIAL_IDX].material = (ecs_component_material_t){ .texture_asset_id=INVALID_ASSET_ID, .shader_asset_id=INVALID_ASSET_ID },
        }
    });
    SAVE_AND_DESTROY(ecs);

    ecs_t *ecs2 = ecs_init();
    ecs_load_savefile(ecs2, str(TEST_FILE));
    ecs_component_mesh_t     *mesh = ecs_entity_query_components(ecs2, 1, ECS_CMP_MESH).entity_cmp_data[ECS_CMP_MESH_IDX];
    ecs_component_material_t *mat  = ecs_entity_query_components(ecs2, 1, ECS_CMP_MATERIAL).entity_cmp_data[ECS_CMP_MATERIAL_IDX];
    TEST_ASSERT(mesh, "asset_mgr: mesh present");
    TEST_ASSERT(mesh->asset_id == 2, "asset_mgr: mesh id preserved");
    TEST_ASSERT(mat->texture_asset_id == INVALID_ASSET_ID, "asset_mgr: invalid texture id");
    TEST_ASSERT(mat->shader_asset_id == INVALID_ASSET_ID, "asset_mgr: invalid shader id");

    ecs_teardown(ecs2);
    arena_destroy(&eng_arena);
    global_engine = NULL;
    global_bgtask_manager = NULL;
}

/* ================================================================ */

int main(void)
{
    dbg_init();
    runtimectx_init();
    setbuf(stdout, NULL);
    printf("ecs save/load tests\n===================\n");

    test_all_component_types();         /* 40 assertions */
    test_all_components_single_entity(); /* 9 assertions */
    test_multiple_same_type();           /* 2 assertions */
    test_entity_id_counter();            /* 1 assertion */

    printf("\nResults: %d passed, %d failed\n", tests_passed, tests_failed);
    return tests_failed ? 1 : 0;
}
