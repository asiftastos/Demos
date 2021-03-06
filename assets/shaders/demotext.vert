#version 450 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texcoord;

uniform mat4 model;
uniform mat4 proj;

out vec2 uv0;

void main()
{
    gl_Position = proj * model * vec4(position, 1.0f);
    uv0 = texcoord;
}