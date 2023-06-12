// Vertex shader: Normal mapping shading
// ================
#version 450 core

// Input vertex data, different for all executions of this shader.
layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 TexCoord;
layout (location = 3) in vec3 Tangent;

out vec2 _TexCoord;
out vec3 TangentLightPos;
out vec3 TangentViewPos;
out vec3 TangentFragPos;

// Values that stay constant for the whole mesh.
uniform mat4 P;
uniform mat4 V;
uniform mat4 M; // position*rotation*scaling
uniform vec3 camera_position;

struct PointLight{
	vec3 position;
	vec3 color;
	float power;
 };
uniform PointLight light;

void main()
{
    gl_Position = P * V * M * vec4(Position, 1.0);
	// we pass the texture coordinate to the fragment shader
	_TexCoord = TexCoord;
	
    mat3 normalMatrix = transpose(inverse(mat3(M)));
    vec3 T = normalize(normalMatrix * Tangent);
    vec3 N = normalize(normalMatrix * Normal);
    T = normalize(T - dot(T, N) * N);

    vec3 B = normalize(cross(N, T));

    mat3 TBNinv = transpose(mat3(T, B, N));    

	// We use the TBNinv matrix to transform world space vectors in tangent space
	TangentLightPos = TBNinv * light.position;
    TangentViewPos  = TBNinv * camera_position;
    TangentFragPos  = TBNinv * vec3( M * vec4(Position, 1.0));
} 