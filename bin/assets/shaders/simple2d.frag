#version 330 core

in vec3 fCol;

out vec4 FragColor;

uniform float alpha;
//uniform vec3 color;

void main()
{
    FragColor = vec4(fCol, alpha);
}
