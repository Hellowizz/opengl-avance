#version 330

in vec3 vViewSpacePosition;
in vec3 vViewSpaceNormal;
in vec2 vTexCoords;

out vec3 fFragColor;

layout(location = 0) out vec3 fPosition;
layout(location = 1) out vec3 fNormal;
layout(location = 2) out vec3 fAmbient;
layout(location = 3) out vec3 fDiffuse;
layout(location = 4) out vec4 fGlossyShininess;

uniform sampler2D uKdSampler; // couleur diffuse 
uniform sampler2D uKaSampler; // couleur ambiante
uniform sampler2D uKsSampler; // couleur speculaire
uniform sampler2D uShininessSampler;

uniform sampler2D uGPosition;
uniform sampler2D uGNormal;
uniform sampler2D uGAmbient;
uniform sampler2D uGDiffuse;
uniform sampler2D uGlossyShininess;

uniform vec3 uKa; // couleur amb.
uniform vec3 uKd; // couleur diffuse 
uniform vec3 uKs; // couleur speculaire
uniform float uShininess; // brillance
uniform vec3 uLightDir_vs; // direction vers la lumiere // Notez le suffixe _vs sur la direction: cela indique que nous allons travailler dans le view space; il faudra donc multiplier la direction de la lumière par la View Matrix avant de l'envoyer au shader. 
uniform vec3 uLightIntensity; // intensite de la lumiere (couleur)

vec3 blinnPhong(vec3 diffuse, vec3 specular){
	vec3 lightVector = normalize(uLightDir_vs);
	vec3 viewVector = (normalize(-vViewSpacePosition));
	vec3 halfVector = normalize(lightVector + viewVector);
	return uLightIntensity * (diffuse * dot(lightVector,vViewSpaceNormal) + specular * pow(dot(halfVector,vViewSpaceNormal), uShininess));
}

void main() {
	vec3 diffuse = uKd * texture(uKdSampler, vTexCoords).rgb;
	vec3 ambiant = uKa * texture(uKaSampler, vTexCoords).rgb;
	vec3 specular = uKs * texture(uKsSampler, vTexCoords).rgb;
	float shininess = uShininess * texture(uShininessSampler, vTexCoords).r;

	fFragColor = blinnPhong(diffuse, specular) + ambiant;

	fPosition = vec3(texelFetch(uGPosition, ivec2(fFragCoord.xy), 0)); // Correspond a vViewSpacePosition dans le forward renderer
	fNormal = vec3(texelFetch(uGNormal, ivec2(fFragCoord.xy), 0));
	fAmbient =  vec3(texelFetch(uGAmbiant, ivec2(fFragCoord.xy), 0));
	fDiffuse =  vec3(texelFetch(uGDiffuse, ivec2(fFragCoord.xy), 0));
	fGlossyShininess =  vec3(texelFetch(uGlossyShininess, ivec2(fFragCoord.xy), 0));

	fNormal = normalize(vViewSpaceNormal);
	fPosition = vViewSpacePosition;
	fAmbient = ambiant;
	fDiffuse = diffuse;
	fGlossyShininess = vec4(specular, shininess);
}