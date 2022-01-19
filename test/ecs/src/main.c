#include "../../../application.h"
#include "../../../ecs/entitymanager.h"
#include "../../../ecs/systems.h"


int main(void)
{
    entitymanager_t manager = entitymanager_init(6);
    s_renderer2d_t render = s_renderer2d_init();

    c_transform_t *t1 = c_transform_init(vec3f(0.0f), vec3f(0.0f), 0.0f, 0.0f);
    c_transform_t *t2 = c_transform_init(vec3f(0.0f), vec3f(0.0f), 0.0f, 0.0f);
    c_transform_t *t3 = c_transform_init(vec3f(0.0f), vec3f(0.0f), 0.0f, 0.0f);
    c_transform_t *t4 = c_transform_init(vec3f(0.0f), vec3f(0.0f), 0.0f, 0.0f);
    c_transform_t *t5 = c_transform_init(vec3f(0.0f), vec3f(0.0f), 0.0f, 0.0f);

    entity_t *e1 = entitymanager_add_entity(&manager, 0);
    entity_add_component(e1, t1, c_transform_t );
    entity_t *e2 = entitymanager_add_entity(&manager, 0);
    entity_add_component(e2, t2, c_transform_t );
    entity_t *e3 = entitymanager_add_entity(&manager, 0);
    entity_add_component(e3, t3, c_transform_t );
    entity_t *e4 = entitymanager_add_entity(&manager, 0);
    entity_add_component(e4, t4, c_transform_t );
    entity_t *e5 = entitymanager_add_entity(&manager, 0);
    entity_add_component(e5, t5, c_transform_t );

    window_t win = window_init("test", 700, 600, SDL_INIT_VIDEO);

    while(win.is_open)
    {
        window_update_user_input(&win);

        entitymanager_update(&manager);
    }

    entitymanager_destroy(&manager);
    window_destroy(&win);
}

