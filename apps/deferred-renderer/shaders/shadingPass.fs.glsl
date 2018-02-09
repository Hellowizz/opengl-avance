#version 330

uniform sampler2D uGPosition;
uniform sampler2D uGNormal;
uniform sampler2D uGAmbiant;
uniform sampler2D uGDiffuse;
uniform sampler2D uGlossyShininess;

uniform vec3 uLightDir_vs; // direction vers la lumiere // Notez le suffixe _vs sur la direction: cela indique que nous allons travailler dans le view space; il faudra donc multiplier la direction de la lumière par la View Matrix avant de l'envoyer au shader. 
uniform vec3 uLightIntensity; // intensite de la lumiere (couleur)

out vec3 fFragColor;

vec3 blinnPhong(vec3 diffuse, vec4 specular, vec3 normal, vec3 position){
	vec3 lightVector = normalize(uLightDir_vs);
	vec3 viewVector = (normalize(-position));
	vec3 halfVector = normalize(lightVector + viewVector);
	return uLightIntensity * (diffuse * max(0, dot(lightVector, normal)) /*+ specular * pow(dot(halfVector, position), specular.z)*/);
}

void main() {
	vec3 position = vec3(texelFetch(uGPosition, ivec2(gl_FragCoord.xy), 0)); // Correspond a vViewSpacePosition dans le forward renderer
	vec3 normal = vec3(texelFetch(uGNormal, ivec2(gl_FragCoord.xy), 0));
	vec3 ambiant = vec3(texelFetch(uGAmbiant, ivec2(gl_FragCoord.xy), 0));
	vec3 diffuse = vec3(texelFetch(uGDiffuse, ivec2(gl_FragCoord.xy), 0));
	vec4 glossyShininess = texelFetch(uGlossyShininess, ivec2(gl_FragCoord.xy), 0);

	fFragColor = blinnPhong(diffuse, glossyShininess, normal, position) + ambiant;	
}