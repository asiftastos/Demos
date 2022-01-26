#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aUV;

uniform mat4 model;
uniform mat4 proj;

out vec3 fColor;
out vec2 fUV;

void main()
{
    gl_Position = proj * model * vec4(aPos, 1.0f);
    fColor = aColor;
    fUV = aUV;
}
