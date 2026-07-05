# Wwise Audio Integration Analysis

## Overview

This document records the analysis of an existing Wwise project
(`ExplorationSystem_ASK`, created in Wwise v2025.1.6) and proposes an
integration strategy for poglib that avoids the Wwise SDK at runtime.

The core idea: **treat Wwise as a composition/design tool only**, parse its
XML-based project files (`.wproj` / `.wwu`) to understand the arrangement,
and drive playback through a lightweight SDL2 multi-track mixer built
directly into poglib.

---

## Wwise Project Structure

### Topology

```
ExplorationSystem_ASK.wproj
├── Platforms: Mac, Windows
├── Conversion: Default (PCM 48kHz, 16-bit)
├── Busses: Main Audio Bus (System device)
│
└── Containers/Default Work Unit.wwu
    └── _source/
        └── Team 3 - AMBIENCE ASSETS/
            ├── Groove (MusicSegment, 66 BPM)
            ├── Pads   (MusicSegment, 66 BPM)
            ├── Shimmer(MusicSegment, 66 BPM)
            └── Melody (MusicSegment, 66 BPM)
    └── Ambience_system (MusicPlaylistContainer)
        ├── Intro (MusicSegment)
        │   └── Track: SS_Intro_1 → SS_Intro_1.wav
        └── Ambience Main (MusicSegment)
            ├── Bass track   → 6 clips (A/D/E notes)
            ├── Groove track → 3 clips (SS_Groove1-3)
            ├── Shimmer track→ 5 clips (KL + SS)
            ├── Pads track   → 2 clips (AM_Pad_01-02)
            ├── Melody track → 4 clips (AM_Melody_01-04, -7dB)
            └── (empty tracks for unused variations)
        └── Impact (MusicSegment, stinger)
            └── Track: "New Random Music Track"
                └── 4 clips (Impact_KL_01-04)
```

### Audio Assets

All source WAVs live in `Originals/SFX/Team 3 - AMBIENCE ASSETS/`:

| Category | Files | Type |
|---|---|---|
| Bass | SS_BassA_1/2, SS_BassD_1/2, SS_BassE_1/2 | Looping stems (keys A, D, E) |
| Groove | SS_Groove1/2/3 | Rhythmic percussion loops |
| Shimmer | Shimmer_KL_01-04, Shimmer_SS_01 | Atmospheric texture layers |
| Pads | AM_Pad_01, AM_Pad_02 | Pad/synth layers |
| Melody | AM_Melody_01-04 | Melodic phrases |
| Impact | Impact_KL_01-04 | Short percussive hits (stingers) |
| Intro | SS_Intro_1 | Opening sequence |

### Playlist Flow (from XML)

```
Ambience_system (MusicPlaylistContainer)
  │
  ├── Playlist Item 0: Intro ───────────── plays once
  │     └── Transition: 3s source fade-out
  │                      3s destination fade-in (offset -3s)
  │
  └── Playlist Item 1: Ambience Main ───── loops infinitely (LoopCount=0)
        └── 6 simultaneous tracks, each with staggered clips
```

### Key XML Tags Parsed

The `.wwu` files use a well-structured XML schema. The relevant tags for
runtime reconstruction:

| Tag | Data Extracted |
|---|---|
| `<MusicPlaylistContainer>` | Name, playlist structure |
| `<MusicPlaylistItem>` | Segment reference, loop count |
| `<MusicSegment>` | Name, tempo (BPM), end position (ms) |
| `<MusicTrack>` | Name, volume offset (dB) |
| `<AudioFileSource>` | WAV file path (relative to `Originals/`) |
| `<MusicTrackSequence>` | Groups clips into variations |
| `<MusicClip>` | PlayAt, trim begin/end, fade in/out durations |
| `<PropertyCurve>` (`<Point>`) | Automation: Volume, Highpass, Lowpass curves |
| `<MusicTransition>` | Crossfade durations between segments |
| `<MusicStinger>` | Segment + trigger for one-shot impacts |

---

## Current Audio State in Poglib

**File:** `application/sound.h` (72 lines)

- Single-header wrapper around SDL2 audio
- `SDL_LoadWAV` → `SDL_QueueAudio` model (push-based, no callback)
- Single stream only — no mixing, no multi-track
- No volume abstraction (volume change iterates the raw buffer destructively)
- No 3D audio, no spatialization, no streaming
- No ECS components for audio

---

## Proposed Integration Architecture

### Data Flow

```
┌─────────────────────────────────────────────────────────────┐
│                    BUILD TIME (Wwise Authoring)              │
│                                                             │
│  Music Team edits in Wwise                                  │
│       │                                                     │
│       ├── Saves .wwu XML files (arrangement, timings, etc.) │
│       └── Saves .wav files in Originals/                    │
│                                                             │
└─────────────────────────────────────────────────────────────┘
         │                          │
         │ read at startup          │ read at startup
         ▼                          ▼
┌─────────────────────────────────────────────────────────────┐
│                    RUNTIME (poglib engine)                   │
│                                                             │
│  wwise_parser.h                                              │
│       │                                                     │
│       ├── Scans .wwu XML for segment/track/clip structure   │
│       ├── Resolves WAV paths from Originals/                │
│       └── Produces: wwise_project_t (arena-allocated)       │
│                                                             │
│  mixer.h                                                    │
│       │                                                     │
│       ├── mixer_init() → SDL_OpenAudioDevice (callback)     │
│       ├── mixer_add_stream() → loads WAV, assigns stream ID │
│       ├── mixer_stream_volume(id, v) → atomic volume set    │
│       └── SDL callback: sums active streams into output     │
│                                                             │
│  Game (app->update)                                         │
│       │                                                     │
│       └── mixer_stream_volume("bass",     progress(0.0))    │
│           mixer_stream_volume("groove",   progress(0.2))    │
│           mixer_stream_volume("shimmer",  progress(0.4))    │
│           mixer_stream_volume("pads",     progress(0.6))    │
│           mixer_stream_volume("melody",   progress(0.8))    │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### New Files

| File | Purpose |
|---|---|
| `audio/mixer.h` | SDL2 callback-based multi-track mixer. Per-stream volatile volume. |
| `audio/wwise_parser.h` | Minimal XML tag scanner that deserializes `.wwu` files into `wwise_project_t`. |

### Modified Files

| File | Change |
|---|---|
| `application.h` | Replace `#include "application/sound.h"` with new audio headers. Wire init/term into app lifecycle. |

### Mixer Design (mixer.h)

```c
typedef struct mixer_t mixer_t;

mixer_t mixer_init(void);
int     mixer_add_stream(mixer_t *m, const char *name,
                         const u8 *wav_data, u32 wav_size);
void    mixer_stream_volume(mixer_t *m, int stream, float volume);
void    mixer_play(mixer_t *m);
void    mixer_stop(mixer_t *m);
void    mixer_destroy(mixer_t *m);
```

- Uses `SDL_AudioCallback` (not `SDL_QueueAudio`) to mix all streams into
  the hardware buffer on demand
- Each stream has a `volatile float volume` — the game thread writes it,
  the audio callback reads it (lock-free for a single float)
- Tracks loop by resetting read position on buffer end

### Wwise Parser Design (wwise_parser.h)

- Not a full XML parser — a forward-only tag scanner that recognises ~10
  specific tag names
- Arena-allocated output structures so no malloc/free in hot paths
- Resolves WAV paths relative to the `Originals/` directory (configured in
  `.wproj`)

---

## Clip Timing — Two Approaches

The XML contains per-clip trim and offset data:

```xml
<MusicClip Name="SS_BassA_1">
  <Property Name="PlayAt" Value="-3636.36"/>
  <Property Name="BeginTrimOffset" Value="3636.36"/>
  <Property Name="EndTrimOffset" Value="62131.67"/>
  <Property Name="FadeInDuration" Value="7196.04"/>
  <Property Name="FadeOutDuration" Value="5440.91"/>
</MusicClip>
```

**Approach A (recommended for progression-based music):**
Ignore clip-level trimming. Load each stem WAV as a full continuous loop.
Fade entire stems in/out based on game state (parkour performance). This is
simpler and maps directly to the game's needs.

**Approach B (faithful reproduction):**
Reproduce exact clip arrangement in the mixer — schedule clips at their
`PlayAt` offsets, trim to clip boundaries, apply fade curves. This matches
what Wwise does internally but adds complexity.

---

## Integration Sequence

| Step | File | Description |
|---|---|---|
| 1 | `audio/mixer.h` | Build SDL2 multi-track mixer (callback-based, per-stream volume) |
| 2 | `audio/wwise_parser.h` | Build minimal XML scanner + Wwise schema deserializer |
| 3 | `application.h` | Wire mixer init/term into app lifecycle |
| 4 | Game side | In `app->update`, drive stream volumes from gameplay |

---

## Open Questions

1. **Clip timing fidelity** — reproduce exact Wwise clip arrangement or treat
   stems as continuous loops? Current recommendation: continuous loops for
   parkour progression model.
2. **Hot-reload** — monitor file modification times on `.wwu` / `.wav` for
   live updates during development, or require restart?
3. **ECS components** — add `ECS_CMP_AUDIO_EMITTER` / `ECS_CMP_AUDIO_LISTENER`
   later for 3D positional audio, or keep the mixer standalone?
4. **One-shot SFX** — should Impact stingers and other short SFX go through
   the same mixer or a separate channel?
5. **Project location** — should the Wwise project files live inside the
   poglib repo, or should the engine accept a configurable path to the
   Wwise project directory?
