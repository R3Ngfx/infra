// Rainbow Storm
// Author: Mateo Vallejo
// License: CC BY-NC 4.0

#version 330 core
uniform float time;
uniform vec4 audio;
uniform vec4 audioInc;
uniform vec2 resolution;
uniform sampler2D tex;
layout(location = 0) out vec4 outCol;

#define pi 3.14159
#define oz vec2(1,0)
#define rot(t) mat2(cos(t), -sin(t), sin(t), cos(t))
#define sat(t) clamp(t, 0.0, 1.0)

// Color from hue
vec3 col(float t) {
	return 0.5+0.5*cos(2.0*pi*(t+vec3(0, 0.33, 0.67)));
}

// Random [0,1]
float hash12(vec2 p) {
	vec3 p3  = fract(vec3(p.xyx) * .1031);
	p3 += dot(p3, p3.yzx + 33.33);
	return fract((p3.x + p3.y) * p3.z);
}

// Random normalized vector
vec2 randVec(vec2 p) {
	float r = hash12(p);
	r *= 2.0*pi;
	r +=  time * 1.0; // Rotate gradient vector to simulate 3D noise
	return vec2(sin(r), cos(r));
}

// Perlin noise
float perlin(vec2 p) {
	vec2 f = fract(p);
	vec2 s = smoothstep(0.0, 1.0, f);
	vec2 i = floor(p);
	float a = dot(randVec(i), f);
	float b = dot(randVec(i+oz.xy), f-oz.xy);
	float c = dot(randVec(i+oz.yx), f-oz.yx);
	float d = dot(randVec(i+oz.xx), f-oz.xx);
	return mix(mix(a, b, s.x), mix(c, d, s.x), s.y);
}

// Fractal noise
float fbm(vec2 p) {
	float a = 0.5;
	float r = 0.0;
	for (int i = 0; i < 6; i++) {
		r += a*perlin(p);
		a *= 0.5;
		p *= 2.0;
	}
	return r;
}

// Main effect
vec4 eff(vec2 uv) {
	uv *= 10.0;
	uv *= rot(length(0.1*uv)*1.25);
	uv.y -= time*0.75;
	return vec4(col(3.0*fbm(uv/4.0+time*0.1+fbm(2.5*uv-time*0.1))), 1.0);
}

void main(void) {
	vec2 uv = gl_FragCoord.xy/resolution.y;
	uv.y = 1.-uv.y;
	uv -= resolution.xy/resolution.y/2;
	// Main effect with differing offsets for each color channel
	vec4 col = eff(uv);
	float aberration = 1.015 + 0.2*smoothstep(-1.0, 1.0, sin(0.25*time));
	col.r = eff(uv/aberration).r;
	col.b = eff(uv*aberration).b;
	outCol = col;
}
