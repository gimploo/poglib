#include <poglib/ds.h>

typedef struct foo {
    const char label[16];
    int list[5];
} foo;
void print_foo(void *x)
{
    foo *tmp = (foo *) x;
    printf("%s ", tmp->label);
    for (int i = 0; i < 5; i++)
        printf(" %i",tmp->list[i]);
    printf("\n");
}

void test01(void)
{    
    printf("loading\n");
    hashtable_t output;
    file_t *file = file_init("config", "rb");
    output = file_load_hashtable(file);
    file_destroy(file);

    hashtable_dump(&output);
    hashtable_print(&output, print_foo);
    hashtable_destroy(&output);
}

void test02(void)
{    
    printf("loading\n");
    slot_t output;
    file_t *file = file_init("config", "rb");
    output = file_load_slot(file);
    file_destroy(file);

    slot_dump(&output);
    slot_print(&output, print_foo);
    slot_destroy(&output);
}

int main(void)
{
    /*test01();*/
    test02();
    return 0;
}
