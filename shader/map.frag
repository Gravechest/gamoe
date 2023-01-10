#version 460 core

out vec4 FragColor;

in vec2 TextCoords;

uniform sampler2D t_texture;
uniform vec2 offset;

void main(){
	FragColor = texture(t_texture,TextCoords+offset);
}