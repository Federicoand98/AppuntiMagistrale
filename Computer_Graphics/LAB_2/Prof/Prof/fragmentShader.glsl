#version 400 core

// Interpolated values from the vertex shaders
in vec4 ourColor;

out vec4 color;

void main(){


	color = ourColor;

}