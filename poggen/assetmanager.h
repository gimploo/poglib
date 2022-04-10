#pragma once
#include "../ds/list.h"
#include "../ds/map.h"
#include "../poggen/asset.h"



/*==============================================================================
                        -- ASSET MANAGER --
===============================================================================*/



typedef struct assetmanager_t assetmanager_t ;

assetmanager_t  assetmanager_init(void);

#define         assetmanager_add_shader(PASSETMANAGER, LABEL, VERTEXFILE, FRAGMENTFILE)    __impl_assetmanager_add_asset((PASSETMANAGER), (LABEL), (VERTEXFILE), (FRAGMENTFILE), AT_GLSHADER) 
#define         assetmanager_add_texture2d(PASSETMANAGER, LABEL, FILEPATH)                 __impl_assetmanager_add_asset((PASSETMANAGER), (LABEL), (FILEPATH), NULL, AT_GLTEXTURE2D) 
#define         assetmanager_add_freetypefont(PASSETMANAGER, LABEL, FILEPATH)              __impl_assetmanager_add_asset((PASSETMANAGER), (LABEL), (FILEPATH), NULL, AT_FONT_TTF) 

#define         assetmanager_get_shader(PASSETMANAGER, LABEL)       __impl_assetmanager_get_asset((PASSETMANAGER), (LABEL), AT_GLSHADER)
#define         assetmanager_get_texture2d(PASSETMANAGER, LABEL)    __impl_assetmanager_get_asset((PASSETMANAGER), (LABEL), AT_GLTEXTURE2D)
#define         assetmanager_get_freetypefont(PASSETMANAGER, LABEL) __impl_assetmanager_get_asset((PASSETMANAGER), (LABEL), AT_FONT_TTF)

void            assetmanager_destroy(assetmanager_t *self);





#ifndef IGNORE_ASSETMANAGER_IMPLEMENTATION


struct assetmanager_t {

    list_t assetmaps;

};

assetmanager_t assetmanager_init(void)
{
    assetmanager_t o = {
        .assetmaps = list_init(AT_COUNT, map_t )
    };

    for (u32 i = 0; i < AT_COUNT; i++)
    {
        map_t *elem = (map_t *)list_get_value(&o.assetmaps, i);
        switch(i)
        {
            case AT_GLSHADER: {
                *elem = map_init(10, asset_t );
            } break;

            case AT_GLTEXTURE2D: {
                *elem = map_init(10, asset_t );
            } break;

            case AT_SOUND_WAV:
                *elem = map_init(4, asset_t );
            break;

            case AT_FONT_FreeType: {
                *elem = map_init(4, asset_t );
            } break;

            default: eprint("type not accounted for");
        }
    }

    return o;
}

asset_t * __impl_assetmanager_add_asset(assetmanager_t *manager, const char *label, const char *filepath01, const char *filepath02, asset_type type) 
{
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
            output.texture2d = gltexture2d_init(filepath01);
        break;

        case AT_FONT_FreeType:
            output.font = glfreetypefont_init(filepath01, 45);
        break;

        case AT_SOUND_WAV:
            eprint("TODO : asset type");
        break;

        default: eprint("type not accounted for");
    }

    map_t *table = (map_t *)list_get_value(&manager->assetmaps, type);
    assert(table);
    
    return (asset_t *)map_insert(table, label, output);

}


void assetmanager_destroy(assetmanager_t *self) 
{
    assert(self);

    for (u32 i = 0; i < AT_COUNT; i++)
    {
        map_t *table = (map_t *)list_get_value(&self->assetmaps, i);
        assert(table);

        switch(i)
        {
            case AT_GLSHADER:
                for (u32 i = 0; i < table->len; i++)
                {
                    const char *value = (const char *)list_get_value(&table->__keys, i);
                    assert(value);

                    asset_t *asset = (asset_t *)map_get_value(table, value);
                    assert(asset);

                    glshader_destroy(&asset->shader);
                }
            break;

            case AT_GLTEXTURE2D:
                for (u32 i = 0; i < table->len; i++)
                {
                    const char *value = (const char *)list_get_value(&table->__keys, i);
                    assert(value);

                    asset_t *asset = (asset_t *)map_get_value(table, value);
                    assert(asset);

                    gltexture2d_destroy(&asset->texture2d);
                }
            break;

            case AT_FONT_FreeType:
                for (u32 i = 0; i < table->len; i++)
                {
                    const char *value = (const char *)list_get_value(&table->__keys, i);
                    assert(value);

                    asset_t *asset = (asset_t *)map_get_value(table, value);
                    assert(asset);

                    glfreetypefont_destroy(&asset->font);
                }
            break;

            case AT_SOUND_WAV:
                // TODO: 
            break;

            default: eprint("type not accounted for ");
        }
        map_destroy(table);

    }

    list_destroy(&self->assetmaps);
}


asset_t * __impl_assetmanager_get_asset(assetmanager_t *manager, const char *label, asset_type type)
{
    assert(manager);

    map_t *table = (map_t *)list_get_value(&manager->assetmaps, type);
    assert(table);

    return (asset_t *)map_get_value(table, label);
}

#endif
