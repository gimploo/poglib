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


