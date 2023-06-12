#version 420 core

// Interpolated values from the vertex shaders

out vec4 color;

void main(){
//Viene assegnato lo stesso colore ad ogni pixel
	color = vec4(1.0,0.0,0.0,1.0);
}