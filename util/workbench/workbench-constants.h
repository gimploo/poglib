#pragma once
#include <poglib/basic.h>

f32 CAMERA_VERTICES [] = {
       // Main camera body (trapezoid shape - wider at back)
    // Front face (smaller)
    -0.3, -0.2, 0.2,    // 0: front bottom-left
     0.3, -0.2, 0.2,    // 1: front bottom-right
     0.3,  0.2, 0.2,    // 2: front top-right
    -0.3,  0.2, 0.2,    // 3: front top-left
    
    // Back face (larger - creates trapezoid)
    -0.4, -0.3, -0.3,   // 4: back bottom-left
     0.4, -0.3, -0.3,   // 5: back bottom-right
     0.4,  0.3, -0.3,   // 6: back top-right
    -0.4,  0.3, -0.3,   // 7: back top-left
    
    // Lens cylinder (8 sides for smooth look)
     0.0,  0.0, 0.2,    // 8: lens center back
     0.0,  0.0, 0.4,    // 9: lens center front
     0.15, 0.0, 0.2,    // 10: lens right back
     0.15, 0.0, 0.4,    // 11: lens right front
     0.11, 0.11, 0.2,   // 12: lens top-right back
     0.11, 0.11, 0.4,   // 13: lens top-right front
     0.0,  0.15, 0.2,   // 14: lens top back
     0.0,  0.15, 0.4,   // 15: lens top front
    -0.11, 0.11, 0.2,   // 16: lens top-left back
    -0.11, 0.11, 0.4,   // 17: lens top-left front
    -0.15, 0.0, 0.2,    // 18: lens left back
    -0.15, 0.0, 0.4,    // 19: lens left front
    -0.11,-0.11, 0.2,   // 20: lens bottom-left back
    -0.11,-0.11, 0.4,   // 21: lens bottom-left front
     0.0, -0.15, 0.2,   // 22: lens bottom back
     0.0, -0.15, 0.4,   // 23: lens bottom front
     0.11,-0.11, 0.2,   // 24: lens bottom-right back
     0.11,-0.11, 0.4,   // 25: lens bottom-right front
    
    // Viewfinder on top
    -0.1,  0.3, 0.0,    // 26: viewfinder bottom-left
     0.1,  0.3, 0.0,    // 27: viewfinder bottom-right
     0.1,  0.4, 0.0,    // 28: viewfinder top-right
    -0.1,  0.4, 0.0,    // 29: viewfinder top-left
};

// Indices for rendering triangles
u32 CAMERA_INDICES[] = {
       // Camera body faces (trapezoid)
    0, 1, 2,  2, 3, 0,    // front
    4, 7, 6,  6, 5, 4,    // back
    0, 4, 5,  5, 1, 0,    // bottom (trapezoid)
    2, 6, 7,  7, 3, 2,    // top (trapezoid)
    0, 3, 7,  7, 4, 0,    // left (trapezoid)
    1, 5, 6,  6, 2, 1,    // right (trapezoid)
    
    // Lens end caps
    8, 10, 12,  8, 12, 14,  8, 14, 16,  8, 16, 18,  // back cap
    8, 18, 20,  8, 20, 22,  8, 22, 24,  8, 24, 10,  // back cap continued
    9, 13, 11,  9, 15, 13,  9, 17, 15,  9, 19, 17,  // front cap
    9, 21, 19,  9, 23, 21,  9, 25, 23,  9, 11, 25,  // front cap continued
};

const f32 BULB_VERTICES[] = {
      // Base center + rim (0-8)
    0.0f, 0.0f, 0.0f,                      // 0 - base center
    0.2f, 0.0f, 0.0f,                      // 1
    0.14f, 0.0f, 0.14f,                    // 2
    0.0f, 0.0f, 0.2f,                      // 3
    -0.14f, 0.0f, 0.14f,                   // 4
    -0.2f, 0.0f, 0.0f,                     // 5
    -0.14f, 0.0f, -0.14f,                  // 6
    0.0f, 0.0f, -0.2f,                     // 7
    0.14f, 0.0f, -0.14f,                   // 8

    // Neck ring (9-16)
    0.2f, 0.1f, 0.0f,                      // 9
    0.14f, 0.1f, 0.14f,                    // 10
    0.0f, 0.1f, 0.2f,                      // 11
    -0.14f, 0.1f, 0.14f,                   // 12
    -0.2f, 0.1f, 0.0f,                     // 13
    -0.14f, 0.1f, -0.14f,                  // 14
    0.0f, 0.1f, -0.2f,                     // 15
    0.14f, 0.1f, -0.14f,                   // 16

    // Dome mid (17-24)
    0.0f, 0.5f, 0.5f,                      // 17
    0.5f, 0.5f, 0.0f,                      // 18
    0.0f, 0.5f, -0.5f,                     // 19
    -0.5f, 0.5f, 0.0f,                     // 20

    0.0f, 0.9f, 0.25f,                     // 21
    0.25f, 0.9f, 0.0f,                     // 22
    0.0f, 0.9f, -0.25f,                    // 23
    -0.25f, 0.9f, 0.0f,                    // 24

    0.0f, 1.0f, 0.0f                       // 25 - very top
};

const u32 BULB_INDICES[] = {
      // Base cap (0 as center)
    0, 1, 2,
    0, 2, 3,
    0, 3, 4,
    0, 4, 5,
    0, 5, 6,
    0, 6, 7,
    0, 7, 8,
    0, 8, 1,

    // Base to neck
    1, 2, 10,  1, 10, 9,
    2, 3, 11,  2, 11, 10,
    3, 4, 12,  3, 12, 11,
    4, 5, 13,  4, 13, 12,
    5, 6, 14,  5, 14, 13,
    6, 7, 15,  6, 15, 14,
    7, 8, 16,  7, 16, 15,
    8, 1, 9,   8, 9, 16,

    // Neck to dome mid
    9, 10, 18,
    10, 11, 17,
    11, 12, 17,
    12, 13, 20,
    13, 14, 20,
    14, 15, 19,
    15, 16, 19,
    16, 9, 18,

    // Dome mid to top ring
    17, 18, 22,
    17, 22, 21,
    18, 19, 23,
    18, 23, 22,
    19, 20, 24,
    19, 24, 23,
    20, 17, 21,
    20, 21, 24,

    // Top ring to very top
    21, 22, 25,
    22, 23, 25,
    23, 24, 25,
    24, 21, 25
};

