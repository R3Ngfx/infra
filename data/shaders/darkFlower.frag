// Dark Flower
// Author: Mateo Vallejo
// License: CC BY-NC 4.0

#version 330 core
uniform float time;
uniform vec4 audio;
uniform vec4 audioInc;
uniform vec2 resolution;
uniform sampler2D tex;
layout(location = 0) out vec4 outCol;

#define sstep(x,t) smoothstep(1,-1,(x-t)/fwidth(x))

// Random [0,1]
float hash12(vec2 p) {
	p *= 1500;
	vec3 p3  = fract(vec3(p.xyx) * .1031);
	p3 += dot(p3, p3.yzx + 33.33);
	return fract((p3.x + p3.y) * p3.z);
}

// Value noise
float noise(vec2 p){
	vec2 f = smoothstep(0,1,fract(p));
	vec2 i = floor(p);
	float a = hash12(i);
	float b = hash12(i+vec2(1,0));
	float c = hash12(i+vec2(0,1));
	float d = hash12(i+vec2(1,1));
	return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

void main(void) {
	vec2 uv = gl_FragCoord.xy/resolution.y;
	uv.y = 1.-uv.y;
	// Polar coordinates
	vec2 uvc = uv-vec2(resolution.x/resolution.y,1)/2;
	vec2 fuv = vec2(length(uvc)/1.25, atan(uvc.x, uvc.y)/3.14159);
	fuv.y -= 1.0/12;
	// Vignette
	float l = pow(1-length(uvc),2)*max(0.75,1.5*audio.y);
	// Flower
	float d = 2*noise((time/16+audioInc.y/10)+sin(3*fuv*3.14159*2));
	float f = fuv.x*10-(time/24+audioInc.y/15)+d;
	outCol = l*vec4(sstep(abs(fract(f)-0.5), 0.05));
}
