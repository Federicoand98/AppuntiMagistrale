#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 TexCoord;

out vec3 N;
out vec3 L;
out vec3 E;
out vec2 _TexCoord;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

struct PointLight {
    vec3 position;
    vec3 color;
    float power;
};

uniform PointLight light;

void main() {
    gl_Position = P * V * M * vec4(aPos, 1.0);

    // position in VCS
    vec4 eyePosition = V * M * vec4(aPos, 1.0);
    // Light pos in VCS
    vec4 eyeLightPos = V * vec4(light.position, 1.0);

    E = -eyePosition.xyz;
    L = (eyeLightPos - eyePosition).xyz;
    N = transpose(inverse(mat3(V * M))) * aNormal;

    _TexCoord = TexCoord;
}