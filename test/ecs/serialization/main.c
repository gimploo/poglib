#define DEBUG
#include <poglib/poggen.h>
#include <poglib/ecs.h>
#include <poglib/math/la.h>

#define TEST_FILE "./test_save.ecs"

static int tests_passed = 0;
static int tests_failed = 0;

#define CHECK(cond, msg) do { \
    if (!(cond)) { printf("  FAIL: %s\n", msg); tests_failed++; } \
    else { printf("  PASS: %s\n", msg); tests_passed++; } \
} while(0)

static void ecs_setup(void)  { global_ecs = NULL; remove(TEST_FILE); }

static bgtask_manager_t g_bgtask_mgr = {0};
static arena_t           *g_eng_arena;

static void engine_setup(void)
{
    g_bgtask_mgr = (bgtask_manager_t){
        .tasks = queue_init(4, bgtask__internal_t, NULL),
    };
    global_bgtask_manager = &g_bgtask_mgr;
    g_eng_arena = arena_init(NULL, 1 * MB);
    global_engine = arena_store(g_eng_arena,
        &(poggen_t){ .systems.assets = assetmanager_init(&g_bgtask_mgr) }, sizeof(poggen_t));
}

static void engine_teardown(void)
{
    arena_destroy(g_eng_arena);
    global_engine = NULL;
    global_bgtask_manager = NULL;
}

/* ================================================================
   multi-component save → load round-trip
   ================================================================ */

static void test_save_load_roundtrip(void)
{
    printf("\n--- test_save_load_roundtrip ---\n");
    ecs_setup();
    ecs_t *ecs = ecs_init();

    /* entity 1: TRANSFORM + INPUT + CAMERA  (state-heavy entity) */
    ecs_entity_add(ecs, (ecs_componentbundle_t){
        .signature = ECS_CMP_TRANSFORM | ECS_CMP_INPUT | ECS_CMP_CAMERA,
        .component = {
            [ECS_CMP_TRANSFORM_IDX].transform = (ecs_component_transform_t){
                .position    = { 1.5f, 2.5f, 3.5f },
                .orientation = { 0.0f, 0.0f, 0.0f, 1.0f },
                .scale       = { 2.0f, 2.0f, 2.0f },
                .velocity    = { 0.5f, 0.0f, -0.5f },
                .source      = ECS_CMP_TRANSFORM_SOURCE_INPUT,
            },
            [ECS_CMP_INPUT_IDX].input = (ecs_component_input_t){
                .direction_source = ECS_CMP_INPUT_DIRECTION_SOURCE_CAMERA,
                .internal.state = {
                    .current_position    = { 1.5f, 2.5f, 3.5f },
                    .current_orientation = { 0.0f, 0.0f, 0.0f, 1.0f },
                    .velocity            = { 0.1f, 0.2f, 0.3f },
                    .front               = { 0.5f, 0.3f, -0.8f },
                    .right               = { 0.8f, 0.0f,  0.5f },
                    .up                  = { 0.0f, 1.0f,  0.0f },
                },
            },
            [ECS_CMP_CAMERA_IDX].camera = (ecs_component_camera_t){
                .camera = {
                    .position   = { 10.0f, 2.0f, -5.0f },
                    .euler_angle = { 0.5f, -1.2f },
                    .direction  = {
                        .front = { 0.3f,  0.1f, -0.9f },
                        .up    = { -0.1f, 0.9f,  0.1f },
                        .right = { 0.9f,  0.0f,  0.3f },
                    },
                },
                .mode = ECS_CMP_CAMERA_MODE_ORBIT_FOLLOW,
                .follow = {
                    .orbit_radius      = 8.0f,
                    .center_offset     = { 0.0f, 1.5f, 0.0f },
                    .track_entity_id   = 5,
                },
            },
        }
    });

    /* entity 2: MODEL + MATERIAL + MESH  (collision-scene asset patterns) */
    ecs_entity_add(ecs, (ecs_componentbundle_t){
        .signature = ECS_CMP_TRANSFORM | ECS_CMP_MODEL | ECS_CMP_MATERIAL | ECS_CMP_MESH,
        .component = {
            [ECS_CMP_TRANSFORM_IDX].transform = (ecs_component_transform_t){
                .position    = { -3.0f, 0.0f, 7.0f },
                .orientation = { 0.0f, 0.0f, 0.0f, 1.0f },
                .scale       = { 1.0f, 1.0f, 1.0f },
                .source      = ECS_CMP_TRANSFORM_SOURCE_NONE,
            },
            [ECS_CMP_MODEL_IDX].model = (ecs_component_model_t){
                .asset_id = 5,
            },
            [ECS_CMP_MATERIAL_IDX].material = (ecs_component_material_t){
                .shader_asset_id = 6,
                .textures = {
                    .count = 1,
                    .slots = { [0] = { .uniform_name = str_lit("u_texture"), .asset_id = 7 } },
                },
            },
            [ECS_CMP_MESH_IDX].mesh = (ecs_component_mesh_t){
                .asset_id              = GL_MESH_PRIMITIVE_TYPE_CUBE,
                .prototype_sprite_type = PROTOTYPE_SPRITE_BASIC_GRAY,
            },
        }
    });

    /* entity 3: TRANSFORM + COLLIDER  (ground plane) */
    ecs_entity_add(ecs, (ecs_componentbundle_t){
        .signature = ECS_CMP_TRANSFORM | ECS_CMP_COLLIDER,
        .component = {
            [ECS_CMP_TRANSFORM_IDX].transform = (ecs_component_transform_t){
                .position = { 0.0f, -1.0f, 0.0f },
                .scale    = { 10.0f, 0.5f, 10.0f },
                .source   = ECS_CMP_TRANSFORM_SOURCE_PHYSICS,
            },
            [ECS_CMP_COLLIDER_IDX].collider = (ecs_component_collider_t){
                .shape_type        = COLLIDER_SHAPE_TYPE_CUBE,
                .motion_type       = JPH_MotionType_Static,
                .object_layer_type = 1,
                .dim = { .cube = { 10.0f, 0.5f, 10.0f } },
            },
        }
    });

    /* entity 4: TRANSFORM + COLLIDER  (wall) */
    ecs_entity_add(ecs, (ecs_componentbundle_t){
        .signature = ECS_CMP_TRANSFORM | ECS_CMP_COLLIDER | ECS_CMP_MESH | ECS_CMP_MATERIAL,
        .component = {
            [ECS_CMP_TRANSFORM_IDX].transform = (ecs_component_transform_t){
                .position = { 0.0f, 1.5f, -5.0f },
                .scale    = { 4.0f, 1.5f, 0.5f },
                .source   = ECS_CMP_TRANSFORM_SOURCE_NONE,
            },
            [ECS_CMP_COLLIDER_IDX].collider = (ecs_component_collider_t){
                .shape_type        = COLLIDER_SHAPE_TYPE_CUBE,
                .motion_type       = JPH_MotionType_Static,
                .object_layer_type = 1,
                .dim = { .cube = { 4.0f, 1.5f, 0.5f } },
            },
            [ECS_CMP_MESH_IDX].mesh = (ecs_component_mesh_t){
                .asset_id = GL_MESH_PRIMITIVE_TYPE_CUBE,
            },
            [ECS_CMP_MATERIAL_IDX].material = (ecs_component_material_t){
                .shader_asset_id = 6,
            },
        }
    });

    /* entity 5: TRANSFORM + COLLIDER  (capsule — player pattern) */
    ecs_entity_add(ecs, (ecs_componentbundle_t){
        .signature = ECS_CMP_TRANSFORM | ECS_CMP_INPUT | ECS_CMP_MODEL | ECS_CMP_MATERIAL | ECS_CMP_COLLIDER,
        .component = {
            [ECS_CMP_TRANSFORM_IDX].transform = (ecs_component_transform_t){
                .position = { 2.0f, 0.0f, 0.0f },
                .scale    = { 1.0f, 1.0f, 1.0f },
                .source   = ECS_CMP_TRANSFORM_SOURCE_INPUT,
            },
            [ECS_CMP_INPUT_IDX].input = (ecs_component_input_t){
                .direction_source = ECS_CMP_INPUT_DIRECTION_SOURCE_CAMERA,
            },
            [ECS_CMP_MODEL_IDX].model = (ecs_component_model_t){
                .asset_id = 5,
            },
            [ECS_CMP_MATERIAL_IDX].material = (ecs_component_material_t){
                .shader_asset_id = 6,
                .textures = {
                    .count = 1,
                    .slots = { [0] = { .uniform_name = str_lit("u_texture"), .asset_id = 7 } },
                },
            },
            [ECS_CMP_COLLIDER_IDX].collider = (ecs_component_collider_t){
                .shape_type        = COLLIDER_SHAPE_TYPE_CAPSULE,
                .motion_type       = JPH_MotionType_Kinematic,
                .object_layer_type = 0,
                .dim = { .capsule = { .half_height = 0.75f, .radius = 0.4f } },
            },
        }
    });

    /* entity 6: TRANSFORM + CAMERA  (player follow camera) */
    ecs_entity_add(ecs, (ecs_componentbundle_t){
        .signature = ECS_CMP_TRANSFORM | ECS_CMP_CAMERA | ECS_CMP_INPUT,
        .component = {
            [ECS_CMP_TRANSFORM_IDX].transform = (ecs_component_transform_t){
                .orientation = { 0.0f, 0.0f, 0.0f, 1.0f },
                .source      = ECS_CMP_TRANSFORM_SOURCE_INPUT,
            },
            [ECS_CMP_CAMERA_IDX].camera = (ecs_component_camera_t){
                .camera = {
                    .position   = { 0.0f, 0.0f, 0.0f },
                    .euler_angle = { 0.0f, 0.0f },
                },
                .mode   = ECS_CMP_CAMERA_MODE_ORBIT_FOLLOW,
                .follow = {
                    .orbit_radius    = 5.0f,
                    .center_offset   = { 0.0f, 1.5f, 0.0f },
                    .track_entity_id = 5,
                },
            },
            [ECS_CMP_INPUT_IDX].input = (ecs_component_input_t){
                .direction_source = ECS_CMP_INPUT_DIRECTION_SOURCE_CAMERA,
            },
        }
    });

    /* entities 7-9: TRANSFORM only (test multiple entities of same type) */
    for (u32 i = 0; i < 3; i++)
        ecs_entity_add(ecs, (ecs_componentbundle_t){
            .signature = ECS_CMP_TRANSFORM,
            .component = {
                [ECS_CMP_TRANSFORM_IDX].transform = (ecs_component_transform_t){
                    .position = { 100.0f + (f32)i, 0.0f, 0.0f },
                    .scale    = { 1.0f, 1.0f, 1.0f },
                    .source   = ECS_CMP_TRANSFORM_SOURCE_NONE,
                },
            },
        });

    /* entity 10: all 7 components in one entity  (stress test) */
    ecs_entity_add(ecs, (ecs_componentbundle_t){
        .signature = ECS_CMP_TRANSFORM | ECS_CMP_MODEL | ECS_CMP_INPUT
                   | ECS_CMP_MATERIAL | ECS_CMP_CAMERA | ECS_CMP_COLLIDER
                   | ECS_CMP_MESH,
        .component = {
            [ECS_CMP_TRANSFORM_IDX].transform = (ecs_component_transform_t){
                .position    = { 5.0f, 10.0f, 15.0f },
                .orientation = { 0.0f, 1.0f, 0.0f, 0.0f },
                .scale       = { 3.0f, 3.0f, 3.0f },
                .velocity    = { 1.0f, 0.0f, -1.0f },
                .source      = ECS_CMP_TRANSFORM_SOURCE_INPUT,
            },
            [ECS_CMP_MODEL_IDX].model = (ecs_component_model_t){
                .asset_id = 5,
            },
            [ECS_CMP_INPUT_IDX].input = (ecs_component_input_t){
                .direction_source = ECS_CMP_INPUT_DIRECTION_SOURCE_ENTITY,
                .internal.state = {
                    .current_position    = { 5.0f, 10.0f, 15.0f },
                    .current_orientation = { 0.0f, 1.0f, 0.0f, 0.0f },
                    .velocity = { 0.0f, 0.0f, -1.0f },
                    .front = { 0.0f, 0.0f, -1.0f },
                    .right = { 1.0f, 0.0f, 0.0f },
                    .up    = { 0.0f, 1.0f, 0.0f },
                },
            },
            [ECS_CMP_MATERIAL_IDX].material = (ecs_component_material_t){
                .shader_asset_id = 6,
                .textures = {
                    .count = 3,
                    .slots = {
                        [0] = { .uniform_name = str_lit("u_texture"),      .asset_id = 7 },
                        [1] = { .uniform_name = str_lit("light.ambient"),  .asset_id = 7 },
                        [2] = { .uniform_name = str_lit("light.color"),    .asset_id = 7 },
                    },
                },
            },
            [ECS_CMP_CAMERA_IDX].camera = (ecs_component_camera_t){
                .camera = {
                    .position   = { 0.0f, 5.0f, -10.0f },
                    .euler_angle = { 0.0f, 0.0f },
                    .direction  = {
                        .front = { 0.0f, 0.0f, -1.0f },
                        .up    = { 0.0f, 1.0f,  0.0f },
                        .right = { 1.0f, 0.0f,  0.0f },
                    },
                },
                .mode = ECS_CMP_CAMERA_MODE_FREE_FLY,
            },
            [ECS_CMP_COLLIDER_IDX].collider = (ecs_component_collider_t){
                .shape_type        = COLLIDER_SHAPE_TYPE_SPHERE,
                .motion_type       = JPH_MotionType_Dynamic,
                .object_layer_type = 0,
                .dim = { .sphere = { 1.5f } },
            },
            [ECS_CMP_MESH_IDX].mesh = (ecs_component_mesh_t){
                .asset_id              = GL_MESH_PRIMITIVE_TYPE_CYLINDER,
                .prototype_sprite_type = PROTOTYPE_SPRITE_CHECKERED_LIGHT_GRAY,
            },
        }
    });

    /* --- save to file --- */

    ecs_save_to_file(ecs, str(TEST_FILE));

    /* inject dummy assets (uniforms + tilecount) for load-phase testing */
    {
        u64 sz = file_get_size(TEST_FILE);
        char *buf = calloc(1, sz + 512);
        file_t f = file_init(TEST_FILE, "r");
        file_readall(&f, buf, sz);
        file_destroy(&f);

        const char *marker = "section_end:assets\n";
        char *insert = strstr(buf, marker);
        if (insert)
        {
            const char *extra =
                "\tassetid:6,assettype:1,assetpath:[res/test_shader.vs,res/test_shader.fs]\n"
                "\t\tsection_begin:uniforms\n"
                "\t\t\tuniform_name:projection,type:0\n"
                "\t\t\tuniform_name:view,type:0\n"
                "\t\tsection_end:uniforms\n"
                "\tassetid:7,assettype:3,assetpath:[res/test_sprites.png]\n"
                "\t\ttilecount:[8,4]\n";
            u32 extra_len = (u32)strlen(extra);

            u64 tail = sz - (u64)(insert - buf);
            memmove(insert + extra_len, insert, tail);
            memcpy(insert, extra, extra_len);

            f = file_init(TEST_FILE, "w");
            file_writebytes(&f, buf, sz + extra_len);
            file_destroy(&f);
        }
        free(buf);
    }

    printf("  saved entities to %s\n", TEST_FILE);

    CHECK(file_check_exist(TEST_FILE), "save: file created");

    ecs_destroy(ecs);
    global_ecs = NULL;

    /* --- load & verify skipped (ecs_load_savefile in progress) --- */
}

void test_load_savefile_roundtrip()
{
    ecs_t *ecs = ecs_init();
    {
        ecs_load_savefile(ecs, str("./test_save.ecs"));
    }
    ecs_destroy(ecs);
}


/* ================================================================ */

int main(void)
{
    dbg_init();
    runtimectx_init();
    engine_setup();

    printf("=== ECS Serialization Tests ===\n");

    test_save_load_roundtrip();
    test_load_savefile_roundtrip();

    printf("\n--- results ---\n");
    printf("  passed: %d\n", tests_passed);
    printf("  failed: %d\n", tests_failed);

    if (tests_failed) printf("\n  SOME TESTS FAILED!\n");

    engine_teardown();
    runtimectx_destroy();
    dbg_destroy();
    return tests_failed ? 1 : 0;
}
