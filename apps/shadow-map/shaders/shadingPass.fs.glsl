#version 330

uniform sampler2D uGPosition;
uniform sampler2D uGNormal;
uniform sampler2D uGAmbiant;
uniform sampler2D uGDiffuse;
uniform sampler2D uGlossyShininess;

uniform vec3 uLightDir_vs; // direction vers la lumiere // Notez le suffixe _vs sur la direction: cela indique que nous allons travailler dans le view space; il faudra donc multiplier la direction de la lumière par la View Matrix avant de l'envoyer au shader. 
uniform vec3 uLightIntensity; // intensite de la lumiere (couleur)

uniform mat4 uDirLightViewProjMatrix;
uniform sampler2D uDirLightShadowMap;
uniform float uDirLightShadowMapBias;

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

	vec4 positionInDirLightScreen = uDirLightViewProjMatrix * vec4(position, 1);
	vec3 positionInDirLightNDC = vec3(positionInDirLightScreen / positionInDirLightScreen.w) * 0.5 + 0.5;
	float depthBlockerInDirSpace = texture(uDirLightShadowMap, positionInDirLightNDC.xy).r;
	float dirLightVisibility = positionInDirLightNDC.z < depthBlockerInDirSpace + uDirLightShadowMapBias ? 1.0 : 0.0;

        fFragColor = blinnPhong(diffuse, glossyShininess, normal, position) * (dirLightVisibility * max(0.f, dot(normal, uLightDir_vs)));
//        fFragColor = vec3(texelFetch(uDirLightShadowMap, ivec2(gl_FragCoord.xy), 0));
}
