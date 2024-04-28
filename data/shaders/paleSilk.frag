// Pale Silk
// Author: Mateo Vallejo
// License: CC BY-NC 4.0

#version 330 core
uniform float time;
uniform vec4 audio;
uniform vec4 audioInc;
uniform vec2 resolution;
uniform sampler2D tex;
layout(location = 0) out vec4 outCol;

void main(void) {
	vec2 uv = gl_FragCoord.xy/resolution.y;
	uv.y = 1.-uv.y;
	float t = 0.3*time;
	uv.y += 0.03*sin(8.0*uv.x-t);
	float d1 = cos(3.0*uv.x+5.0*uv.y);
	float d2 = sin(20.0*(uv.x+uv.y-0.1*t));
	float f = 0.6+0.4*sin(5.0*(uv.x+uv.y+d1+0.02*t)+d2);
	outCol = vec4(0.9,0.95,1,1)*vec4(f);
}
