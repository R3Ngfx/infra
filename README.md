# Infra
Infra is a tool for creating textures and videos by exporting the output of GLSL fragment shaders.

[screenshot]

## Usage
With Infra you can load and visualize fragment shaders, adjust the export parameters and render video sequences or individual frames.
Infra can be used for creating images, animations and even audio visualizers and music videos with a single fragment shader.
Check the *INFO* tab in the application for more details about its usage.

### Shader syntax
Infra uses GLSL shaders. Five different variables are exposed and can be used in the shader:
- `time` *(float)*: Current time in seconds.
- `resolution` *(vec2)*: Horizontal and vertical resolution of the rendered image.
- `lows` *(float)*: Value of low frequencies (20Hz-250Hz) for audio visualization.
- `mids` *(float)*: Value of mid frequencies (250Hz-2000Hz) for audio visualization.
- `highs` *(float)*: Value of high frequencies (2000Hz-20000Hz) for audio visualization.

You can find examples of working shaders under the `data` directory.

> *Note:* Infra's screen space coordinates are centered on the top left corner of the screen. Shaders ported from other programs might appear upside down, this can be fixed by flipping the y axis at the start of the shader `uv.y = 1-uv.y`.

## Dependencies
Included dependencies are located in the `src` directory.  
External dependencies must be installed separately.

### Included
- [Miniaudio](https://miniaud.io/) *(MIT License)*
- [Nuklear](https://github.com/Immediate-Mode-UI/Nuklear) *(MIT License)*
- [TinyDir](https://github.com/cxong/tinydir) *(Modified BSD License)*

### External
- [SDL2](https://www.libsdl.org/) *(Zlib License)*
- [FFTW](http://www.fftw.org/) *(GPLv2)*
- [GLEW](https://github.com/nigels-com/glew) *(Modified BSD License)*
- [LIBAV](https://libav.org/) *(GPLv2)*

## License
GNU General Public License Version 2.  
Read the full [license](LICENSE)
