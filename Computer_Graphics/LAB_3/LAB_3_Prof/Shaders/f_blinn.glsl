// Fragment shader: Blinn shading
// ================
#version 450 core

in vec3 E;
in vec3 N;
in vec3 L;
in vec3 H;

// Ouput data
out vec4 FragColor;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
}; 
struct PointLight{
	vec3 position;
	vec3 color;
	float power;
 };

uniform Material material;
uniform PointLight light;

void main()
{    
    // ambient
    vec3 ambient = light.power * material.ambient;
  	
    // diffuse 
    vec3 norm = normalize(N);
	vec3 lightDir = normalize(L);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.power * light.color * (diff * material.diffuse);

    // specular
	vec3 Half = normalize(H);

    float spec = pow(max(dot(Half, norm), 0.0), 4.0f * material.shininess);
    vec3 specular =  light.power * light.color * (spec * material.specular);  
    
    // specular *= diff;
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
    
}
