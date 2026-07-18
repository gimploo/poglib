#pragma once
// =============================================================================
// PLANNED(feat/miniaudio-music-system) — NEW FILE (comments only until approved)
// =============================================================================
// Purpose:
//   Own the global miniaudio engine/device for the whole process.
//
// After approval, implement roughly:
//   bool audio_device_init(void);
//     - #define MINIAUDIO_IMPLEMENTATION in exactly one TU (this file or a .c)
//     - ma_engine_init / ma_engine_uninit
//     - master volume default
//   void audio_device_shutdown(void);
//   ma_engine *audio_device_get_engine(void);  // used by audio_music / audio_sfx
//
// Depends on: lib/poglib/external/miniaudio.h (to be vendored).
// Wired from: application.h application_run init + shutdown (see PLANNED comments there).
// =============================================================================
