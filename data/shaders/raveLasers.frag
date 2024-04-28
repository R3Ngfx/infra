// Rave Lasers
// Author: Mateo Vallejo
// License: CC BY-NC 4.0

#version 330 core
uniform float time;
uniform vec4 audio;
uniform vec4 audioInc;
uniform vec2 resolution;
uniform sampler2D tex;
layout(location = 0) out vec4 outCol;

// Random [0,1]
float rand(vec2 p) {
	p *= 500.0;
	vec3 p3  = fract(vec3(p.xyx) * .1031);
	p3 += dot(p3, p3.yzx + 33.33);
	return fract((p3.x + p3.y) * p3.z);
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
	for (int i = 0; i < 8; i++) {
		r += a*noise(p);
		a *= 0.5;
		p *= 2.0;
	}
	return r;
}

// Lasers originating from a central point
float laser(vec2 p, int num) {
	float r = atan(p.x, p.y);
	float sn = sin(r*float(num)+time);
	float lzr = 0.5+0.5*sn;
	lzr = lzr*lzr*lzr*lzr*lzr;
	float glow = pow(clamp(sn, 0.0, 1.0),100.0);
	return lzr+glow;
}

// Mix of fractal noises to simulate fog
float clouds(vec2 uv) {
	vec2 t = vec2(0,time);
	float c1 = fbm(fbm(uv*3.0)*0.75+uv*3.0+t/3.0);
	float c2 = fbm(fbm(uv*2.0)*0.5+uv*7.0+t/3.0);
	float c3 = fbm(fbm(uv*10.0-t)*0.75+uv*5.0+t/6.0);
	float r = mix(c1, c2, c3*c3);
	return r*r;
}

void main(void) {
	vec2 uv = gl_FragCoord.xy/resolution.y;
	uv.y = 1.-uv.y;
	vec2 hs = resolution.xy/resolution.y*0.5;
	vec2 uvc = uv-hs;
	// Lasers
	float l = (1.0 + 3.0*noise(vec2(15.0-time)))
		* laser(vec2(uv.x+0.5, uv.y*(0.5 + 10.0*noise(vec2(time/5.0))) + 0.1), 15);
	l += fbm(vec2(2.0*time))
		* laser(vec2(hs.x-uvc.x-0.2, uv.y+0.1), 25);
	l += noise(vec2(time-73.0))
		* laser(vec2(uvc.x, 1.0-uv.y+0.5), 30);
	// Fog
	float c = clouds(uv);
	vec4 col = vec4(0, 1, 0, 1)*(uv.y*l+uv.y*uv.y)*c;
	outCol = pow(col, vec4(0.75));
}
