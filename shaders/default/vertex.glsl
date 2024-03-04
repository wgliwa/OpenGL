#version 330 core

// Atrybuty wierzcholkow z VAO
layout( location = 0 ) in vec4 inPosition;
layout( location = 1 ) in vec2 inUV;
layout( location = 2 ) in vec3 inNormal;


// Struktura parametrow swiatla
struct LightParam
{
	vec3 Ambient;
	vec3 Diffuse;
	vec3 Specular;
	vec3 Attenuation;
	vec3 Position; // Direction dla kierunkowego
};
// Przykladowe swiatlo
uniform LightParam myLight;

// Struktura parametrow materialu
struct MaterialParam
{
	vec3 Ambient;
	vec3 Diffuse;
	vec3 Specular;
	float Shininess;
};
// Przykladowy material
uniform MaterialParam myMaterial;

// Macierze rzutowania i transformacji
uniform mat4 matProj;
uniform mat4 matView;
uniform mat4 matModel;
uniform mat3 matNormal;
uniform vec3 cameraPos;
uniform int blinn=0;
uniform int light_on=1;
uniform int direction=0;
uniform int env_map=0;
// Dane przesylane do kolejnego etapu
out vec3 Position;
out vec2 UV;
out vec3 Normal;
out vec3 lightCoef;

// ------------------------------------------------------------
// Oswietlenie punktowe
vec3 calculatePointLight(vec4 Position, vec3 Normal, LightParam light, MaterialParam material)
{
	// Ambient
	vec3 ambientPart = light.Ambient * material.Ambient;
	vec3 L;
	if(direction==1)
	L =normalize(-light.Position);
	else
	L = normalize(light.Position - Position.xyz);
	float diff = max(dot(L, Normal), 0);
	vec3 diffusePart = diff * light.Diffuse * material.Diffuse;

	// Specular
	vec3 E = normalize(cameraPos - Position.xyz);
	float spec = 0.0;

	if(blinn==0){
		vec3 H = normalize(L+E);
		spec = pow(max(dot(Normal, H), 0), material.Shininess);

	}
	else{
		vec3 R = reflect(-E,Normal);
		spec = pow(max(dot(R,L), 0), material.Shininess);
	}

	vec3 specularPart = spec * light.Specular * material.Specular;
	// Wspolczynnik tlumienia
	float LV = distance(Position.xyz, light.Position);
	float latt = 1.0 / (light.Attenuation.x + light.Attenuation.y * LV + light.Attenuation.z * LV * LV);

	// Glowny wzor
	vec3 lightCoef = ambientPart + latt * (diffusePart + specularPart);
	return lightCoef;
}

// ------------------------------------------------------------
void main()
{

	// Przekazanie danych do shadera fragmentow
	vec4 xPosition = matModel*inPosition ;
	UV = inUV;


	vec3 xNormal = matNormal * inNormal ;
	// Wspolczynnik swiatla
	//
	if(light_on==1)
	lightCoef = calculatePointLight(xPosition, xNormal, myLight, myMaterial);
	else
	lightCoef = vec3(1.0);

	// Ostateczna pozycja wierzcholka
	Normal = mat3(transpose(inverse(matModel))) * inNormal;
	Position = vec3(matModel * inPosition);
	gl_Position = matProj * matView * matModel * inPosition;
}
