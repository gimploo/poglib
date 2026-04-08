#pragma once

#define COLOR_BLACK     (vec4f_t ){0.0f, 0.0f, 0.0f ,1.0f}
#define COLOR_RED       (vec4f_t ){1.0f, 0.0f, 0.0f, 1.0f}
#define COLOR_BLUE      (vec4f_t ){0.0f, 0.0f, 1.0f, 1.0f}
#define COLOR_GREEN     (vec4f_t ){0.0f, 1.0f, 0.0f, 1.0f}
#define COLOR_WHITE     (vec4f_t ){1.0f, 1.0f, 1.0f, 1.0f}
#define COLOR_OFFWHITE  (vec4f_t){0.8f, 0.8f, 0.8f, 1.0f}
#define COLOR_NOT_AS_BRIGHT_AS_WHITE     (vec4f_t ){0.9f, 0.9f, 0.9f, 1.0f}
#define COLOR_GRAY      (vec4f_t ){0.4f, 0.4f, 0.4f ,1.0f}
#define COLOR_DARK_GRAY (vec4f_t ){0.2f, 0.2f, 0.2f, 1.0f}
#define COLOR_NEUTRAL   COLOR_WHITE

// --- Debug Palette (High Visibility) ---
#define COLOR_ORANGE  (vec4f_t){1.00f, 0.57f, 0.00f, 1.0f }
#define COLOR_MINT    (vec4f_t){0.00f, 1.00f, 0.62f, 1.0f }
#define COLOR_CYAN    (vec4f_t){0.00f, 0.90f, 1.00f, 1.0f }
#define COLOR_ABYSS   (vec4f_t){0.04f, 0.05f, 0.08f, 1.0f }

// Abyss Blue - Ideal for glClearColor
#define COLOR_ABYSS_BLUE    (vec4f_t){ 0.043f, 0.055f, 0.078f, 1.0f }

// Charcoal - Ideal for UI Panels/Backgrounds
#define COLOR_CHARCOAL      (vec4f_t){ 0.102f, 0.110f, 0.137f, 1.0f }

// Soft White - High readability for text
#define COLOR_SOFT_WHITE    (vec4f_t){ 0.878f, 0.878f, 0.878f, 1.0f }

// Crimson Red - Accents, health bars, or primary highlights
#define COLOR_CRIMSON       (vec4f_t){ 0.863f, 0.078f, 0.235f, 1.0f }

// --- Utility Colors ---
#define COLOR_TRANSPARENT   (vec4f_t){ 0.000f, 0.000f, 0.000f, 0.00f }
#define COLOR_ERROR_MAGENTA (vec4f_t){ 1.000f, 0.000f, 1.000f, 1.0f }



