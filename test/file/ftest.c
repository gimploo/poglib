#include <stdio.h>
#include "../file.h"
#include "../str.h"



int main(void)
{
    File file = file_init("ftest.c");
    char *buffer = file_readall(&file);
    /*str_t *buffer = str_read_file_to_str(file.name);*/
    printf("%s", buffer);
    /*str_print(buffer);*/

    FILE *tmp = fopen("tmp.txt", "a");
    fwrite(buffer, file.size, 1, tmp);
    /*fwrite(buffer->buf, buffer->size, 1, tmp);*/
    fclose(tmp);
    
    free(buffer);
    /*pstr_free(buffer);*/

    return 0;
}
