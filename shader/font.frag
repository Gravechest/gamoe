#version 460 core

out vec4 FragColor;

in vec2 TextCoords;

uniform sampler2D font_texture;

void main(){
	FragColor = texture(font_texture,TextCoords);
	if(FragColor.r==0.0 && FragColor.g == 0.0 && FragColor.b == 0.0){
		FragColor.rgb = vec3(1.0);
	}
	else{
		discard;
	}
}