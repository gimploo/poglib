#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>


#include "../../basic.h"
#include "../bitmap.h"
#include "../png.h"


int main(void)
{
    Png img = png_init("./charmap-oldschool_black.png");
    png_dump_everything(&img); 

    return 0;
}

int oldmain(void)
{
    /*BitMap img = bitmap_init("./sample.bmp");*/
    BitMap img = bitmap_init("./charmap-oldschool_black.png");
    bitmap_dump_spec(&img);

    bitmap_clone_image(&img, bitmap_get_width(&img), bitmap_get_height(&img));

    bitmap_destroy(&img);




    return 0;
}

