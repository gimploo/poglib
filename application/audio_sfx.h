#pragma once
#include "basic/common.h"
#include "poglib/external/miniaudio.h"

/*  audio_sfx.h — One-shot SFX (impact stingers over music)
 *
 *  Mirrors the Wwise Stinger_impacts trigger set from Music_Khushi.
 *  Non-looping ma_sound instances mixed over the layered music.
 *  Skips Wwise-only effects (pitch shifter) for v1.
 */

#define SFX_MAX_SOUNDS 16
#define SFX_MAX_SETS    4
#define SFX_SET_MAX     8

typedef struct {
    char        name[64];
    ma_sound    sound;
    bool        loaded;
} sfx_sound_t;

typedef struct {
    char        label[32];
    u32         indices[SFX_SET_MAX];
    u32         count;
    u32         next;
} sfx_set_t;

typedef struct {
    sfx_sound_t  sounds[SFX_MAX_SOUNDS];
    u32          sound_count;
    sfx_set_t    sets[SFX_MAX_SETS];
    u32          set_count;
    bool         initialized;
} sfx_system_t;

bool    sfx_system_init(void);
u32     sfx_register(const char *name, const char *filepath, ma_engine *engine);
u32     sfx_set_create(const char *label);
void    sfx_set_add(u32 set_id, u32 sound_id);
void    sfx_play(u32 sound_id);
void    sfx_play_from_set(u32 set_id);
void    sfx_system_unload(void);

#ifndef IGNORE_AUDIO_SFX_IMPLEMENTATION

global sfx_system_t g_sfx = {0};

bool sfx_system_init(void)
{
    if (g_sfx.initialized) return true;
    memset(&g_sfx, 0, sizeof(sfx_system_t));
    g_sfx.initialized = true;
    return true;
}

u32 sfx_register(const char *name, const char *filepath, ma_engine *engine)
{
    if (g_sfx.sound_count >= SFX_MAX_SOUNDS) {
        eprint("[audio] sfx: max sounds reached\n");
        return (u32)-1;
    }

    u32 id = g_sfx.sound_count++;
    sfx_sound_t *s = &g_sfx.sounds[id];
    strncpy(s->name, name, sizeof(s->name) - 1);

    ma_result result = ma_sound_init_from_file(engine, filepath,
        0, NULL, NULL, &s->sound);
    if (result != MA_SUCCESS) {
        eprint("[audio] sfx: failed to load `%s`: %d\n", filepath, result);
        s->loaded = false;
        return (u32)-1;
    }

    s->loaded = true;
    return id;
}

u32 sfx_set_create(const char *label)
{
    if (g_sfx.set_count >= SFX_MAX_SETS) {
        eprint("[audio] sfx: max sets reached\n");
        return (u32)-1;
    }
    u32 id = g_sfx.set_count++;
    sfx_set_t *set = &g_sfx.sets[id];
    strncpy(set->label, label, sizeof(set->label) - 1);
    set->count = 0;
    set->next = 0;
    return id;
}

void sfx_set_add(u32 set_id, u32 sound_id)
{
    if (set_id >= g_sfx.set_count || sound_id >= g_sfx.sound_count) return;
    sfx_set_t *set = &g_sfx.sets[set_id];
    if (set->count >= SFX_SET_MAX) return;
    set->indices[set->count++] = sound_id;
}

void sfx_play(u32 sound_id)
{
    if (sound_id >= g_sfx.sound_count) return;
    sfx_sound_t *s = &g_sfx.sounds[sound_id];
    if (!s->loaded) return;

    ma_sound_set_volume(&s->sound, 1.0f);
    ma_sound_seek_to_pcm_frame(&s->sound, 0);
    ma_sound_start(&s->sound);
}

void sfx_play_from_set(u32 set_id)
{
    if (set_id >= g_sfx.set_count) return;
    sfx_set_t *set = &g_sfx.sets[set_id];
    if (set->count == 0) return;

    u32 idx = set->indices[set->next];
    set->next = (set->next + 1) % set->count;
    sfx_play(idx);
}

void sfx_system_unload(void)
{
    for (u32 i = 0; i < g_sfx.sound_count; i++) {
        if (g_sfx.sounds[i].loaded) {
            ma_sound_uninit(&g_sfx.sounds[i].sound);
            g_sfx.sounds[i].loaded = false;
        }
    }
    g_sfx.sound_count = 0;
    g_sfx.set_count = 0;
    g_sfx.initialized = false;
}

#endif
