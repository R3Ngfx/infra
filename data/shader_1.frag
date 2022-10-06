#version 330 core
uniform float time;
uniform vec2 resolution;
layout(location = 0) out vec4 outCol;

#define pi 3.14159
#define oz vec2(1,0)
#define tn 10.0
#define t vec2(5)
#define sat(x) clamp(x, 0.0, 1.0)
#define rot(a) mat2x2(cos(a), -sin(a), sin(a), cos(a))

// Random normalized vector
vec2 randVec(vec2 p) {
	float r = fract(sin(dot(p, vec2(12.345, 741.85)))*4563.12);
	r *= 2*pi;
	r += time;
	return vec2(sin(r), cos(r));
}

// Seamless tiled perlin noise
float perlin(vec2 p, float r) {
	vec2 f = fract(p);
	vec2 s = smoothstep(0.0, 1.0, f);
	vec2 i = floor(p);
	// Apply mod() to vertex position to make it tileable
	float a = dot(randVec(mod(i,t*r)), f);
	float b = dot(randVec(mod(i+oz.xy,t*r)), f-oz.xy);
	float c = dot(randVec(mod(i+oz.yx,t*r)), f-oz.yx);
	float d = dot(randVec(mod(i+oz.xx,t*r)), f-oz.xx);
	return mix(mix(a, b, s.x), mix(c, d, s.x), s.y);
}

// Fractal noise
float fbm(vec2 p) {
	float a = 0.5;
	float r = 0.0;
	for (int i = 0; i < 8; i++) {
		r += a*perlin(p, 1.0/a/2.0);
		a *= 0.5;
		p *= 2.0;
	}
	return r;
}

void main(void) {
	vec2 uv = gl_FragCoord.xy/resolution.xy;
	uv.y = 1-uv.y;
	outCol = vec4(smoothstep(-1, 1, perlin(t*uv, 1)));
	//float stp = floor(h*12.)/12.;
	//float frc = fract(h*12.);
	//outCol = vec4(smoothstep(0.1, 0, abs(frc-0.5))*stp);
}
