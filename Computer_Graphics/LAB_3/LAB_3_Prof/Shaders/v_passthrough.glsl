// Vertex shader: Pass-through shading
// ================
#version 450 core

// Input vertex data, different for all executions of this shader.
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

// Values that stay constant for the whole mesh.
uniform mat4 P;
uniform mat4 V;
uniform mat4 M; // position*rotation*scaling

//uniform vec4 Color;
in vec4 Color;

void main()
{
    gl_Position = P * V * M * vec4(aPos, 1.0);
}