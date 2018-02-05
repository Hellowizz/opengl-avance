#version 330

in vec3 vViewSpacePosition;
in vec3 vViewSpaceNormal;
in vec2 vTexCoords;

uniform sampler2D uKdSampler;

uniform vec3 uKd; // couleur diffuse 
uniform vec3 uKs; // couleur speculaire
uniform float uShininess; // brillance
uniform vec3 uLightDir_vs; // direction vers la lumiere // Notez le suffixe _vs sur la direction: cela indique que nous allons travailler dans le view space; il faudra donc multiplier la direction de la lumière par la View Matrix avant de l'envoyer au shader. 
uniform vec3 uLightIntensity; // intensite de la lumiere (couleur)


out vec3 fFragColor;

vec3 blinnPhong(){
	
	vec3 lightVector = normalize(uLightDir_vs);
	vec3 viewVector = (normalize(-vViewSpacePosition));
	vec3 halfVector = normalize(lightVector + viewVector);
	return uLightIntensity * (uKd * dot(lightVector,vViewSpaceNormal) + uKs * pow(dot(halfVector,vViewSpaceNormal), uShininess));
}

void main() {
	vec4 poil = texture(uKdSampler, vTexCoords);

	fFragColor = poil.xyz * uKd * blinnPhong();	
}