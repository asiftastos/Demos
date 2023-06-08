#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aCol;

uniform mat4 model;
uniform mat4 proj;

out vec3 fCol;

void main()
{
    gl_Position = proj * model * vec4(aPos, 1.0f);
    fCol = aCol;
}
