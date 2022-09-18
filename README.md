# Poglib
   * A header only C11 library for game and desktop development.
   * Multiplatorm i.e Windows and Unix (Android soon)

# Requirements
    For windows:    msvc compiler
    For unix:       gcc

# Usage
```bash
    ease --init helloworld --target win64
    cd helloworld
    forge run
```

# Dependencies
    SDL2 / GLFW :platform code
    OpenGL      :graphics api
    cglm        :math
    stb_image   :image processing
    freetype    :text processing
