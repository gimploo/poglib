#ifndef __GL_TEXTURE_H__
#define __GL_TEXTURE_H__


#include "gl_common.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../misc/stb_image.h"

#include "../image/bitmap.h"
#include "../file.h"


//NOTE: color is of RGB type 
//TODO: support RGBA



typedef struct Texture2D Texture2D;

struct Texture2D {
    
    GLuint id; const char *file_path;
    unsigned char * buf;
    int width;
    int height;
    int bpp; //BytesPerPixel


};





Texture2D   texture_init(const char * file_path);
void        texture_destroy(Texture2D *texture);

#define texture_bind(ptex, slot) {\
    GL_CHECK(glActiveTexture(GL_TEXTURE0 + (slot)));\
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, (*ptex).id));\
}

#define texture_unbind()    GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0)         





Texture2D texture_init(const char *file_path)
{
    if (file_path == NULL) eprint("file argument is null");

    GLuint id;
    int width = 0, height = 0, bpp = 0;
    u8 *buf = NULL;

    /*
     *
     * STBI stuff here
     *
     */
    // opengl (0,0) starts at the bottom left
    //stbi_set_flip_vertically_on_load(1);                                
    //buf = stbi_load(file_path, &width, &height, &bpp, 0); //RGB
    //if (buf == NULL) eprint("Failed to load texture");


    /*
     * My bitmap stuff here
     *
     */

    //NOTE: my bitmap lib works with opengl, apparantly bitmap dont have an alpha channel
    //so uncomment below lines for it to work with bitmaps and comment the stbi lines aswell
    //
    BitMap bitmap = bitmap_init(file_path);   
    width   = bitmap_get_width(&bitmap);
    height  = bitmap_get_height(&bitmap);
    buf     = (u8 *)bitmap.pixels;

    GL_CHECK(glGenTextures(1, &id));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, id));

    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));	
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

    GL_CHECK(glTexImage2D(
        GL_TEXTURE_2D, 
        0, 
        GL_RGBA8, // The format opengl will store the pixel data in
        width,
        height,
        0,
        GL_RGB, // The format the buf variable is in
        GL_UNSIGNED_BYTE,
        buf
     ));

    GL_CHECK(glGenerateMipmap(GL_TEXTURE_2D));

    GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));

    stbi_image_free(buf);
    
    printf("[POG]\tTexture `%i` successfully created\n", id);

    return (Texture2D) {
        .id        = id,
        .file_path = file_path,
        .buf       = NULL,
        .width     = width,
        .height    = height,
        .bpp       = bpp
    };
}

void texture_destroy(Texture2D *texture)
{
    if (texture == NULL) eprint("texture argument is null");

    GL_CHECK(glDeleteTextures(1, &texture->id));
    printf("[INFO]\tTexture `%i` successfully deleted\n", texture->id);
}


#endif //__TEXTURE_H__
