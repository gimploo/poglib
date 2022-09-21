#include <poglib/basic.h>
#define _WIN64

int test1(void)
{
    const char *path01 = "C:\\Users\\User\\OneDrive\\Documents\\projects\\poglib\\external\\cglm\\struct\\clipspace";
    /*const char *path02 = "/mnt/c/Users/User/OneDrive/Documents/projects/poglib/test/str";*/
    str_t directory01 = str_get_directory_path(path01);
    /*str_t directory02 = str_get_directory_path(path02);*/
    str_print(&directory01);
    return 0;
}

int test2(void)
{
    const char *path = "mnt/c/df/ad/";
    const char *file = "file";

    char buffer[32];
    cstr_combine_path(path, file, buffer);

    printf("%s\n", buffer);
}

void main(void)
{
    test2();
}
