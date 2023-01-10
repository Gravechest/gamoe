#version 460 core

layout (location = 0) in vec2 verticles;
layout (location = 1) in vec2 textcoords;

out vec2 TextCoords;

void main(){
	TextCoords = textcoords;
	gl_Position = vec4(vec2(verticles),0.0,1.0);
}