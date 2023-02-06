#ifndef __MY_GL_2D_TEXTURE_H__
#define __MY_GL_2D_TEXTURE_H__

/*===================================================
 // OpenGL texture handling library
==================================================== */

#include "common.h"
#include <poglib/image.h>

typedef struct gltexture2d_t {
    
    GLuint          id; 
    const char      uniform[16];
    const char      filepath[64];
    const char      type[32];
    unsigned char   *buf;
    int             width;
    int             height;
    int             bpp;        //BytesPerPixel

} gltexture2d_t;


/*-----------------------------------------------------
 // Declarations
-----------------------------------------------------*/


gltexture2d_t        gltexture2d_init(const char * filepath);
gltexture2d_t        gltexture2d_empty_init(u32 width, u32 height);
void                 gltexture2d_destroy(const gltexture2d_t *texture);
//NOTE:(macro)       gltexture2d_bind(gltexture2d_t *, u32 slot) --> void
//NOTE:(macro)       gltexture2d_unbind(void) --> void
void                 gltexture2d_dump(const gltexture2d_t *texture);


/*------------------------------------------------------
 // Implementation
------------------------------------------------------*/

#define gltexture2d_bind(ptex, slot) do {\
\
    GL_CHECK(glActiveTexture(GL_TEXTURE0 + (slot)));\
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, (ptex)->id));\
\
} while(0)
#define gltexture2d_unbind()    (GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0))


//NOTE: this function was created for framebuffers since they use color textures also the data is stored in RGB not RGBA8
gltexture2d_t gltexture2d_empty_init(u32 width, u32 height)
{   
    GLuint id;

    GL_CHECK(glGenTextures(1, &id));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, id));

    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));	
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));


    GL_CHECK(glTexImage2D(
        GL_TEXTURE_2D, 
        0, 
        GL_RGB, // The format opengl will store the pixel data in
        width,
        height,
        0,
        GL_RGB, // The format the buf variable is in
        GL_UNSIGNED_BYTE,
        NULL
     ));

    GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
    GL_LOG("Texture `%i` successfully created", id);

    return (gltexture2d_t) {
        .id        = id,
        .filepath = NULL,
        .buf       = NULL,
        .width     = (int)width,
        .height    = (int)height,
        .bpp       = 0,
    };

}

gltexture2d_t gltexture2d_init(const char *filepath)
{
    if (filepath == NULL) eprint("file argument is null");

    GLuint id;
    int width = 0, height = 0, bpp = 0;
    u8 *buf = NULL;
    stbi_set_flip_vertically_on_load(1);                                
    buf = stbi_load(filepath, &width, &height, &bpp, 4); //forcing RGBA per pixel
    if (buf == NULL) eprint("Failed to load texture");

    GLenum format;
    if (bpp == 1)       format = GL_RED;
    else if (bpp == 3)  format = GL_RGB;
    else if (bpp == 4)  format = GL_RGBA;

    GL_CHECK(glGenTextures(1, &id));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, id));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));	
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
    GL_CHECK(glTexImage2D(
        GL_TEXTURE_2D, 
        0, 
        format, 
        width,
        height,
        0,
        format, // The format the buf variable is in
        GL_UNSIGNED_BYTE,
        buf
     ));
    GL_CHECK(glGenerateMipmap(GL_TEXTURE_2D));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
    GL_LOG("Texture `%i` successfully created", id);

    gltexture2d_t o = {
        .id         = id,
        .filepath   = {0},
        .type       = {0},
        .buf        = buf,
        .width      = width,
        .height     = height,
        .bpp        = bpp,
    };

    const u64 len = strlen(filepath);
    if (len > sizeof(o.filepath)) eprint("filepath too long");
    memcpy((char *)o.filepath, filepath, len);

    return o;
}

void gltexture2d_destroy(const gltexture2d_t *texture)
{
    if (texture == NULL) eprint("texture argument is null");

    GL_LOG("Texture `%i` successfully deleted", texture->id);
    GL_CHECK(glDeleteTextures(1, &texture->id)); 

    if (texture->buf) stbi_image_free(texture->buf);
}

void gltexture2d_dump(const gltexture2d_t *texture)
{
    if (texture == NULL) eprint("texture argument is null");

    const char *FMT = 
        "GLuint = %i\n"
        "filepath = %s\n"
        "buf = %s\n"
        "width = %i\n"
        "height = %i\n"
        "bpp = %i\n";

    printf(FMT,
            texture->id, 
            texture->filepath, 
            texture->buf, 
            texture->width, 
            texture->height, 
            texture->bpp);
}

#endif //__TEXTURE_H__
