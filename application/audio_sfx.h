#pragma once
// =============================================================================
// PLANNED(feat/miniaudio-music-system) — NEW FILE (comments only until approved)
// =============================================================================
// Purpose:
//   One-shot SFX mixed over music (Wwise stinger set Stinger_impacts).
//
// After approval, implement roughly:
//   bool sfx_load_from_music_def(const char *music_def_json_path); // or separate list
//   void sfx_play(const char *name);
//   void sfx_play_random_impact(void);  // Impact_KL_01..04
//   void sfx_unload(void);
//
// Uses audio_device_get_engine(). Non-looping ma_sound instances.
// Skip Wwise-only effects on some impacts (pitch shifter) for v1.
// =============================================================================
