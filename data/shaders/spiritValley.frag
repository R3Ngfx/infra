// Spirit Valley
// Author: Mateo Vallejo
// License: CC BY-NC 4.0

#version 330 core
uniform float time;
uniform vec4 audio;
uniform vec4 audioInc;
uniform vec2 resolution;
uniform sampler2D tex;
layout(location = 0) out vec4 outCol;

#define oz vec2(1,0)
#define pi 3.14159
#define rot(t) mat2(cos(t), -sin(t), sin(t), cos(t))

float rand(vec2 p) {
	vec3 p3  = fract(vec3(p.xyx) * .1031);
	p3 += dot(p3, p3.yzx + 33.33);
	return fract((p3.x + p3.y) * p3.z);
}

vec2 randVec(vec2 p) {
	float r = rand(p);
	r *= 2.0*pi;
	return vec2(sin(r), cos(r));
}

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

float fbm(vec2 p) {
	float a = 0.5;
	float r = 0.0;
	for (int i = 0; i < 6; i++) {
		r += a*perlin(p);
		a *= 0.6;
		p *= 2.0;
	}
	return r;
}


float scene(vec3 p) {
	p.z = 0.01*p.z+time/10.0;
	float f = 0.5+0.5*fbm(p.zy*vec2(7,0.0175));
	//float f = texture(tex, p.zy*vec2(1,0.0025)).r;
	p.x += 2.5+2*sin(10*p.z)+10*sin(1*p.z);
	f = p.x < 0 ? f : 1-f;
	f = 20*f-0;
	return max(p.y, f-abs(p.x+0.1*p.y));
}

struct mr {
	int steps;
	float dist;
	vec3 pos;
};

mr march(vec3 p, vec3 dir) {
	float dist = 0;
	int i = 0;
	for (i = 0; i < 100; i++) {
		float step = 0.3*scene(p);
		p += dir * step;
		dist += step;
		if (dist > 200 || step < 0.01) break;
	}
	float d = scene(p);
	return mr(i, dist, p);
}

void main(void) {
	vec2 uv = gl_FragCoord.xy/resolution.y;
	uv.y = 1.-uv.y;
	uv -= resolution.xy/resolution.y/2.0f;
	vec3 cam = vec3(30*sin(time/10.0), 30+30*(0.5+0.5*sin(0.5*(time/20.0)+3.14159)), -60);
	vec3 f = normalize(vec3(0, -10, 0)-cam);
	vec3 r = cross(vec3(0, 1, 0), f);
	vec3 u = cross(f, r);
	vec3 ray = f*1 + r*uv.x + u*uv.y;
	mr m = march(cam, ray);
	float floorMask = smoothstep(200, 0, step(0, m.pos.y)*m.dist);
	float depths = smoothstep(-0, -250, m.pos.y);
	float mz = m.pos.z = 0.01*m.pos.z+time/10.0;
	float surface = smoothstep(2, 200, abs(m.pos.x+(2.5+2*sin(10*mz)+10*sin(1*mz))));
	surface *= 0.3*surface;
	float s = 5.*floorMask*mix(depths, surface, step(0, m.pos.y));
	outCol = pow(mix(vec4(0,0,0.01, 1), vec4(0.3,1,1, 1), vec4(s)), vec4(1./2.2));
}
