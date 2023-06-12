#version 450 core

// Input vertex data, different for all executions of this shader.
layout (location = 0) in vec3 aPos;

out vec3 _3D_TexCoord;

// Values that stay constant for the whole mesh.
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform vec3 camera_position;


void main()
{
    // The texture is projected by the so no need of any UV mapping
    // Instead, passing the vertex position is enough
    _3D_TexCoord = aPos; 
/*
Usually we don't want the user to notice that the background is faked by a cube.
To not allow the viewer to go near the edges of the skybox,
 we remove the traslation part from the view matrix, 
 the one that lies in the fourth row.
*/
    mat4 static_View = mat4(mat3(V));
    gl_Position = P * static_View * M * vec4(aPos, 1.0);
}  