#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D frame_texture;

void main()
{
    vec4 col = texture(screenTexture, TexCoords);
    vec4 col2 = texture(frame_texture, TexCoords);
    FragColor = mix(col,col2,col2.a);
}
