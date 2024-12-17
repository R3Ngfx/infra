// Light Tunnel
// Author: Mateo Vallejo
// License: CC BY-NC 4.0

#version 330 core
uniform float time;
uniform float audio[8];
uniform float audioInc[8];
uniform vec2 resolution;
layout(location = 0) out vec4 outCol;

#define pi 3.14159
#define oz vec2(1,0)
#define mx 0.707107

// Random normalized vector
vec2 randVec(vec2 p) {
	float r = fract(sin(dot(p, vec2(12.345, 741.85)))*4563.12);
	r *= 2.0*pi;
	r +=  time;
	return vec2(sin(r), cos(r));
}

// Seamless tiled perlin noise
float perlin(vec2 p, vec2 tiling) {
	vec2 f = fract(p);
	vec2 s = smoothstep(0.0, 1.0, f);
	vec2 i = floor(p);
	// Apply mod to vertex position to make it tileable
	float a = dot(randVec(mod(i,tiling)), f);
	float b = dot(randVec(mod(i+oz.xy,tiling)), f-oz.xy);
	float c = dot(randVec(mod(i+oz.yx,tiling)), f-oz.yx);
	float d = dot(randVec(mod(i+oz.xx,tiling)), f-oz.xx);
	return mix(mix(a, b, s.x), mix(c, d, s.x), s.y);
}

// Fractal noise
float fbm(vec2 p, vec2 tiling) {
	float a = 0.5;
	float r = 0.0;
	for (int i = 0; i < 8; i++) {
		r += a*perlin(p, tiling);
		a *= 0.5;
		p *= 2.0;
		tiling *= 2;
	}
	return r;
}

void main(void) {
	vec2 uv = gl_FragCoord.xy/resolution.y;
	uv.y = 1.-uv.y;
	uv -= resolution.xy/resolution.y/2.0;
	// Tunnel coordinates
	vec2 cuv = vec2((atan(uv.x, uv.y)+pi)/(2.0*pi), 0.005/length(uv)+0.01*time);
	// Center highlight
	float hl = (1.0-length(uv));
	hl *= hl * hl;
	// Texture
	outCol = vec4(pow(0.9+0.5*fbm(20*cuv, vec2(20)), 10.0)+hl);
}
