#version 460 core

out vec4 FragColor;

in vec2 TextCoords;

uniform vec3 color;

void main(){
	FragColor.rgb = color;
}