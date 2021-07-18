/*
 * source:  http://www.ece.ualberta.ca/~elliott/ee552/studentAppNotes/2003_w/misc/bmp_file_format/bmp_file_format.htm
 * wiki:    https://en.wikipedia.org/wiki/BMP_file_format
 *
 */

#ifndef __BITMAP_H__
#define __BITMAP_H__

#include <stdio.h>
#include <stdlib.h>

#include "../file.h"

typedef unsigned char   u8;
typedef char            i8;
typedef u_int16_t       u16;
typedef int16_t         i16;
typedef u_int32_t       u32;
typedef u_int64_t       u64;
typedef int             i32;
typedef long            i64;
typedef float           f32;
typedef double          f64;

// Disables padding in structs
#define NOPADDING __attribute__((packed)) 
#define KB 1024

#define eprint(fmt, ...) {\
    fprintf(stderr, "(%s:%d):%s -> ",__FILE__, __LINE__, __func__); \
    fprintf(stderr, fmt"\n" __VA_OPT__(,)  __VA_ARGS__);\
    exit(1);                                    \
}

typedef struct BITMAP_header BITMAP_header;
struct NOPADDING BITMAP_header {
    char    format[2];
    u32     file_size;
    u16     r_tmp1;     // <-|reserved field (dont know what its for) 
    u16     r_tmp2;     // <-|
    u32     image_offset;
};

typedef struct DIB_header DIB_header;
struct NOPADDING DIB_header {
    u32     header_size;
    u32     width;
    u32     height;
    u16     colorplanes;
    u16     bitsperpixel;
    u32     compression;
    u32     image_size;
    u8      dogshit[100]; // extra padding for BITMAPV5HEADER
};

typedef struct Color Color;
struct NOPADDING Color {
    u8 r;
    u8 g;
    u8 b;
};

typedef struct Pixel_data Pixel_data;
struct Pixel_data {
    Color pixel;
};

typedef struct BitMapImage BitMapImage;
struct BitMapImage {
    BITMAP_header bm_h;
    DIB_header    dib_h;
    Pixel_data    *pixels;
};





BitMapImage     bitmap_init(File *file);
void            bitmap_clone_image(BitMapImage *img, size_t width, size_t height);
void            bitmap_dump_spec(BitMapImage *img);
void            bitmap_destroy(BitMapImage *img);





void bitmap_destroy(BitMapImage *img)
{
    if (img == NULL) eprint("img is null");

    free(img->pixels);
}

void bitmap_dump_spec(BitMapImage *img)
{
    if (img == NULL) eprint("img is null");

    printf("Header size: \t\t%i\n",     img->bm_h.file_size);
    printf("Header char: \t\t%c%c\n",   img->bm_h.format[0], img->bm_h.format[1]);
    printf("Header offset img: \t%i\n", img->bm_h.image_offset);
    printf("DIB header size: \t%i\n",   img->dib_h.header_size);
    printf("DIB width: \t\t%i\n",       img->dib_h.width);
    printf("DIB height: \t\t%i\n",      img->dib_h.height);
    printf("DIB colorplanes: \t%i\n",   img->dib_h.colorplanes);
    printf("DIB bitsperpixel: \t%i\n",  img->dib_h.bitsperpixel);
    printf("DIB compression: \t%i\n",   img->dib_h.compression);
    printf("DIB image_size: \t%i\n",    img->dib_h.image_size);
    
    DIB_header *dib = &img->dib_h;

    switch (dib->header_size)
    {
        case 12:
            printf("Bitmap header:  \tBITMAPCOREHEADER OS21XBITMAPHEADER\n");
            break;
        case 64:
        case 16:
            printf("Bitmap header:  \tOS22XBITMAPHEADER\n");
            break;
        case 40:
            printf("Bitmap header:  \tBITMAPINFOHEADER\n");
            break;
        case 52:
            printf("Bitmap header:  \tBITMAPV2INFOHEADER\n");
            break;
        case 56:
            printf("Bitmap header:  \tBITMAPV3INFOHEADER\n");
            break;
        case 108:
            printf("Bitmap header:  \tBITMAPV4HEADER\n");
            break;
        case 124:
            printf("Bitmap header:  \tBITMAPV5HEADER\n");
            break;
        default:
            printf("Bitmap header:  \tUnkown\n");
            break;
    }
    switch(dib->compression)
    {
        case 0:
            fprintf(stderr, "Compression type:  \tBI_RGB (no compression)\n");
            break;
        case 1:
            fprintf(stderr, "Compression type:  \tBI_RLE8\n");
            break;
        case 2:
            fprintf(stderr, "Compression type:  \tBI_RLE4\n");
            break;
        default:
            fprintf(stderr, "Compression type:  \tUnkown\n");
            break;
    }

    switch(dib->bitsperpixel)
    {
        case 1:
            fprintf(stderr, "BitsPerPixel used:  \tmonochrome palette\n");
            break;
        case 4:
            fprintf(stderr, "BitsPerPixel used:  \t4bit palletized\n");
            break;
        case 8:
            fprintf(stderr, "BitsPerPixel used:  \t8bit palletized\n");
            break;
        case 16:
            fprintf(stderr, "BitsPerPixel used:  \t16bit RGB\n");
            break;
        case 24:
            fprintf(stderr, "BitsPerPixel used:  \t24bit RGB\n");
            break;
        default:
            fprintf(stderr, "BitsPerPixel used:  \tUnkown\n");
            break;
    }

}

BitMapImage bitmap_init(File *file)
{
    if (file == NULL) eprint("file arg is null\n");

    BitMapImage img = {0};
    fread(&img.bm_h, sizeof(BITMAP_header), 1, file->fp); 
    fread(&img.dib_h, sizeof(DIB_header), 1, file->fp); 


    if (img.dib_h.compression != 0) {

        fprintf(stderr, "Error: bitmap is compressed\n");
        bitmap_dump_spec(&img);
        exit(1);

    } else if (img.dib_h.bitsperpixel != 24 
                || img.dib_h.header_size != 124) {

        fprintf(stderr, "Error: Bitmap format not supported yet\n");
        bitmap_dump_spec(&img);
        exit(1);
    }

    fseek(file->fp, img.bm_h.image_offset, SEEK_SET);

    u32 width = img.dib_h.width;
    u32 height = img.dib_h.height;

    img.pixels = (Pixel_data *)malloc(sizeof(Pixel_data) * height * width); 
    fread(img.pixels, width * height * sizeof(Pixel_data),1, file->fp);


    return img;

}

void bitmap_clone_image(BitMapImage *img, size_t width, size_t height)
{
    FILE *file = fopen("tmp.bmp", "wb");
    if (file == NULL) eprint("file arg is null\n");

    fwrite(&img->bm_h,sizeof(BITMAP_header), 1, file);
    fwrite(&img->dib_h, sizeof(DIB_header), 1, file);
    fwrite(img->pixels, width * height * sizeof(Pixel_data), 1, file);

    fclose(file);
}



#endif //__BITMAP_H__
