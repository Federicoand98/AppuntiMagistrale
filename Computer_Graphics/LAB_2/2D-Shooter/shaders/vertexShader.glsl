#version 400 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec4 Color;

out vec4 ourColor;

uniform mat4 Projection;
uniform mat4 Model;

void main(){	
	gl_Position = Projection * Model * vec4(aPos, 1.0);
	ourColor=Color;
}

