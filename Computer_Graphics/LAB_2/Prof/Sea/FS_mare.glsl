#version 330 core

uniform float time;
uniform vec2 resolution;
in vec4 ourColor;
vec3 color;

float noise(float x, float amplitude) {
	float n = sin(x*8.0 + time) * 0.05 +
			  sin(x*27.3 + time*0.5) * 0.005 + 
		      sin(time) * 0.01;
	return n * amplitude;
}

const vec3 water = vec3(0.2,0.1,1.0);
 
 void main() {
	//Conversione da coordinate di frammento a coordinate normalizzate di device (ndc)
	vec2 ndc = vec2 ((gl_FragCoord.x / resolution.x- 0.5) * 2, (gl_FragCoord.y / resolution.y - 0.5) * 2);
	
   	float v = 0.0;
	//Se le ordinate ndc del frammento sotto esame variano tra -1 e 0
	if(ndc.y> -1 && ndc.y<0)
	{
	//Costruisci le componenti di frequenza di rumore sinusoidale
		float n0 = noise(ndc.x * 3.18, 0.3);
		float n1 = noise(ndc.x * 2.34, 0.4);
		float n2 = noise(ndc.x * 2.14, 0.6);
		float n3 = noise(ndc.x * 1.4, 0.8);
		float n4 = noise(ndc.x * 1.23, 1.0);
	    
		//Determina il colore del mare in base alla distanza delle coordinate ndc dal fondo
		if (ndc.y <- 0.8-n4) 
		{
			v = 1;
		} 			
		else if (ndc.y <  -0.6- n3) {
			v = 0.8;
		}
		else if (ndc.y < -0.4 - n2) {
			v = 0.6;
		} else if (ndc.y <- 0.2 - n1) {
			v = 0.4;
		} else if (ndc.y < - n0) {
			v = 0.3;
		}
 		//Miscela il colore che secondo l'applicazione avrebbe dovuto avere il frammento con il colore dell'acqua
		if (v > 0.0) {
			 vec3 water_color = water * v;
			color = mix(vec3(ourColor.r,ourColor.g,ourColor.b),water_color,0.8);
		}
		 gl_FragColor = vec4(color, 1.0);
	}		
		else
		//Colora con il colore stabilito dall'applicazione gli altri frammenti
		gl_FragColor = ourColor;
}