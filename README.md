# Infra
Infra is a tool for creating textures and videos by exporting the output of GLSL fragment shaders.

![infra](https://github.com/user-attachments/assets/4e79cac6-e28a-44e6-ac74-9739c6ce86e3)

## Usage
With Infra you can load and visualize fragment shaders, adjust the export parameters and render video sequences or individual frames.
Infra can be used to create images, textures, animations and even audio visualizers and music videos with a single fragment shader.
Check the *INFO* tab in the application for more details about its usage.

### Shader syntax
Infra uses GLSL shaders. Five different variables are exposed and can be used in the shader:
- `time` *(float)*: Current time in seconds.
- `resolution` *(vec2)*: Horizontal and vertical resolution of the rendered image.
- `audio` *float[8]*: Values of audio visualization.
    - `audio[0]`: Sub Bass (20Hz-60Hz).
    - `audio[1]`: Bass (60Hz-250Hz).
    - `audio[2]`: Low Midrange (250Hz-500Hz).
    - `audio[3]`: Midrange (500Hz-2000Hz).
    - `audio[4]`: Upper Midrange (2000Hz-4000Hz).
    - `audio[5]`: Presence (4000Hz-6000Hz).
    - `audio[6]`: Brilliance (6000Hz-20000Hz).
    - `audio[7]`: All (20Hz-20000Hz).
- `audioInc` *float[8]*: Incremental value of audio visualization. Indexed in the same way as with `audio`.
- `tex<NUM>` *(sampler2D)*: Up to 16 texture images can be loaded, with `<NUM>` being the number of the image.
    - `tex0`: First image
    - `tex1`: Second image
    - ...
    - `tex15`: Sixteenth image

You can find examples of working shaders under the `data` directory.

> *Note:* Infra's screen space coordinates are centered on the top left corner of the screen. Shaders ported from other programs might appear upside down, this can be fixed by flipping the y axis at the start of the shader `uv.y = 1-uv.y`.

## Dependencies
Included dependencies are located in the `src` directory.  
External dependencies must be installed separately.

### Included
- [Miniaudio](https://miniaud.io/) *(MIT License)*
- [Nuklear](https://github.com/Immediate-Mode-UI/Nuklear) *(MIT License)*
- [TinyDir](https://github.com/cxong/tinydir) *(Modified BSD License)*
- [stb](https://github.com/nothings/stb) *(MIT License)*

### External
- [SDL2](https://www.libsdl.org/) *(Zlib License)*
- [FFTW](http://www.fftw.org/) *(GPLv2)*
- [GLEW](https://github.com/nigels-com/glew) *(Modified BSD License)*
- [LIBAV](https://libav.org/) *(GPLv2)*

## License
Source code for Infra is provided under the [GNU General Public License Version 2](LICENSE).  
Original shaders are provided under the [Attribution-NonCommercial 4.0 International](https://creativecommons.org/licenses/by-nc/4.0/) license.
