#version 400 core

uniform float time;
uniform vec2 resolution;

mat2 rotate(float r) {
    return mat2(cos(r), sin(r), -sin(r), cos(r));
}

void main() {
    vec2 coords = gl_FragCoord.xy - resolution * 1.;
    vec2 uv = (coords-.5 * resolution.xy) / resolution.y * 4.;
    float t = time;
    vec2 n = vec2(0);
    vec2 q = vec2(0);
    vec2 pos = uv;
    float d = 0.7;
    float size = 4.;
    float a = 0.0;
    mat2 rot = rotate(.1);

    for(float j = 0.; j < 20.; j++) {
        pos *= rot;
        q = pos * size + t * 1. + n;
        a += dot(sin(q) / size, vec2(.3));
        n -= cos(q);
        size *= 1.2;
    }

    vec3 col = vec3(5, 3, 1) * (a + .2) + a + a - d;

    gl_FragColor = vec4(vec3(0, 0, a + 0.2), 1.0);
}