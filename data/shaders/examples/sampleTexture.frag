// Example - Sampling a texture
// License: MIT

#version 330 core
uniform float time;
uniform vec4 audio;
uniform vec4 audioInc;
uniform vec2 resolution;
uniform sampler2D tex0;
layout(location = 0) out vec4 outCol;

void main(void) {
	vec2 uv = gl_FragCoord.xy/resolution.y;
	uv.y = 1.-uv.y;
	outCol = texture(tex0, uv+time);
}
