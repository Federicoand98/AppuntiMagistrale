#version 450 core

// Input vertex data, different for all executions of this shader.
layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 TexCoord;

// Output data ; will be interpolated for each fragment.
out vec2 _TexCoord;

// Values that stay constant for the whole mesh.
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform vec3 camera_position;

void main(){

	// Output position of the vertex, in clip space : MVP * position
	gl_Position =  P * V * M * vec4(vertex,1);

	// we pass the texture coordinate to the fragment shader
	_TexCoord = TexCoord;
}
