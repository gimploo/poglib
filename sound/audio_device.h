#pragma once
#include "./common.h"

global audio_device_t       global_audio_device = {0};
global ma_engine            global_audio_engine = {0};

void                        audio_device_init(void);
void                        audio_device_destroy(void);


#ifndef IGNORE_AUDIO_DEVICE_IMPLEMENTATION

void audio_device_init(void)
{
    if (global_audio_device.is_initialized) 
        eprint("audio device already initalized, trying to reinitalize again");

    global_audio_device.config = ma_engine_config_init();
    global_audio_device.config.channels    = 2;
    global_audio_device.config.sampleRate  = 48000;

    const ma_result result = ma_engine_init(&global_audio_device.config, &global_audio_engine);
    if (result != MA_SUCCESS) 
        eprint("Failed to initialize miniaudio engine: %d\n", result);

    global_audio_device.is_initialized = true;

    ma_engine_set_volume(&global_audio_engine, 0.5f);

    logging("[audio] miniaudio engine initialized (2ch, 48kHz)");
}

void audio_device_destroy(void)
{
    if (!global_audio_device.is_initialized) return;

    ma_engine_uninit(&global_audio_engine);
    global_audio_device.is_initialized = false;
    logging("[audio] miniaudio engine shutdown");
}

#endif
