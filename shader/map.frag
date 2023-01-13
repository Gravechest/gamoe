#version 460 core

out vec4 FragColor;

in vec2 TextCoords;

uniform sampler2D t_texture;
uniform sampler2D map;
uniform vec2 offset;
uniform vec2 camera;

void main(){
	vec2 fpos = fract((TextCoords+offset)*128.0);
	switch(int(texture(map,TextCoords/3.0+camera).r*255.0)){
	case 0:
	case 1:
	case 2:
		FragColor += texture(t_texture,TextCoords/1.015625+offset+0.0078125);
		break;
	}
}