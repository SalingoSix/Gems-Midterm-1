#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;

uniform int white;

void main()
{    
	if (white == 1)
	{	
		FragColor = vec4(1.0, 1.0, 1.0, 1.0);
	}
	
	else
	{
		FragColor = vec4(texture(texture_diffuse1, TexCoords));
	}
}