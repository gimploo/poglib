#ifndef __MY_PNG_H__
#define __MY_PNG_H__

#include "../basic.h"
#include "../file.h"

typedef struct Png          Png;
typedef struct png_header   png_header;
typedef struct png_data     png_data;

// NOTE: png is too fucking complicated 


struct NOPADDING png_header {
    
    char    format[8];
    u32     width;
    u32     height;
    u8      bit_depth; // not per pixel but number of bits per sample or per pallete index
    u8      color_type;
    u8      compression;
    u8      filer_method;
    u8      interlace_method;

};

struct NOPADDING png_data {
    u32 IDAT;
};

struct Png {
    png_header IHDR;
    png_data   IDAT;

};




Png     png_init(const char *file_path);
void    png_dump_everything(Png *png);




bool __png_check_format(File *file)
{
    if (file == NULL) eprint("file argument is null");
    if (!file_open(file, "rb")) eprint("Failed to open file");

    u8 buf[9] = {0};

    fread(buf, 9, 1, file->fp);
    file_close(file);

    if (
            buf[0] == 0x89 
            && buf[1] == 0x50 
            && buf[2] == 0x4E 
            && buf[3] == 0x47 
            && buf[4] == 0x0D
            && buf[5] == 0x0A
            && buf[6] == 0x1A
            && buf[7] == 0x0A

        ) return true;
    

    
    return false;
}

Png png_init(const char *file_path)
{
    if (file_path == NULL) eprint("file argument is null");
    
    Png png_file = {0};

    File file = file_init(file_path);
    file_open(&file , "rb");
    fread(&png_file.IHDR, sizeof(png_file.IHDR), 1, file.fp);
    
    file_destroy(&file);

    return png_file; 
}

void png_dump_everything(Png *png)
{
    if (!png) eprint("file argument is null");

    png_header head = png->IHDR;

    printf("format              = ");
    for (int i = 0; i < sizeof(png->IHDR.format); i++)
        printf("0x%.2X ", head.format[i]);

    printf("\nwidth               = %i\n",head.width); 
    printf("height              = %i\n",head.height); 
    printf("bit_depth           = %i\n",head.bit_depth); 
    printf("color_type          = %i\n",head.color_type); 
    printf("compression         = %i\n",head.compression); 
    printf("filter_method       = %i\n",head.filer_method); 
    printf("interlace_method    = %i\n",head.interlace_method);
}




#endif // __MY_PNG_H__
