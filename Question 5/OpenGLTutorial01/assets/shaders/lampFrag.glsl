#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform vec3 diffuseColour;

void main()
{
	FragColor = vec4(diffuseColour, 1.0);
}