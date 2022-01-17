#pragma once
#include "../lib/ecs/entitymanager.h"
#include "../lib/ecs/systems.h"

#define MAX_BULLET_SPEED    0.08f
#define BULLET_SIDE         0.03f
#define ENEMY_SIDE          0.2f
#define PLAYER_SPEED        0.6f
#define ENEMY_SPEED         0.008f

typedef enum game_entity_type {

    PLAYER,
    ENEMY,
    ENEMY_EXPLODED,
    BULLET,
    ULT,

    COUNT

} game_entity_type;

typedef struct game_t {

    entity_t        *player;
    entitymanager_t manager;

    s_renderer2d_t renderer;

} game_t;


void game_system_spawn_player(game_t *game, window_t *win)
{
    entity_t        *player     = entitymanager_add_entity(&game->manager, PLAYER);

    c_transform_t   *transform  = c_transform_init(
                                        vec3f(0.0f), vec3f(0.0f),
                                        0.02f, 0.012f);

    c_shape2d_t     *shape      = c_shape2d_init(SQUARE, 0.2, ((vec4f_t ){1.0f, 0.2f, 0.0f, 1.0f}));
    c_shader_t      *shader     = c_shader_init("./res/player.vs", "./res/player.fs");
    c_input_t       *input      = c_input_init(win);
    c_boxcollider2d_t *collider = c_boxcollider2d_init(transform->position, shape->radius);
    c_mesh2d_t      *mesh       = c_mesh2d_init(transform->position, shape->type, shape->radius);

    entity_add_component(player, transform,     c_transform_t );
    entity_add_component(player, shape,         c_shape2d_t );
    entity_add_component(player, shader,        c_shader_t );
    entity_add_component(player, input,         c_input_t );
    entity_add_component(player, collider,      c_boxcollider2d_t );
    entity_add_component(player, mesh,          c_mesh2d_t );


    game->player = player;
}

void game_system_enemy_spawner(game_t *game, f32 dt)
{
    assert(game);

    static f32 delay = 0.0f; 
    if (delay > 1.0f) {
        delay = 0.0f;
    } else {
        delay += 1.0f *dt;
        return;
    }

    entity_t *player = game->player;
    vec3f_t playerpos = ((c_transform_t *)entity_get_component(player, c_transform_t ))->position;

    entitymanager_t *manager    = &game->manager;
    entity_t *enemy             = entitymanager_add_entity(manager, ENEMY);
    assert(enemy);

    static u32 count = 0;
    vec3f_t randpos[10] = {
        vec3f(0.8f),
        vec3f(-0.8f),
        (vec3f_t ){0.8f, -0.8f, 0.0f},
        (vec3f_t ){-0.8f, 0.8f, 0.0f},

        vec3f(0.4f),
        vec3f(-0.4f),
        (vec3f_t ){-0.4f, 0.4f, 0.0f},
        (vec3f_t ){0.4f, -0.4f, 0.0f},

        vec3f(-0.9f),
        vec3f(0.9f),
    }; 
    vec3f_t enemypos = randpos[count];
    count = ++count % 10;

    vec3f_t distancevec = vec3f_sub(playerpos, enemypos);

    f32 distance = vec2f_distance(
            vec2f(enemypos), 
            vec2f(playerpos));

    f32 theta = atan2(distancevec.cmp[Y], distancevec.cmp[X]);


    printf("pos generated " VEC3F_FMT "\n", VEC3F_ARG(&enemypos));


    c_transform_t   *transform  = c_transform_init(
                        enemypos, vec3f_init(ENEMY_SPEED, theta),
                        0.03f, 0.012f);

    c_shape2d_t     *shape      = c_shape2d_init((c_shape_type)randint(0, CIRCLE), ENEMY_SIDE, COLOR_WHITE);
    c_shader_t      *shader     = c_shader_init("./res/player.vs", "./res/player.fs");
    c_boxcollider2d_t *collider = c_boxcollider2d_init(transform->position, shape->radius);
    c_mesh2d_t      *mesh       = c_mesh2d_init(transform->position, shape->type, shape->radius);

    entity_add_component(enemy, transform,     c_transform_t );
    entity_add_component(enemy, shape,         c_shape2d_t );
    entity_add_component(enemy, shader,        c_shader_t );
    entity_add_component(enemy, collider,      c_boxcollider2d_t );
    entity_add_component(enemy, mesh,          c_mesh2d_t );

}




void game_system_spawn_bullet(game_t *game)
{
    entitymanager_t *manager = &game->manager; assert(manager);
    entity_t *player = game->player; assert(player);
    assert(manager);
    assert(player);

    c_input_t *input = (c_input_t *)entity_get_component(player, c_input_t );
    assert(input);

    window_t *win = input->win; 
    assert(win);

    // bullet creation
    //
    entity_t *bullet = entitymanager_add_entity(manager, BULLET);
    assert(bullet);

    c_transform_t *playertransform  = (c_transform_t *)entity_get_component(player, c_transform_t );
    vec3f_t playerpos               = playertransform->position;

    vec2f_t mousepos = window_mouse_get_norm_position(win);
    vec2f_t distancevec = vec2f_sub(
            mousepos, 
            (vec2f_t ){playerpos.cmp[X], playerpos.cmp[Y]});

    f32 distance = vec2f_distance(
            mousepos, 
            (vec2f_t ){playerpos.cmp[X], playerpos.cmp[Y]});

    f32 theta = atan2(distancevec.cmp[Y], distancevec.cmp[X]);

    c_transform_t *t = c_transform_init(
            playerpos,
            vec3f_init(MAX_BULLET_SPEED, theta),
            0.0f, 0.0f);
    assert(t);

    c_shape2d_t *shape = c_shape2d_init(CIRCLE, BULLET_SIDE, COLOR_CYAN);
    assert(shape);

    c_boxcollider2d_t *box = c_boxcollider2d_init(t->position, BULLET_SIDE);
    assert(box);

    c_shader_t *shader = c_shader_init("./res/bullet.vs", "./res/bullet.fs");
    assert(shader);

    c_lifespan_t *life = c_lifespan_init(40);
    assert(life);

    c_mesh2d_t *mesh = c_mesh2d_init(t->position, shape->type, shape->radius);
    assert(mesh);

    entity_add_component(bullet, t, c_transform_t );
    entity_add_component(bullet, shape, c_shape2d_t );
    entity_add_component(bullet, box, c_boxcollider2d_t );
    entity_add_component(bullet, shader, c_shader_t );
    entity_add_component(bullet, life, c_lifespan_t );
    entity_add_component(bullet, mesh, c_mesh2d_t );


}

void game_system_player_update(game_t *game, f32 dt)
{
    assert(game);

    entity_t *player = game->player; assert(player);

    c_transform_t *transform        = (c_transform_t *)entity_get_component(player, c_transform_t );
    c_shape2d_t *shape              = (c_shape2d_t *)entity_get_component(player, c_shape2d_t );
    c_boxcollider2d_t  *collider    = (c_boxcollider2d_t *)entity_get_component(player, c_boxcollider2d_t );
    c_input_t  *input               = (c_input_t *)entity_get_component(player, c_input_t );
    c_shader_t *shader              = (c_shader_t *)entity_get_component(player, c_shader_t );
    c_mesh2d_t *mesh                = (c_mesh2d_t *)entity_get_component(player, c_mesh2d_t );

    window_t *win = input->win; 
    assert(win);

    transform->velocity = vec3f(0.0f);

    if (window_keyboard_is_key_pressed(win, SDLK_d)) {

        transform->velocity = (vec3f_t ){ PLAYER_SPEED * dt, 0.0f, 0.0f };
    } 

    if (window_keyboard_is_key_pressed(win, SDLK_a)) {

        transform->velocity = (vec3f_t ){ -PLAYER_SPEED *dt , 0.0f, 0.0f };

    } 

    if (window_keyboard_is_key_pressed(win, SDLK_w)) {

        transform->velocity = (vec3f_t ){ 0.0f, PLAYER_SPEED *dt, 0.0f };

    } 

    if (window_keyboard_is_key_pressed(win, SDLK_s)) {

        transform->velocity = (vec3f_t ){ 0.0f, -PLAYER_SPEED *dt, 0.0f };

    } 

    if (collision2d_check_out_of_screen(collider)) {

        transform->velocity = vec3f_scale(transform->velocity, -6.0f);

    }

    transform_update(transform);
    transform_mesh2d_update(transform ,mesh);
    transform_boxcollider2d_update(transform, collider);


    // NOTE: MANAGES THE FIRE RATE OF THE PLAYER
    static f32 framerate_counter = 0.0f;
    if (framerate_counter == 4.0f) {

        if (window_mouse_button_just_pressed(win))  game_system_spawn_bullet(game);
        framerate_counter = 0.0f;

    } else framerate_counter++;
    

}

void game_system_bullet_update(game_t *game, f32 dt)
{
    entitymanager_t *manager = &game->manager; assert(manager);
    assert(manager);

    list_t *bullets = entitymanager_get_all_entities_by_tag(manager, BULLET);

    for (u64 i = 0; i < bullets->len; i++)
    {
        entity_t *e = *(entity_t **)list_get_element_by_index(bullets, i);

        // components
        c_transform_t *transform        = (c_transform_t *)entity_get_component(e, c_transform_t );
        c_shape2d_t *shape              = (c_shape2d_t *)entity_get_component(e, c_shape2d_t );
        c_boxcollider2d_t *collider     = (c_boxcollider2d_t *)entity_get_component(e, c_boxcollider2d_t );
        c_lifespan_t *lifespan          = (c_lifespan_t *)entity_get_component(e, c_lifespan_t );
        c_mesh2d_t *mesh                = (c_mesh2d_t *)entity_get_component(e, c_mesh2d_t );
        c_shader_t *shader              = (c_shader_t *)entity_get_component(e, c_shader_t );


        if(!lifespan->is_alive) 
            entity_destroy(e);
        else 
            shape->fill.cmp[3] -= 0.02f;


        transform_update(transform);
        transform_mesh2d_update(transform, mesh);
        transform_boxcollider2d_update(transform, collider);

        lifespan->update(lifespan, 1);

    }
}

void game_system_enemy_update(game_t *game, f32 dt)
{
    entitymanager_t *manager = &game->manager;
    assert(manager);

    list_t *enemies = entitymanager_get_all_entities_by_tag(manager, ENEMY);
    assert(enemies);

    for (u64 i = 0; i < enemies->len; i++)
    {
        entity_t *e = *(entity_t **)list_get_element_by_index(enemies, i);
        assert(e);

        // components
        c_transform_t *transform        = (c_transform_t *)entity_get_component(e, c_transform_t );
        c_shape2d_t *shape              = (c_shape2d_t *)entity_get_component(e, c_shape2d_t );
        c_boxcollider2d_t *collider     = (c_boxcollider2d_t *)entity_get_component(e, c_boxcollider2d_t );
        c_mesh2d_t *mesh                = (c_mesh2d_t *)entity_get_component(e, c_mesh2d_t );
        c_shader_t *shader              = (c_shader_t *)entity_get_component(e, c_shader_t );


        transform_update(transform);
        transform_mesh2d_update(transform, mesh);
        transform_boxcollider2d_update(transform, collider);

    }
}


void game_system_spawn_explosion(game_t *game, entity_t *enemy)
{
    assert(game);
    assert(enemy);

    entitymanager_t *manager = &game->manager;
    assert(manager);

    vec3f_t enemypos = ((c_transform_t *)entity_get_component(enemy, c_transform_t ))->position;
    f32 nvertex = ((c_shape2d_t *)entity_get_component(enemy, c_shape2d_t ))->nvertex;
    c_shape_type type = ((c_shape2d_t *)entity_get_component(enemy, c_shape2d_t ))->type;

    f32 theta = 0.0f;
    for (u32 i = 0; i < nvertex; i++)
    {
        theta = (PI / 180) * (360 / nvertex) * (i + 1);

        entity_t *e = entitymanager_add_entity(manager, ENEMY_EXPLODED);

        c_transform_t   *transform  = c_transform_init(
                            enemypos, vec3f_init(ENEMY_SPEED, theta),
                            0.03f, 0.032f);

        c_shape2d_t     *shape      = c_shape2d_init(type, ENEMY_SIDE / 2.0f, COLOR_WHITE);
        c_shader_t      *shader     = c_shader_init("./res/player.vs", "./res/player.fs");
        c_mesh2d_t      *mesh       = c_mesh2d_init(transform->position, shape->type, shape->radius);
        c_lifespan_t    *lifespan   = c_lifespan_init(30);

        entity_add_component(e, transform,     c_transform_t );
        entity_add_component(e, shape,         c_shape2d_t );
        entity_add_component(e, shader,        c_shader_t );
        entity_add_component(e, mesh,          c_mesh2d_t );
        entity_add_component(e, lifespan,      c_lifespan_t );
    }

}

void game_system_explosion_update(game_t *game)
{
    assert(game);

    entitymanager_t *manager = &game->manager;
    assert(manager);

    list_t *explosions = entitymanager_get_all_entities_by_tag(manager, ENEMY_EXPLODED);
    assert(explosions);

    for (u32 i = 0; i < explosions->len; i++)
    {
        entity_t *e = *(entity_t **)list_get_element_by_index(explosions, i);
        assert(e);

        // components
        c_transform_t *transform        = (c_transform_t *)entity_get_component(e, c_transform_t );
        c_shape2d_t *shape              = (c_shape2d_t *)entity_get_component(e, c_shape2d_t );
        c_mesh2d_t *mesh                = (c_mesh2d_t *)entity_get_component(e, c_mesh2d_t );
        c_shader_t *shader              = (c_shader_t *)entity_get_component(e, c_shader_t );
        c_lifespan_t *lifespan          = (c_lifespan_t *)entity_get_component(e, c_lifespan_t );

        if (!lifespan->is_alive)
            entity_destroy(e);
        else
            shape->fill.cmp[3] -= 0.02f;

        transform_update(transform);
        transform_mesh2d_update(transform, mesh);
        lifespan->update(lifespan, 1);

    }
}

void game_system_collision(game_t *game)
{
    assert(game);

    entitymanager_t *manager = &game->manager; assert(manager);
    entity_t *player = game->player; assert(player);

    c_boxcollider2d_t *pbox = (c_boxcollider2d_t *)entity_get_component(player, c_boxcollider2d_t );
    assert(pbox);

    list_t *bullets = entitymanager_get_all_entities_by_tag(manager, BULLET);
    list_t *enemies = entitymanager_get_all_entities_by_tag(manager, ENEMY);

    for (u64 i = 0; i < enemies->len; i++)
    {
        entity_t *enemy = *(entity_t **)list_get_element_by_index(enemies, i);
        assert(enemy);
        c_boxcollider2d_t *ebox = (c_boxcollider2d_t *)entity_get_component(enemy, c_boxcollider2d_t );
        if (ebox == NULL) eprint("enemy entity is missing a box collider");

        for (u64 j = 0; j < bullets->len; j++)
        {
            entity_t *bullet = *(entity_t **)list_get_element_by_index(bullets, j);
            assert(bullet);
            c_boxcollider2d_t *bbox = (c_boxcollider2d_t *)entity_get_component(bullet, c_boxcollider2d_t );
            if (bbox == NULL) eprint("bullet entity is missing a box collider");

            if (collision2d_check_collision_by_AABB(ebox, bbox))
            {
                entity_destroy(bullet);
                entity_destroy(enemy);

                game_system_spawn_explosion(game, enemy);
            }

        }
    }

    for (u64 i = 0; i < enemies->len; i++)
    {
        entity_t *enemy = *(entity_t **)list_get_element_by_index(enemies, i);
        assert(enemy);
        c_boxcollider2d_t *ebox = (c_boxcollider2d_t *)entity_get_component(enemy, c_boxcollider2d_t );
        if (ebox == NULL) eprint("enemy entity is missing a box collider");

        if (collision2d_check_collision_by_AABB(ebox, pbox))
        {
            entity_destroy(enemy);
            //entity_destroy(player);
            game_system_spawn_explosion(game, enemy);
        }

    }
}
