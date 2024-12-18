// Bioscanner
// Author: Mateo Vallejo
// License: CC BY-NC 4.0

#version 330 core
uniform float time;
uniform float lows;
uniform float mids;
uniform float highs;
uniform vec2 resolution;
layout(location = 0) out vec4 outCol;

#define pi 3.14159
#define oz vec2(1,0)
#define tn 10.0
#define t vec2(20)
#define sat(x) clamp(x, 0.0, 1.0)
#define rot(a) mat2x2(cos(a), -sin(a), sin(a), cos(a))
#define sstep(x,t) smoothstep(1,-1,(x-t)/fwidth(x))

// random [0,1]
float hash12(vec2 p) {
	vec3 p3  = fract(vec3(p.xyx) * .1031);
	p3 += dot(p3, p3.yzx + 33.33);
	return fract((p3.x + p3.y) * p3.z);
}

// random normalized vector
vec2 randVec(vec2 p) {
	float r = hash12(p);
	r *= 2.0*pi;
	r += time;
	return vec2(sin(r), cos(r));
}

// perlin noise
float perlin(vec2 p) {
	vec2 f = fract(p);
	vec2 s = smoothstep(0.0, 1.0, f);
	vec2 i = floor(p);
	float a = dot(randVec(i), f);
	float b = dot(randVec(i+oz.xy), f-oz.xy);
	float c = dot(randVec(i+oz.yx), f-oz.yx);
	float d = dot(randVec(i+oz.xx), f-oz.xx);
	return mix(mix(a, b, s.x), mix(c, d, s.x), s.y)/0.707107;
}

// fractal noise
float fbm(vec2 p) {
	float a = 0.5;
	float r = 0.0;
	for (int i = 0; i < 8; i++) {
		r += a*perlin(p);
		a *= 0.5;
		p *= 2.0;
	}
	return r;
}

// 2D square sdf
float square(vec2 p, vec2 s) {
	vec2 d = abs(p)-s;
	return length(max(d, 0)) + min(0, max(d.x,d.y));
}

void main(void) {
	// coordinates
	vec2 uv = gl_FragCoord.xy/resolution.y;
	uv.y = 1.-uv.y;
	float ar = resolution.x/resolution.y;
	uv -= vec2(ar/2.0,0.5);
	float uxy = uv.x+uv.y;
	// distortion noise
	float n = fbm(vec2(10*uv.y, 3*time));
	float g = 0.2*n-0.6+sin(0.3*time);
	uv.x += smoothstep(0.15, 0, abs(g))*n;
	// wave
	float s = perlin(vec2(0.3*time));
	vec2 p = vec2(perlin(vec2(0.5*time)), 0.5*perlin(vec2(-time)))*0.5*(resolution.xy - vec2(300, 300))/resolution.y;
	p.y -= 0.1*sin(6*p.x+time);
	float a = (0.05+0.05*sin(time))+smoothstep(-0.5, -0.1, s)*smoothstep(mix(0.01, 0.9, smoothstep(-0.5, 0.5, s)), 0, length(uv-p));
	float r = 0;
	for (int i = 1; i <= 8; i++) {
		float f = uv.y;
		f += 0.2*sin(5*uv.x+time+a*sin(i+time)*1.25*cos(5*uv.x+0.5*i));
		r += 0.003/abs(f);
	}
	// ui
	float ob = square(uv, 0.5*vec2(0.9*ar,1-0.1*ar));
	float ui = 1.5*smoothstep(fwidth(uxy), -fwidth(uxy), abs(ob));
	float ms = step(ob, -0.005) * smoothstep(0.01, 0.005, abs(abs(uv.y)-0.475*(1-0.1*ar)));
	ms *= smoothstep(2*fwidth(uv.x), fwidth(uv.x), fract(uv.x*100)/100);
	ui = max(ui, ms);
	float cr = smoothstep(fwidth(uxy), 0, abs(min(abs(uv.x-p.x), abs(uv.y-p.y))));
	cr = max(cr, 3*smoothstep(fwidth(uxy), -fwidth(uxy), abs(square(uv-p, (0.1+smoothstep(-0.25, 0.5, s))*vec2(0.15)))));
	cr *= smoothstep(0, -0.1, ob);
	ui = max(ui, cr);
	r = max(r, 2*ui);
	r *= 2*pow(0.5+0.5*n, 2);
	r *= 0.5+0.5*hash12(gl_FragCoord.xy+time);
	r = clamp(r, 0, 1);
	outCol = vec4(mix(r, pow(1-r, 3), smoothstep(-0.025, 0.025, g)));
}

