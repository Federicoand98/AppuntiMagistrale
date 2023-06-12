// Vertex shader: Phong shading
// ================
#version 450 core

// Input vertex data, different for all executions of this shader.
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 N;
out vec3 L;
out vec3 E;

// Values that stay constant for the whole mesh.
uniform mat4 P;
uniform mat4 V;
uniform mat4 M; // position*rotation*scaling

struct PointLight{
	vec3 position;
	vec3 color;
	float power;
 };
uniform PointLight light;


void main()
{
    gl_Position = P * V * M * vec4(aPos, 1.0);

	// Position in VCS
	vec4 eyePosition = V * M * vec4(aPos, 1.0);
	// LightPos in VCS
	vec4 eyeLightPos = V * vec4(light.position, 1.0);

	// Compute vectors E,L,N in VCS
	E = -eyePosition.xyz;
	L = (eyeLightPos - eyePosition).xyz;
	N = transpose(inverse(mat3(V * M))) * aNormal;
	
} 

// REMARK: Normal of the the vertex, in camera space
// Normal_cameraspace = ( V * M * vec4(vertexNormal_modelspace,0)).xyz; 
// Only correct if ModelMatrix does not scale the model ! Use its inverse transpose if not.
// Normal_cameraspace = transpose(inverse(mat3(V * M))) * aNormal; 