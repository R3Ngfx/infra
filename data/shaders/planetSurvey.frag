// Planet Survey
// Author: Mateo Vallejo
// License: CC BY-NC 4.0

#version 330 core
uniform float time;
uniform vec4 audio;
uniform vec4 audioInc;
uniform vec2 resolution;
uniform sampler2D tex;
layout(location = 0) out vec4 outCol;

#define rot(t) mat2(cos(t), -sin(t), sin(t), cos(t))
#define MIN_DIST 0.01
#define MAX_DIST 125.0
#define MAX_STEPS 30

// Random [0,1]
float rand(vec2 p) {
	return fract(sin(dot(p, vec2(12.543,514.123)))*4732.12);
}

// Value noise
float noise(vec2 p) {
	vec2 f = smoothstep(0.0, 1.0, fract(p));
	vec2 i = floor(p);
	float a = rand(i);
	float b = rand(i+vec2(1.0,0.0));
	float c = rand(i+vec2(0.0,1.0));
	float d = rand(i+vec2(1.0,1.0));
	return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
	
}

// Fractal noise
float fbm(vec2 p) {
	float a = 0.5;
	float r = 0.0;
	for (int i = 0; i < 4; i++) {
		r += a*noise(p);
		a *= 0.5;
		p *= 2.0;
	}
	return r;
}

// Plane sdf with distortion
float scene(vec3 p) {
	float curvature = 0.005*(p.x*p.x+p.z*p.z);
	p.z += 0.5*time;
	float f = fbm(0.3*p.xz);
	return	p.y+6.0-3.0*f+curvature;
}

// Ray marching
float march(vec3 cam, vec3 ray) {
	float dist = 0.0;
	for (int i = 0; i < MAX_STEPS; i++) {
		vec3 p = cam+ray*dist;
		float d = scene(p);
		dist += d;
		if (d < MIN_DIST || dist > MAX_DIST) break;
	}
	return dist;
}

void main() {
	vec2 uv = gl_FragCoord.xy/resolution.y;
	uv.y = 1.-uv.y;
	uv -= resolution.xy/resolution.y/2.0;
	// Camera setup
	vec3 cam = vec3(0.0);
	vec3 ray = normalize(vec3(uv.x, uv.y, 0.75));
	float r = time/30.0;
	ray.xy *= rot(r);
	ray.yz *= rot(0.3);
	// Ray march
	float rm = march(cam, ray);
	// Lighting and color
	float d = clamp(rm/float(MAX_DIST/2.0), 0.0, 1.0);
	vec4 sky = mix(vec4(0.29, 0.28, 0.37, 1), vec4(0.11, 0.12, 0.17, 1), vec4(2.0*(uv*rot(r)).y));
	vec4 terrain = mix(vec4(0.54, 0.66, 1.00, 1), vec4(0.37, 0.28, 0.36, 1), vec4(pow(d, 0.5)));
	outCol = clamp(2.5*mix(terrain, sky, vec4(d*d))*d, vec4(0.0), vec4(1.0));
}
