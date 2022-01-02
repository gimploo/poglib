#include <stdio.h>
#include ".././queue.h"
#include "../../math/vec.h"

void print_int(void *elem)
{
    int value = *(int *)elem;
    printf("%i ", value);
}

typedef struct foo {

    vec3f_t x;

} foo;

void print_pfoo(void *arg)
{
    vec3f_t *vec = *(vec3f_t **)arg;
    printf("["VEC3F_FMT"] ", VEC3F_ARG(vec));
}

void print_foo(void *arg)
{
    vec3f_t *vec = (vec3f_t *)arg;
    printf("["VEC3F_FMT"] ", VEC3F_ARG(vec));
}

int main(void)
{
    queue_t queue = queue_init(10, vec3f_t *);
    /*queue_t queue = queue_init(10, vec3f_t );*/

    vec3f_t vecs[10] = { vec3f(0.0f), vec3f(1.0f), vec3f(2.0f) };

    vec3f_t *pvecs[3] = { &vecs[1], &vecs[2], &vecs[3] };

    /*queue_print(&queue, print_foo);*/
    queue_print(&queue, print_pfoo);

    for (int i = 0; i < 3; i++)
        queue_put(&queue, pvecs[i]);

    /*queue_print(&queue, print_foo);*/
    queue_print(&queue, print_pfoo);

    for (int i = 0; i < 3; i++)
    {
        // pointer
            vec3f_t *vec = *(vec3f_t **)queue_get(&queue);

        // value
        /*vec3f_t *vec = (vec3f_t *)queue_get(&queue);*/

        printf(VEC3F_FMT"\n", VEC3F_ARG(vec));
        queue_print(&queue, print_pfoo);
        /*queue_print(&queue, print_foo);*/
    }

    /*queue_print(&queue, print_foo);*/
    queue_print(&queue, print_pfoo);
    queue_destroy(&queue);

    return 0;
}
