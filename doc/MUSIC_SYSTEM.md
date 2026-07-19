# Hybrid Wwise / miniaudio Music System

## Table of Contents

1. [What This System Does](#what-this-system-does)
2. [Why a Hybrid Approach](#why-a-hybrid-approach)
3. [Architecture Overview](#architecture-overview)
4. [Wwise Authoring Project (Original Design)](#wwise-authoring-project-original-design)
5. [How It Maps to Code](#how-it-maps-to-code)
6. [File Reference](#file-reference)
7. [How the Music Layers Work](#how-the-music-layers-work)
8. [How the RTPC Intensity System Works](#how-the-rtpc-intensity-system-works)
9. [How Impact Stingers Work](#how-impact-stingers-work)
10. [Game Integration (Vedanta)](#game-integration-vedanta)
11. [Runtime Data Flow](#runtime-data-flow)
12. [Building and Running](#building-and-running)
13. [Tradeoffs and Limitations](#tradeoffs-and-limitations)
14. [Future Improvements](#future-improvements)
15. [Troubleshooting](#troubleshooting)

---

## What This System Does

This system plays **adaptive layered music** during gameplay. The music consists of
4 audio tracks (called "layers") that play simultaneously, synced to the same
tempo (80 BPM). A runtime parameter called `Parkour_intensity` (a number from 0
to 100) controls which layers are audible:

- At **low intensity** (idle, walking): only the **bed** layer plays (the musical
  foundation).
- At **medium intensity** (running, sliding): **layer1** fades in on top of the bed.
- At **high intensity** (wallrunning, vaulting): **layer2 and layer3** also fade in,
  creating a full, intense sound.

Additionally, **impact stinger** sound effects (short percussive hits) are triggered
during parkour actions like vaulting.

---

## Why a Hybrid Approach

Wwise is a professional audio middleware that provides:
- A visual authoring tool for designing adaptive music
- RTPC curves, state machines, stinger triggers, bus routing, effects
- SoundBank generation for runtime playback

However, integrating the Wwise SDK into this engine would add:
- A large proprietary runtime dependency (~50-100 MB of DLLs)
- Licensing costs for commercial deployment
- Platform-specific build complexity
- A heavy SDK that conflicts with the engine's minimal C11 header-only design

**The hybrid approach** uses Wwise purely as an **offline authoring tool**:
1. Design the music in Wwise (layers, curves, stingers, states)
2. Export the raw WAV files from Wwise
3. Extract the curve data from Wwise `.wwu` XML files by hand
4. Hardcode the curves and file paths in C
5. Play everything at runtime using **miniaudio** (a single-header C audio library)

This gives us the creative power of Wwise's authoring workflow with zero runtime
dependency on the Wwise SDK.

---

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────┐
│                    WWISE (Offline Only)                      │
│                                                             │
│  Music_Khushi container                                     │
│  ├── PV_main-theme_bed_80bpm.wav    (always audible)       │
│  ├── PV_main-theme_layer1_80bpm.wav (fades in at ~20-31)   │
│  ├── PV_main-theme_layer2_80bpm.wav (fades in at ~45-60)   │
│  ├── PV_main-theme_layer3_80bpm.wav (fades in at ~59-80)   │
│  ├── Stinger_impacts                                      │
│  │   ├── Impact_KL_01.wav                                  │
│  │   ├── Impact_KL_02.wav                                  │
│  │   ├── Impact_KL_03.wav                                  │
│  │   └── Impact_KL_04.wav                                  │
│  └── RTPC: Parkour_intensity (0..100)                       │
└──────────────────────────────┬──────────────────────────────┘
                               │ Export WAVs + extract curves
                               ▼
┌─────────────────────────────────────────────────────────────┐
│                  RUNTIME (miniaudio)                         │
│                                                             │
│  music_config.h    → hardcoded file paths + curve tables    │
│  audio_music.h     → 4-layer streaming + intensity curves   │
│  audio_sfx.h       → impact stinger pool                    │
│  audio_device.h    → miniaudio engine init/shutdown         │
│                                                             │
│  Game calls:                                                │
│    music_config_load()      → loads all WAVs                │
│    music_theme_play()       → starts all layers             │
│    music_theme_set_intensity(N) → sets target (0..100)      │
│    music_theme_update(dt)   → smooths + applies curves      │
│    music_config_play_impact() → plays next stinger          │
│    music_theme_stop(fade)   → fades out and stops           │
│    music_config_unload()    → frees everything              │
└─────────────────────────────────────────────────────────────┘
```

---

## Wwise Authoring Project (Original Design)

The Wwise project lives at:
```
E:\dev\resources\Project Vedanta_theme\Project Vedanta_theme\
```

### Container: Music_Khushi

The main music container (`Containers/Music_Khushi.wwu`) defines:

**MusicSegment: PV_Main-theme** (the composite segment containing all 4 tracks)

| Track | WAV File | Duration (ms) | Tempo |
|-------|----------|---------------|-------|
| bed | PV_main-theme_bed_80bpm.wav | 91,262 | 80 BPM |
| layer1 | PV_main-theme_layer1_80bpm.wav | 84,000 | 80 BPM |
| layer2 | PV_main-theme_layer2_80bpm.wav | 95,358 | 80 BPM |
| layer3 | PV_main-theme_layer3_80bpm.wav | 88,531 | 80 BPM |

All tracks are looped. The Wwise segment exit cue is at 80,000 ms (exactly 80
BPM * 1000 ms/beat = 40 bars at 80 BPM).

**MusicTrack: Stinger_impacts** (randomly selected one-shot hits)

| Clip | WAV File | Notes |
|------|----------|-------|
| Impact_KL_01 | Impact_KL_01.wav | Trimmed to ~3s segment |
| Impact_KL_02 | Impact_KL_02.wav | Has Wwise Pitch Shifter (-100 cents) |
| Impact_KL_03 | Impact_KL_03.wav | Has Wwise Pitch Shifter (-100 cents) |
| Impact_KL_04 | Impact_KL_04.wav | ~4s duration |

### Game Parameter: Parkour_intensity

Defined in `Game Parameters/Default Work Unit.wwu`:
- Range: 0 to 100
- Initial value: 0
- Controls the Volume RTPC on each music track

### RTPC Volume Curves (extracted from Music_Khushi.wwu)

Each track's `MusicTrack` has an `RTPC` binding that maps `Parkour_intensity`
to `Volume` (in dB). Here are the exact curve points from the Wwise XML:

**Bed track** — no RTPC binding on the track itself (always at 0 dB).
The bed has a clip-level Volume curve that stays at 0 dB across the full
intensity range.

**Layer1** (`Parkour_intensity` → Volume):

| Intensity (X) | Volume dB (Y) |
|----------------|---------------|
| 0 | -200 (silent) |
| 20.506 | -200 (silent) |
| 31.078 | 0.018 (near full) |
| 100 | 0 (full) |

Layer1 fades in between intensity 20 and 31.

**Layer2** (`Parkour_intensity` → Volume):

| Intensity (X) | Volume dB (Y) |
|----------------|---------------|
| 0 | -200 (silent) |
| 45.032 | -200 (silent) |
| 60.042 | 0 (full) |
| 100 | 0 (full) |

Layer2 fades in between intensity 45 and 60.

**Layer3** (`Parkour_intensity` → Volume):

| Intensity (X) | Volume dB (Y) |
|----------------|---------------|
| 0 | -200 (silent) |
| 59.197 | -200 (silent) |
| 80.127 | -0.159 (near full) |
| 100 | 0 (full) |

Layer3 fades in between intensity 59 and 80.

### State Group: Gameplay_state

Defined in `States/Default Work Unit.wwu`:
- **None** (Id: 748895195) — default state
- **Play** (Id: 1256202815) — active gameplay state

The `Play` event (in `test_bank`) sets `Gameplay_state` to `Play`. In our
hybrid approach, this state machine is replaced by explicit
`music_theme_play()` / `music_theme_stop()` calls.

---

## How It Maps to Code

### music_config.h — The Translation Layer

This file is the **single source of truth** that maps Wwise authoring data to C
constants. When the Wwise project changes, this is the only file that needs
updating.

```c
// File paths (must match res/audio/ layout)
static const str_t MUSIC_BASE_PATH = { "res/audio", 9, { false } };
static const str_t MUSIC_LAYER_FILES[] = {
    { "music/PV_main-theme_bed_80bpm.wav",   34, { false } },
    { "music/PV_main-theme_layer1_80bpm.wav", 36, { false } },
    { "music/PV_main-theme_layer2_80bpm.wav", 36, { false } },
    { "music/PV_main-theme_layer3_80bpm.wav", 36, { false } },
};

// Volume curves extracted from Wwise RTPC bindings
// Format: {intensity_0_100, volume_dB}
static const music_curve_point_t MUSIC_LAYER_CURVES[][8] = {
    { {0, 0}, {100, 0} },                          // bed: always audible
    { {0, -200}, {20, -200}, {31, 0}, {100, 0} },  // layer1
    { {0, -200}, {45, -200}, {60, 0}, {100, 0} },  // layer2
    { {0, -200}, {59, -200}, {80, 0}, {100, 0} },  // layer3
};
```

**Note:** The curves in the code are simplified approximations of the exact
Wwise values (rounded to clean integers). The Wwise curves have fractional
values like `20.506` and `31.078` — we round these for simplicity. See
[Tradeoffs](#tradeoffs-and-limitations) for impact analysis.

### Why str_t Uses Brace Initializers

The `str()` macro produces a compound literal, which is **not a constant
expression in C** (only in C++). MSVC rejects:
```c
static const str_t x = str("hello");  // ERROR: not a constant
```
So we use direct struct initialization:
```c
static const str_t x = { "hello", 5, { false } };
```
This is a C11-compliant constant initializer.

---

## File Reference

### Poglib (Engine Library)

| File | Purpose |
|------|---------|
| `application/audio_device.h` | Initializes and shuts down the miniaudio engine. Provides `audio_device_get_engine()` to access the `ma_engine*`. |
| `application/audio_music.h` | The layered music system. Loads WAV files as streaming `ma_sound` instances, applies intensity-driven volume curves, handles play/stop/fade. |
| `application/audio_sfx.h` | One-shot sound effect system. Loads non-looping `ma_sound` instances, supports sound sets with round-robin playback. |
| `application/music_config.h` | Wwise-derived configuration. Contains hardcoded file paths, curve tables, and the `music_config_load()` / `music_config_play_impact()` API. |
| `application.h` | Wires `audio_device_init()` and `audio_device_shutdown()` into the application lifecycle. |
| `external/miniaudio.h` | Vendored miniaudio v0.11.25 — single-header audio library providing cross-platform audio I/O, mixing, and effects. |

### Vedanta (Game)

| File | Purpose |
|------|---------|
| `src/game.h` | Defines `parkour_compute_music_intensity()` which maps FSM states to intensity values (0-100). |
| `src/scenes/collision-scene.h` | Calls `music_config_load()` + `music_theme_play()` on init, feeds intensity each frame, calls `music_theme_stop()` + `music_config_unload()` on destroy. |
| `src/scenes/collision_scene/state/vault.h` | Calls `music_config_play_impact()` when a vault action starts. |

---

## How the Music Layers Work

### Loading

When `music_config_load()` is called:

1. The miniaudio engine is initialized (if not already) — stereo, 48 kHz
2. For each of the 4 layers:
   - The full file path is constructed: `res/audio/music/PV_main-theme_*.wav`
   - The WAV is loaded as a **streaming** sound (`MA_SOUND_FLAG_STREAM`) — this
     means the file is read from disk in chunks, not loaded entirely into RAM
   - Looping is enabled (`ma_sound_set_looping`)
   - The volume curve points are copied into the layer struct
3. The initial intensity (0) is applied, setting volumes via the curves

### Playback

When `music_theme_play()` is called:
- All loaded layers start playing simultaneously via `ma_sound_start()`
- They are **not** phase-locked — they start from their current play position.
  If loaded at the same time and started at the same time, they will be roughly
  synced because they share the same tempo and were exported from Wwise at the
  same bar position.

### Volume Control

Each frame, `music_theme_update(dt)` is called:
1. The current intensity smoothly interpolates toward the target:
   ```c
   f32 diff = target - current;
   current += diff * smoothing_speed * dt;
   ```
   The smoothing speed is 5.0 (hardcoded). This means the intensity takes
   roughly 0.2 seconds to reach 63% of the way to the target.

2. For each layer, the intensity is evaluated against the layer's curve to
   produce a dB value.

3. The dB value is converted to a linear gain:
   ```c
   gain = pow(10.0, dB / 20.0);
   ```
   Values at or below -200 dB are treated as fully silent (gain = 0.0).

4. The gain is applied via `ma_sound_set_volume()`.

### Stopping

`music_theme_stop(fade_sec)` supports two modes:
- **Immediate** (`fade_sec <= 0`): all layers stop instantly
- **Fade-out** (`fade_sec > 0`): each layer fades from its current volume to
  silence over the specified duration using `ma_sound_set_fade_in_milliseconds()`,
  then stops. The `-1.0f` first argument means "start from current volume".

---

## How the RTPC Intensity System Works

### In Wwise

`Parkour_intensity` is a Game Parameter (RTPC) that the game code sets each
frame. Each music track has an RTPC binding that maps this parameter to the
track's Volume property (in dB). The curves are piecewise-linear with configurable
point positions.

### In Our Implementation

We replicate this with:

1. **`music_curve_point_t`** — a simple `{x, y}` pair where x = intensity
   (0-100) and y = volume in dB.

2. **`music__internal__db_from_curve()`** — performs piecewise linear
   interpolation across the curve points. Given an intensity value, it finds the
   two surrounding points and linearly interpolates the dB value between them.

3. **`music__internal__db_to_linear()`** — converts dB to a linear gain factor
   suitable for `ma_sound_set_volume()`. Values at or below -200 dB return 0.0
   (effectively mute).

### Intensity Values per Parkour State

Defined in `parkour_compute_music_intensity()` in `game.h`:

| FSM State | Intensity | Layers Active |
|-----------|-----------|---------------|
| Idle | 5 | bed only |
| Walk | 10 | bed only |
| Run | 40 | bed only (layer1 nearly silent) |
| Slide | 60 | bed + layer1 + layer2 |
| Vault | 65 | bed + layer1 + layer2 |
| Climb | 60 | bed + layer1 + layer2 |
| Double Jump | 75 | bed + layer1 + layer2 + layer3 |
| Falling | 70 | bed + layer1 + layer2 + layer3 |
| Wall Run | 85 | bed + layer1 + layer2 + layer3 (full) |

The intensity value is fed to the music system every frame via:
```c
music_theme_set_intensity(parkour_compute_music_intensity(top->state_type));
```

---

## How Impact Stingers Work

### Wwise Design

The `Stinger_impacts` music segment in Wwise contains a **Random Step** track
that randomly selects from 4 impact sounds (Impact_KL_01 through _04). Some
impacts have a Wwise Pitch Shifter effect applied (-100 cents, i.e., down one
semitone).

### Our Implementation

We use the `audio_sfx.h` system:

1. **Registration**: All 4 impact WAVs are loaded as non-looping `ma_sound`
   instances via `sfx_register()`.

2. **Set creation**: They are grouped into an "impacts" set via
   `sfx_set_create()` and `sfx_set_add()`.

3. **Playback**: `sfx_play_from_set()` uses **round-robin** selection — it
   plays the next sound in the set, wrapping around. This approximates Wwise's
   random selection but is deterministic and simpler.

4. **Trigger**: `music_config_play_impact()` is called from `vault.h` when the
   player starts a vault animation:
   ```c
   void ba__state_vault_start(...) {
       glmodel_set_animation(sys->model, MODEL_ANIMATION_LABELS[MAS_VAULT]);
       music_config_play_impact();
   }
   ```

5. **Re-triggering**: `sfx_play()` seeks the sound back to the beginning before
   starting, so rapid triggers will restart the same sound (overlapping is not
   supported in v1).

---

## Game Integration (Vedanta)

### Scene Lifecycle

```
collision_scene_init()
  ├── music_config_load()    → loads all WAVs + curves
  └── music_theme_play()     → starts all layers

collision_scene_update(dt)
  ├── Read current FSM state from behavior automata
  ├── music_theme_set_intensity(parkour_compute_music_intensity(state))
  └── music_theme_update(dt)  → smooths intensity, applies volumes

collision_scene_destroy()
  ├── music_theme_stop(0.5f) → 500ms fade-out
  └── music_config_unload()  → frees all sounds + resets state
```

### Parkour FSM → Music Intensity

The game uses a behavior automata (state machine) for parkour movement. The top
state on the stack determines the current movement state. This is mapped to an
intensity value by `parkour_compute_music_intensity()`.

The mapping was designed so that:
- Low-intensity actions (idle, walk) keep the bed layer soloed
- Medium-intensity actions (run) begin hinting at layer1
- High-intensity parkour (wallrun, vault, double jump) bring in all layers
- The intensity changes are smoothed over ~200ms to avoid abrupt volume jumps

---

## Runtime Data Flow

```
 Every Frame (collision_scene_update)
 ─────────────────────────────────────
 ┌──────────────┐
 │  FSM State   │ → parkour_compute_music_intensity()
 │  (top of     │    returns float 0..100
 │   stack)     │
 └──────┬───────┘
        │
        ▼
 ┌──────────────────────┐
 │ music_theme_set_     │ → stores as intensity_target
 │ intensity(value)     │
 └──────┬───────────────┘
        │
        ▼
 ┌──────────────────────┐
 │ music_theme_update   │ → smooths: current += (target - current) * 5.0 * dt
 │ (dt)                 │
 └──────┬───────────────┘
        │
        ▼
 ┌──────────────────────┐
 │ For each layer:      │
 │  1. Evaluate curve   │ → db = piecewise_lerp(curve, current_intensity)
 │  2. Convert dB→gain  │ → gain = pow(10, db/20)
 │  3. Set volume       │ → ma_sound_set_volume(&layer.sound, gain)
 └──────────────────────┘
```

---

## Building and Running

### Prerequisites

- MSVC (Visual Studio 2022) with `cl` compiler
- Dependencies in `lib/`: SDL2, GLEW, GLFW, FREETYPE, Jolt, Assimp
- miniaudio.h vendored at `external/miniaudio.h`

### Build

From the vedanta worktree root:
```bat
build.bat
```

This compiles a single translation unit (`src/main.c`) which includes
`application.h`, which in turn includes all audio headers. The miniaudio
implementation is compiled inline via `MINIAUDIO_IMPLEMENTATION` in
`audio_device.h`.

### Audio Assets

WAV files must be placed in:
```
res/audio/
├── music/
│   ├── PV_main-theme_bed_80bpm.wav
│   ├── PV_main-theme_layer1_80bpm.wav
│   ├── PV_main-theme_layer2_80bpm.wav
│   └── PV_main-theme_layer3_80bpm.wav
└── sfx/
    ├── Impact_KL_01.wav
    ├── Impact_KL_02.wav
    ├── Impact_KL_03.wav
    └── Impact_KL_04.wav
```

Source WAVs are at:
```
E:\dev\resources\Project Vedanta_theme\Project Vedanta_theme\Originals\SFX\
```

### Run

```bat
cd vedanta-audio
bin\vedanta.exe
```

---

## Tradeoffs and Limitations

### Tradeoffs Made

| Decision | Reason | Cost |
|----------|--------|------|
| **No Wwise SDK** | Avoids proprietary dependency, licensing, build complexity | Lose Wwise runtime features (effects, spatial audio, adaptive filtering) |
| **WAV streaming from disk** | Low RAM usage, simple to implement | Disk I/O during gameplay (negligible for 4 files on SSD) |
| **Hardcoded curves in C** | Zero runtime parsing, compile-time correctness | Must manually update `music_config.h` when Wwise curves change |
| **Piecewise linear interpolation** | Matches Wwise's default curve type | Wwise supports log, S-curve, etc. — we only do linear |
| **Round-robin stinger selection** | Simple, deterministic | Wwise uses weighted random selection with avoid-repeat — we don't |
| **No Wwise effects** | miniaudio doesn't have Wwise's DSP plugins | Impact_KL_02 and _03 lose their Pitch Shifter (-100 cents) effect |
| **Simplified curve points** | Clean integer values for readability | Slight difference from exact Wwise values (e.g., 20.506 → 20) |
| **str_t brace initialization** | MSVC C11 compliance (compound literals aren't constant expressions) | Less readable than `str()` macro |
| **Single global music state** | Matches the single Music_Khushi container in Wwise | Can't play multiple themes simultaneously |
| **No phase locking** | Simplified implementation | Layers may drift slightly over time if loop lengths differ |

### Limitations

1. **No Wwise effects pipeline.** Wwise applies Pitch Shifter to some impact
   stingers and RoomVerb to layer3. These are lost in the miniaudio runtime.
   miniaudio supports basic effects (low-pass, high-pass, biquad) but not
   Wwise-specific plugins.

2. **Layer sync is approximate.** The 4 WAV files have slightly different
   durations (84,000 to 95,358 ms). When looped, they will slowly drift out of
   sync. Wwise handles this with its music engine's quantization and sync
   system. Our implementation just starts them simultaneously — they stay
   roughly synced for the duration of a play session but may drift over very
   long sessions.

3. **Curve values are approximations.** The Wwise RTPC curves have fractional
   control points (e.g., `20.50639`). We round these to integers for
   readability. The audible difference is negligible (sub-dB, sub-intensity).

4. **No random stinger selection.** Wwise's Stinger_impacts uses a Random Step
   container with weighted selection and avoid-repeat. We use round-robin,
   which cycles through impacts in order. This is less musically varied.

5. **No crossfade between layers.** When intensity drops, layers mute
   immediately at the curve threshold. Wwise can use crossfade regions and
   transition rules. Our linear interpolation provides smooth volume changes
   but no overlap/blending between arrangement states.

6. **No adaptive arrangement.** Wwise can switch between different music
   segments, playlists, and transitions based on states. We have a single
   layered segment that plays continuously.

7. **Single RTPC only.** The Wwise project defines `Player_health` as a second
   game parameter, but it's not wired to any music track curves in the current
   design. Our implementation only supports one intensity parameter.

8. **No bus routing or submixing.** All layers play through miniaudio's default
   output. Wwise routes through a Main Audio Bus with per-bus effects and
   volume控制.

9. **`res/audio/` WAV files are gitignored.** The WAVs are not committed to the
   repository. They must be manually copied from the Wwise export directory
   after cloning.

### What's Preserved from Wwise

- The layered music structure (bed + 3 stems)
- The RTPC curve behavior (intensity → volume mapping)
- The 80 BPM tempo alignment
- The impact stinger concept (4 sounds, played on vault)
- The state-based play/stop lifecycle

### What's Different from Wwise

- No runtime SoundBank loading
- No Wwise-specific DSP effects
- No weighted random stinger selection
- No quantized layer transitions
- No multi-RTPC support
- No spatial audio or bus routing
- No music scene graph (switch containers, playlist containers)

---

## Future Improvements

1. **Add low-pass filter sweep.** miniaudio supports biquad filters. Could
   approximate Wwise's highpass/lowpass curves on layer entry/exit.

2. **Implement weighted random stinger selection.** Add weights to `sfx_set_t`
   and randomize selection instead of round-robin.

3. **Phase-lock layers.** Use `ma_sound_get_cursor_in_pcm_frames()` to detect
   when each layer reaches its loop point, and re-sync them periodically.

4. **Add Player_health RTPC.** Wire the second game parameter to a filter or
   volume curve if the game design requires it.

5. **Parse .wwu files at build time.** A build script could extract curve data
   from the Wwise XML and auto-generate `music_config.h`, keeping it perfectly
   in sync with the Wwise project.

6. **Per-layer volume offsets.** Allow the game to adjust individual layer
   volumes for gameplay effects (e.g., ducking the bed during dialogue).

---

## Troubleshooting

| Symptom | Cause | Fix |
|---------|-------|-----|
| `music: failed to load ... : -7` | WAV file not found at expected path | Ensure WAVs are in `res/audio/music/` and `res/audio/sfx/` |
| No audio plays | miniaudio engine failed to init | Check `audio_device_init()` return value; ensure audio device is available |
| Build error: `syntax error: 'constant'` on miniaudio WORD types | `#define WORD 512` from `common.h` collides with Windows SDK `WORD` type | Already fixed — `WORD` macro was removed from poglib |
| Build error: `initializer is not a constant` | Using `str()` macro in file-scope `static const` | Use brace initialization: `{ "string", len, { false } }` |
| Layers sound out of sync | Loop durations differ between tracks | This is expected — see Limitations. Reload music to resync. |
| Stingers sound different from Wwise | Missing Pitch Shifter and RoomVerb effects | Add miniaudio biquad/lowpass effects to approximate |
