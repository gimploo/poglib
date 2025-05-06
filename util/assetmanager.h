#pragma once
#include <poglib/basic.h>
#include "./asset.h"


/*==============================================================================
                        -- ASSET MANAGER --
===============================================================================*/

typedef struct assetmanager_t {

    hashtable_t assetmaps[AT_COUNT];

} assetmanager_t ;


assetmanager_t  assetmanager_init(void);

#define         assetmanager_add_shader(PASSETMANAGER, LABEL, VERTEXFILE, FRAGMENTFILE)    __impl_assetmanager_add_asset((PASSETMANAGER), (LABEL), (VERTEXFILE), (FRAGMENTFILE), 0, AT_GLSHADER) 
#define         assetmanager_add_texture2d(PASSETMANAGER, LABEL, FILEPATH)                 __impl_assetmanager_add_asset((PASSETMANAGER), (LABEL), (FILEPATH), NULL, 0, AT_GLTEXTURE2D) 
#define         assetmanager_add_freetypefont(PASSETMANAGER, LABEL, FILEPATH, FONTSIZE)    __impl_assetmanager_add_asset((PASSETMANAGER), (LABEL), (FILEPATH), NULL, (FONTSIZE), AT_FONT_FREETYPE) 

#define         assetmanager_get_shader(PASSETMANAGER, LABEL)                   (glshader_t *)__impl_assetmanager_get_asset((PASSETMANAGER), (LABEL), AT_GLSHADER)
#define         assetmanager_get_texture2d(PASSETMANAGER, LABEL)                (gltexture2d_t *)__impl_assetmanager_get_asset((PASSETMANAGER), (LABEL), AT_GLTEXTURE2D)
#define         assetmanager_get_freetypefont(PASSETMANAGER, LABEL)             (glfreetypefont_t *)__impl_assetmanager_get_asset((PASSETMANAGER), (LABEL), AT_FONT_FREETYPE)

void            assetmanager_destroy(assetmanager_t *self);


/*------------------------------------------------------------------------------
                        IMPLEMENTATION
-------------------------------------------------------------------------------*/

#ifndef IGNORE_ASSETMANAGER_IMPLEMENTATION


assetmanager_t assetmanager_init(void)
{
    return (assetmanager_t ){
        .assetmaps = {
            [AT_GLSHADER]       = hashtable_init(10, asset_t ),
            [AT_GLTEXTURE2D]    = hashtable_init(10, asset_t ),
            [AT_SOUND_WAV]      = hashtable_init(10, asset_t ),
            [AT_FONT_FREETYPE]  = hashtable_init(10, asset_t ),
        }
    };
}

asset_t * __impl_assetmanager_add_asset(assetmanager_t *manager, const char *label, const char *filepath01, const char *filepath02, const u32 fontsize, asset_type type) 
{
    eprint("Not implemented");
    /*
    assert(filepath01);
    assert(manager);

    asset_t output = {
        .label = label,
        .filepath01 = filepath01,
        .filepath02 = filepath02,
        .type = type
    };

    switch(type)
    {
        case AT_GLSHADER:
            assert(filepath02);
            output.shader = glshader_from_file_init(filepath01, filepath02);
        break;

        case AT_GLTEXTURE2D:
        {
            gltexture2d_t tex = gltexture2d_init(filepath01);
            memcpy(&output.texture2d, &tex, sizeof(gltexture2d_t));
        }
        break;

        case AT_FONT_FREETYPE:
        {
            assert(fontsize != 0);
            glfreetypefont_t font = glfreetypefont_init(filepath01, fontsize);
            memcpy(&output.font, &font, sizeof(font)); 
        }
        break;

        case AT_SOUND_WAV:
            eprint("TODO : asset type");
        break;

        default: eprint("type not accounted for");
    }

    hashtable_t *table = &manager->assetmaps[type];
    assert(table);
    
    return (asset_t *)hashtable_insert(table, label, output);
    */
}


void assetmanager_destroy(assetmanager_t *self) 
{
    eprint("Not implemented");
    /*
    assert(self);

    for (u32 type = 0; type < AT_COUNT; type++)
    {
        hashtable_t *table = &self->assetmaps[type];
        assert(table);

        switch(type)
        {
            case AT_GLSHADER: 
                hashtable_iterator(table, asset) {
                    glshader_t *shader = &((table_entry_t *)entry)->shader;
                    glshader_destroy(shader);
                }
            break;
            case AT_GLTEXTURE2D:
                hashtable_iterator(table, asset) {
                    gltexture2d_t *tex = &((asset_t *)asset)->texture2d;
                    gltexture2d_destroy(tex);
                }
            break;

            case AT_FONT_FREETYPE:
                hashtable_iterator(table, asset) {
                    glfreetypefont_t *font = &((asset_t *)asset)->font;
                    glfreetypefont_destroy(font);
                }
            break;

            case AT_SOUND_WAV:
                // TODO: 
            break;

            default: eprint("type not accounted for ");
        }
        hashtable_destroy(table);

    }
        */
}


const void * __impl_assetmanager_get_asset(const assetmanager_t *manager, const char *label, asset_type type)
{
    assert(manager);

    const hashtable_t *table = &manager->assetmaps[type];
    assert(table);

    asset_t *asset = (asset_t *)hashtable_get_value(table, label);
    assert(asset);

    switch(type)
    {
        case AT_GLSHADER: 
            return &((asset_t *)asset)->shader;
        break;
        case AT_GLTEXTURE2D:
            return &((asset_t *)asset)->texture2d;
        break;

        case AT_FONT_FREETYPE:
            return &((asset_t *)asset)->font;
        break;

        case AT_SOUND_WAV:
            eprint("TODO");
        break;

        default: eprint("Asset `%s` type not accounted for ", label);
    }

}

#endif
