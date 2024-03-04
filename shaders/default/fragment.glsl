#version 150 core

// Kolor ostateczny
out vec4 outColor;

// Dane wejsciowe
in vec3 Position;
in vec2 UV;
in vec3 Normal;
in vec3 lightCoef;


uniform vec3 cameraPos;
uniform sampler2D fragment_texture;
uniform samplerCube tex_skybox;

uniform int env_map=0;// 1-reflection, 2-refraction



void main()//todo outcolor outside if?
{
    vec3 baseColor = texture(fragment_texture, UV).rgb;

    if (env_map==0){
        outColor = vec4(lightCoef * baseColor, 1.0);
    }
    else if (env_map==1){
        vec3 Dir = normalize(Position.xyz - cameraPos);
        vec3 Reflection = reflect(Dir, Normal);
        vec3 ReflectionColor = texture(tex_skybox, Reflection).rgb;
        vec3 final_color = baseColor/3.0 + 2.0*ReflectionColor/4.0;
        outColor = vec4(final_color,1.0);
    }
    else {
        float refractionCoeff = 1.0/1.2;
        vec3 Dir = normalize(Position.xyz - cameraPos);
        vec3 Refraction = refract(Dir, Normal, refractionCoeff);
        vec3 RefractionColor = texture(tex_skybox, Refraction).rgb;
        vec3 final_color = baseColor/3.0+ 2.0*RefractionColor/4.0;
        outColor = vec4(final_color,1.0);

    }

}
