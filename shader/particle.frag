#version 460 core

out vec4 FragColor;

in vec2 TextCoords;

uniform vec3 luminance;

void main(){
	float dst = distance(vec2(0.5),TextCoords);
	if(dst<0.5){
		FragColor.rgb = luminance * (1.0-dst*dst);
	}
	else{
		discard;
	}
}