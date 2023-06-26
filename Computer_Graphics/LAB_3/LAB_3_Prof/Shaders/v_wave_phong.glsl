// Vertex shader: Phong shading
// ================
#version 450 core

// Input vertex data, different for all executions of this shader.
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 N;
out vec3 L;
out vec3 E;

uniform float time;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M; // position*rotation*scaling

struct PointLight{
	vec3 position;
	vec3 color;
	float power;
 };
uniform PointLight light;

float a = 0.1;
float freq = 0.001;

void main()
{
    vec4 v = vec4(aPos, 1.0);
    v.y = a * sin(freq * time + 10 * v.x) * sin(freq * time + 10 * v.z);
    gl_Position = P * V * M * v;

	// Position in VCS
	vec4 eyePosition = V * M * vec4(aPos, 1.0);
	// LightPos in VCS
	vec4 eyeLightPos = V * vec4(light.position, 1.0);

	// Compute vectors E,L,N in VCS
	E = -eyePosition.xyz;
	L = (eyeLightPos - eyePosition).xyz;
	N = transpose(inverse(mat3(V * M))) * aNormal;
	
} 