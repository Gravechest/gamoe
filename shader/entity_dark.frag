#version 460 core

out vec4 FragColor;

in vec2 TextCoords;

uniform vec3 luminance;

uniform sampler2D t_texture;

void main(){
	FragColor = texture(t_texture,TextCoords);
	if(FragColor.r==0.0 && FragColor.g == 0.0 && FragColor.b == 0.0){
		discard;
	}
	FragColor.rgb *= luminance;
}