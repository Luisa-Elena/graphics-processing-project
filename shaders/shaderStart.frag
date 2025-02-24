#version 410 core

in vec3 fNormal;
in vec4 fPosEye;
in vec2 fTexCoords;
in vec4 fragPosLightSpace;

// directional light
out vec4 fColor;
uniform	vec3 lightDir;
uniform	vec3 lightColor;

// point lights
uniform vec3 pointLightColor;
uniform vec3 pointLightPos;
uniform vec3 pointLightColor1;
uniform vec3 pointLightPos1;

// spotlight
uniform vec3 spotLightPos;
uniform vec3 spotLightDir;
uniform vec3 spotLightColor;
uniform float spotLightCutOff;
uniform float spotLightOuterCutOff;

uniform bool pointLightEnabled;
uniform bool spotLightEnabled;

//texture
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

float constant = 1.0f;
float linear = 0.7f;
float quadratic = 1.8f;

float spotConstant = 1.0f;
float spotLinear = 0.09f;
float spotQuadratic = 0.032f;

vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;
float shininess = 32.0f;


void computeDirectionalLightComponents()
{		
	vec3 cameraPosEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin
	
	//transform normal
	vec3 normalEye = normalize(fNormal);	
	
	//compute light direction
	vec3 lightDirN = normalize(lightDir);
	
	//compute view direction 
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
		
	//compute ambient light
	ambient += ambientStrength * lightColor;
	
	//compute diffuse light
	diffuse += max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	
	//compute specular light
	vec3 reflection = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);
	specular += specularStrength * specCoeff * lightColor;
}

void computePointLightComponents(vec3 pointLightPos, vec3 pointLightColor)
{
	vec3 cameraPosEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin
	
	//transform normal
	vec3 normalEye = normalize(fNormal);	
	
	//compute light direction
	vec3 lightDirN = normalize(pointLightPos.xyz - fPosEye.xyz);
	
	//compute view direction 
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);

	//compute half vector
	vec3 halfVector = normalize(lightDirN + viewDirN);

	float dist  = length(pointLightPos - fPosEye.xyz);
	float att = 1.0f / (constant + linear * dist + quadratic * dist * dist);
		
	//compute ambient light
	ambient += att * ambientStrength * pointLightColor;
	
	//compute diffuse light
	diffuse += att * max(dot(normalEye, lightDirN), 0.0f) * pointLightColor;
	
	//compute specular light
	//no need for reflection at all
	vec3 reflection = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(normalEye, halfVector), 0.0f), shininess);
	specular += specularStrength * specCoeff * pointLightColor;
}

void computeSpotlightComponents()
{
	vec3 cameraPosEye = vec3(0.0f);
	vec3 normalEye = normalize(fNormal);
	vec3 lightDirN = normalize(spotLightPos.xyz - fPosEye.xyz);
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
	vec3 halfVector = normalize(lightDirN + viewDirN);
	float dist  = length(spotLightPos - fPosEye.xyz);
	float att = 1.0f / (spotConstant + spotLinear * dist + spotQuadratic * dist * dist);

	float theta = dot(lightDirN, normalize(-spotLightDir));
	float epsilon   = spotLightCutOff - spotLightOuterCutOff;
	float intensity = clamp((theta - spotLightOuterCutOff) / epsilon, 0.0, 1.0);
	
	if(theta > spotLightOuterCutOff) {
		ambient += att * ambientStrength * spotLightColor;
		diffuse += att * max(dot(normalEye, lightDirN), 0.0f) * spotLightColor * intensity;
		vec3 reflection = reflect(-lightDirN, normalEye);
		float specCoeff = pow(max(dot(normalEye, halfVector), 0.0f), shininess);
		specular += specularStrength * specCoeff * spotLightColor * intensity;
	}
}


float computeShadow() {
	vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	normalizedCoords = normalizedCoords * 0.5 + 0.5;
	if (normalizedCoords.z > 1.0f) return 0.0f;
	float closestDepth = texture(shadowMap, normalizedCoords.xy).r;
	float currentDepth = normalizedCoords.z;
	float bias = 0.005f;
	float shadow = currentDepth - bias > closestDepth ? 1.0f : 0.0f;
	return shadow;
}

float computeFog()
{
 float fogDensity = 0.05f;
 float fragmentDistance = length(fPosEye);
 float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));

 return clamp(fogFactor, 0.0f, 1.0f);
}


void main() 
{
	computeDirectionalLightComponents();

	ambient *= texture(diffuseTexture, fTexCoords).rgb;
	diffuse *= texture(diffuseTexture, fTexCoords).rgb;
	specular *= texture(specularTexture, fTexCoords).rgb;

	if(pointLightEnabled) {
		computePointLightComponents(pointLightPos, pointLightColor);
		computePointLightComponents(pointLightPos1, pointLightColor1);
	}

	if(spotLightEnabled) {
		computeSpotlightComponents();
	}
	
	vec3 baseColor = vec3(0.9f, 0.35f, 0.0f);//orange
	
	float shadow = computeShadow();

	vec3 color = min((ambient + (1.0f - shadow) * diffuse) + (1.0f - shadow) * specular, 1.0f);

	float fogFactor = computeFog();
	vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
	fColor = fogColor * (1 - fogFactor) + vec4(color, 0.0f) * fogFactor;
}