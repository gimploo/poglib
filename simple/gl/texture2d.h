#ifndef __MY_GL_2D_TEXTURE_H__
#define __MY_GL_2D_TEXTURE_H__

/*===================================================
 // OpenGL texture handling library
==================================================== */

#include "_common.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../../external/stb_image.h"

typedef struct gl_texture2d_t {
    
    GLuint          id; 
    const char      *file_path;
    unsigned char   *buf;
    int             width;
    int             height;
    int             bpp;        //BytesPerPixel

} gl_texture2d_t;


/*-----------------------------------------------------
 // Declarations
-----------------------------------------------------*/


gl_texture2d_t              texture_init(const char * file_path);
void                        texture_destroy(gl_texture2d_t *texture);
//NOTE:(macro)              texture_bind(gl_texture2d_t *, u32 ) --> void
//NOTE:(macro)              texture_unbind(void) --> void
void                        texture_dump(gl_texture2d_t *texture);


/*------------------------------------------------------
 // Implementation
------------------------------------------------------*/

#define texture_bind(ptex, slot) {\
    GL_CHECK(glActiveTexture(GL_TEXTURE0 + (slot)));\
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, (*ptex).id));\
}
#define texture_unbind()    (GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0))


gl_texture2d_t texture_init(const char *file_path)
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
    stbi_set_flip_vertically_on_load(1);                                
    buf = stbi_load(file_path, &width, &height, &bpp, 0); //RGB
    if (buf == NULL) eprint("Failed to load texture");


    /*
     * My bitmap stuff here
     *
     */

    //NOTE: my bitmap lib works with opengl, apparantly bitmap dont have an alpha channel
    //so uncomment below lines for it to work with bitmaps and comment the stbi lines aswell
    //
    //BitMap bitmap = bitmap_init(file_path);   
    //width   = bitmap_get_width(&bitmap);
    //height  = bitmap_get_height(&bitmap);
    //buf     = (u8 *)bitmap.pixels;

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

    //stbi_image_free(buf);
    
   GL_LOG("Texture `%i` successfully created", id);

    return (gl_texture2d_t) {
        .id        = id,
        .file_path = file_path,
        .buf       = buf,
        .width     = width,
        .height    = height,
        .bpp       = bpp
    };
}

void texture_destroy(gl_texture2d_t *texture)
{
    if (texture == NULL) eprint("texture argument is null");

    GL_LOG("Texture `%i` successfully deleted", texture->id);
    GL_CHECK(glDeleteTextures(1, &texture->id)); 
}

void texture_dump(gl_texture2d_t *texture)
{
    if (texture == NULL) eprint("texture argument is null");

    const char *FMT = 
        "GLuint = %i\n"
        "file_path = %s\n"
        "buf = %s\n"
        "width = %i\n"
        "height = %i\n"
        "bpp = %i\n";

    printf(FMT,
            texture->id, 
            texture->file_path, 
            texture->buf, 
            texture->width, 
            texture->height, 
            texture->bpp);
}

#endif //__TEXTURE_H__
