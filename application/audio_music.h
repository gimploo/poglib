#pragma once
// =============================================================================
// PLANNED(feat/miniaudio-music-system) — NEW FILE (comments only until approved)
// =============================================================================
// Purpose:
//   Hybrid Wwise authoring → custom runtime music system (NO Wwise SDK).
//   Mirrors Project Vedanta_theme Music_Khushi design:
//     - 4 synced looping layers @ 80 BPM (bed + layer1/2/3)
//     - RTPC Parkour_intensity (0..100) drives layer volumes via curves
//     - play/stop/fade; intensity smoothed over time
//
// After approval, implement roughly:
//   bool music_load_theme(const char *music_def_json_path);
//     - parse res/audio/music_def.json (layer paths + volume_db curves)
//     - stream WAVs via miniaudio (do not fully load ~25MB layers into RAM)
//   void music_play(void);
//   void music_stop(f32 fade_sec);
//   void music_set_parkour_intensity(f32 value_0_100);  // target; lerp inside update
//   void music_update(f32 dt);
//   void music_unload(void);
//
// Curves (from Wwise RTPC, re-exported in music_def.json — do NOT parse .wwu):
//   bed:    always audible
//   layer1/2/3: silent (~-200 dB) until intensity ~45, full by ~60
//
// Out of scope v1: Player_health lowpass, Wwise pitch-shifter plugin, bank loading.
// =============================================================================
