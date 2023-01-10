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
		FragColor += texture(t_texture,TextCoords+offset);
		break;
	case 2:
		FragColor.rgb = vec3(1.0);
		break;
	}
}