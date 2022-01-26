#version 330 core

uniform sampler2D mainTex;

in vec3 fColor;
in vec2 fUV;

out vec4 fragColor;

void main()
{
    fragColor = texture(mainTex, fUV) * vec4(fColor, 1.0f);
}
