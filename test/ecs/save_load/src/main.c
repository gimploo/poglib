#include <poglib/ecs.h>
#include <poglib/math/la.h>
#include <poglib/basic/dbg.h>
#include <poglib/basic/runtime-ctx.h>
#include <assert.h>
#include <stdio.h>

#define TEST_FILE "./test_save.ecs"

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_ASSERT(cond, msg) do { \
    if (!(cond)) { printf("  FAIL: %s\n", msg); tests_failed++; } \
    else { printf("  PASS: %s\n", msg); tests_passed++; } \
} while(0)

static void test_roundtrip_transform_only(void)
{
    printf("\n--- test_roundtrip_transform_only ---\n");
    remove(TEST_FILE);

    ecs_t *ecs = ecs_init();
    assert(ecs);

    (void)ecs_entity_add(ecs, (ecs_componentbundle_t){
        .signature = ECS_CMP_TRANSFORM,
        .component = {
            [ECS_CMP_TRANSFORM_IDX].transform = (ecs_component_transform_t){
                .position    = { 1.5f, 2.5f, 3.5f },
                .orientation = { 0.1f, 0.2f, 0.3f, 0.9f },
                .scale       = { 2.0f, 2.0f, 2.0f },
                .velocity    = { 0.5f, 0.0f, -0.5f },
                .source      = ECS_CMP_TRANSFORM_SOURCE_INPUT,
            },
        }
    });

    ecs_save_to_file(ecs, str(TEST_FILE));
    ecs_destroy(ecs);
    global_ecs = NULL;

    ecs_t *ecs2 = ecs_init();
    ecs_load_savefile(ecs2, str(TEST_FILE));

    const ecs_entity_query_t q = ecs_entity_query_components(ecs2, 1, ECS_CMP_TRANSFORM);
    ecs_component_transform_t *t = q.entity_cmp_data[ECS_CMP_TRANSFORM_IDX];

    TEST_ASSERT(t != NULL, "entity created");
    TEST_ASSERT(t->position.x == 1.5f && t->position.y == 2.5f && t->position.z == 3.5f, "position matches");
    TEST_ASSERT(t->scale.x == 2.0f && t->scale.y == 2.0f && t->scale.z == 2.0f, "scale matches");
    TEST_ASSERT(t->velocity.x == 0.5f && t->velocity.z == -0.5f, "velocity matches");
    TEST_ASSERT(t->source == ECS_CMP_TRANSFORM_SOURCE_INPUT, "source matches");
    /* orientation gets normalized during ecs_entity_add, check rough equality */
    TEST_ASSERT(t->orientation.w > 0.8f, "orientation normalised");

    ecs_destroy(ecs2);
    global_ecs = NULL;
    remove(TEST_FILE);
}

static void test_roundtrip_multiple_entities(void)
{
    printf("\n--- test_roundtrip_multiple_entities ---\n");
    remove(TEST_FILE);

    ecs_t *ecs = ecs_init();
    assert(ecs);

    ecs_entity_add(ecs, (ecs_componentbundle_t){
        .signature = ECS_CMP_TRANSFORM | ECS_CMP_MESH | ECS_CMP_MATERIAL,
        .component = {
            [ECS_CMP_TRANSFORM_IDX].transform = (ecs_component_transform_t){
                .position = { 10.0f, 0.0f, 5.0f },
                .scale    = { 1.0f, 1.0f, 1.0f },
                .source   = ECS_CMP_TRANSFORM_SOURCE_NONE,
            },
            [ECS_CMP_MESH_IDX].mesh = (ecs_component_mesh_t){
                .asset_id = 2,
                .prototype_sprite_type = 20,
            },
            [ECS_CMP_MATERIAL_IDX].material = (ecs_component_material_t){
                .texture_asset_id = 6,
                .shader_asset_id  = 7,
            },
        }
    });

    ecs_entity_add(ecs, (ecs_componentbundle_t){
        .signature = ECS_CMP_TRANSFORM | ECS_CMP_COLLIDER,
        .component = {
            [ECS_CMP_TRANSFORM_IDX].transform = (ecs_component_transform_t){
                .position = { 0.0f, -1.0f, 0.0f },
                .scale    = { 5.0f, 0.5f, 5.0f },
                .source   = ECS_CMP_TRANSFORM_SOURCE_NONE,
            },
            [ECS_CMP_COLLIDER_IDX].collider = (ecs_component_collider_t){
                .shape_type       = COLLIDER_SHAPE_TYPE_CUBE,
                .motion_type      = 0,
                .object_layer_type = 2,
                .dim.cube = { .half_width = 5.0f, .half_height = 0.5f, .half_depth = 5.0f },
            },
        }
    });

    ecs_entity_add(ecs, (ecs_componentbundle_t){
        .signature = ECS_CMP_TRANSFORM | ECS_CMP_INPUT | ECS_CMP_CAMERA,
        .component = {
            [ECS_CMP_TRANSFORM_IDX].transform = (ecs_component_transform_t){
                .position    = { 5.0f, 2.0f, -10.0f },
                .orientation = { 0.0f, 0.0f, 0.0f, 1.0f },
                .source      = ECS_CMP_TRANSFORM_SOURCE_INPUT,
            },
            [ECS_CMP_INPUT_IDX].input = (ecs_component_input_t){
                .direction_source = 0,
                .internal.state.current_position = { 5.0f, 2.0f, -10.0f },
            },
            [ECS_CMP_CAMERA_IDX].camera = (ecs_component_camera_t){
                .mode = ECS_CMP_CAMERA_MODE_FREE_FLY,
                .follow.orbit_radius = 5.0f,
                .follow.track_entity_id = 1,
            },
        }
    });

    ecs_save_to_file(ecs, str(TEST_FILE));
    ecs_destroy(ecs);
    global_ecs = NULL;

    ecs_t *ecs2 = ecs_init();
    ecs_load_savefile(ecs2, str(TEST_FILE));

    /* verify entity 1 */
    {
        const ecs_entity_query_t q = ecs_entity_query_components(ecs2, 1, ECS_CMP_TRANSFORM | ECS_CMP_MESH | ECS_CMP_MATERIAL);
        ecs_component_transform_t *t   = q.entity_cmp_data[ECS_CMP_TRANSFORM_IDX];
        ecs_component_mesh_t      *m   = q.entity_cmp_data[ECS_CMP_MESH_IDX];
        ecs_component_material_t  *mat = q.entity_cmp_data[ECS_CMP_MATERIAL_IDX];
        TEST_ASSERT(t && m && mat, "entity 1 all components present");
        TEST_ASSERT(t->position.x == 10.0f, "entity 1 position.x");
        TEST_ASSERT(m->asset_id == 2 && m->prototype_sprite_type == 20, "entity 1 mesh");
        TEST_ASSERT(mat->texture_asset_id == 6 && mat->shader_asset_id == 7, "entity 1 material");
    }

    /* verify entity 2 */
    {
        const ecs_entity_query_t q = ecs_entity_query_components(ecs2, 2, ECS_CMP_TRANSFORM | ECS_CMP_COLLIDER);
        ecs_component_transform_t *t = q.entity_cmp_data[ECS_CMP_TRANSFORM_IDX];
        ecs_component_collider_t *c  = q.entity_cmp_data[ECS_CMP_COLLIDER_IDX];
        TEST_ASSERT(t && c, "entity 2 all components present");
        TEST_ASSERT(t->position.y == -1.0f, "entity 2 position.y");
        TEST_ASSERT(c->shape_type == COLLIDER_SHAPE_TYPE_CUBE, "entity 2 collider type");
        TEST_ASSERT(c->dim.cube.half_depth == 5.0f, "entity 2 collider dim");
    }

    /* verify entity 3 */
    {
        const ecs_entity_query_t q = ecs_entity_query_components(ecs2, 3, ECS_CMP_TRANSFORM | ECS_CMP_INPUT | ECS_CMP_CAMERA);
        ecs_component_transform_t *t = q.entity_cmp_data[ECS_CMP_TRANSFORM_IDX];
        ecs_component_input_t    *in = q.entity_cmp_data[ECS_CMP_INPUT_IDX];
        ecs_component_camera_t   *cam = q.entity_cmp_data[ECS_CMP_CAMERA_IDX];
        TEST_ASSERT(t && in && cam, "entity 3 all components present");
        TEST_ASSERT(t->source == ECS_CMP_TRANSFORM_SOURCE_INPUT, "entity 3 source");
        TEST_ASSERT(in->direction_source == 0, "entity 3 direction");
        TEST_ASSERT(cam->mode == ECS_CMP_CAMERA_MODE_FREE_FLY, "entity 3 camera mode");
        TEST_ASSERT(cam->follow.track_entity_id == 1, "entity 3 track entity");
    }

    ecs_destroy(ecs2);
    global_ecs = NULL;
    remove(TEST_FILE);
}

int main(void)
{
    dbg_init();
    runtimectx_init();
    setbuf(stdout, NULL); /* unbuffered output */
    printf("ecs save/load tests\n===================\n");

    test_roundtrip_transform_only();
    test_roundtrip_multiple_entities();

    printf("\nResults: %d passed, %d failed\n", tests_passed, tests_failed);
    return tests_failed ? 1 : 0;
}
