#version 450 core

out vec4 FragColor;

in vec3 _3D_TexCoord;

uniform samplerCube skybox;

void main()
{    
    // the samplerCube projects the fragment position 
    //and finds the right texel on the Cube texture
    FragColor = texture(skybox, _3D_TexCoord);
}