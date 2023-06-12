// Fragment shader: : Passthrough shading
// ================
#version 450 core

// Ouput data
out vec4 FragColor;

uniform vec4 Color;

void main()
{
   FragColor = Color;
}
