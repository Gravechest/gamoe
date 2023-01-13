#version 460 core

out vec4 FragColor;

in vec2 TextCoords;

uniform vec3 luminance;

void main(){
	FragColor.rgb = luminance;
}