#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D frame_texture;
uniform int effect;//1-inversion, 2-sharpen, 3-blur
uniform int frame_switch;//1-on, 0-off
const float offset = 1.0 / 300.0;

void main()
{
    vec4 post;
    float kernel1[9] = float[](
    -1, -1, -1,
    -1, 9, -1,
    -1, -1, -1
    );
    float kernel2[9] = float[](
    1.0 / 16, 2.0 / 16, 1.0 / 16,
    2.0 / 16, 4.0 / 16, 2.0 / 16,
    1.0 / 16, 2.0 / 16, 1.0 / 16
    );
    if (effect==1){
        post = vec4(vec3(1.0 - texture(screenTexture, TexCoords)), 1.0);
    }
    else {
        vec2 offsets[9] = vec2[](
        vec2(-offset, offset), // top-left
        vec2(0.0f, offset), // top-center
        vec2(offset, offset), // top-right
        vec2(-offset, 0.0f), // center-left
        vec2(0.0f, 0.0f), // center-center
        vec2(offset, 0.0f), // center-right
        vec2(-offset, -offset), // bottom-left
        vec2(0.0f, -offset), // bottom-center
        vec2(offset, -offset)// bottom-right
        );
        vec3 sampleTex[9];
        for (int i = 0; i < 9; i++)
        {
            sampleTex[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
        }
        vec3 col = vec3(0.0);
        for (int i = 0; i < 9; i++){
            if (effect == 2)
                col += sampleTex[i] * kernel1[i];
            else
                col += sampleTex[i] * kernel2[i];
        }

        post = vec4(col, 1.0);
    }

    if (frame_switch==1){
        vec4 col2 = texture(frame_texture, TexCoords);
        FragColor = mix(post, col2, col2.a);
    }
    else {
        FragColor=post;
    }
}