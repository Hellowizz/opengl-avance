#version 330

layout(location = 0) out vec3 fPosition;
layout(location = 1) out vec3 fNormal;
layout(location = 2) out vec3 fAmbient;
layout(location = 3) out vec3 fDiffuse;
layout(location = 4) out vec4 fGlossyShininess;

uniform sampler2D uGPosition;
uniform sampler2D uGNormal;
uniform sampler2D uGAmbient;
uniform sampler2D uGDiffuse;
uniform sampler2D uGlossyShininess;

uniform vec3 uLightDir_vs; // direction vers la lumiere // Notez le suffixe _vs sur la direction: cela indique que nous allons travailler dans le view space; il faudra donc multiplier la direction de la lumière par la View Matrix avant de l'envoyer au shader. 
uniform vec3 uLightIntensity; // intensite de la lumiere (couleur)

void main() {
	fPosition = vec3(texelFetch(uGPosition, ivec2(gl_fFragCoord.xy), 0)); // Correspond a vViewSpacePosition dans le forward renderer
	fNormal = vec3(texelFetch(uGNormal, ivec2(gl_fFragCoord.xy), 0));
	fAmbient = vec3(texelFetch(uGAmbiant, ivec2(gl_fFragCoord.xy), 0));
	fDiffuse = vec3(texelFetch(uGDiffuse, ivec2(gl_fFragCoord.xy), 0));
	fGlossyShininess = vec3(texelFetch(uGlossyShininess, ivec2(gl_fFragCoord.xy), 0) ;
}