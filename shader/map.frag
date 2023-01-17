#version 460 core

out vec4 FragColor;

in vec2 TextCoords;

uniform sampler2D t_texture;
uniform sampler2D map;
uniform vec2 offset;
uniform vec2 camera;

uniform float zoom;

void main(){
	vec2 fpos = fract((TextCoords/1.015625+offset+0.0078125)*zoom-0.5);
	ivec2 block_pos = ivec2(((TextCoords/1.015625+offset+0.0078125)*zoom+camera)-0.5);
	int block_id = int(texelFetch(map,ivec2(((TextCoords/1.015625+offset+0.0078125)*zoom+camera)),0).r*255.0);
	switch(block_id){
	case 0:
	case 1:
	case 2:{
		vec4 source_light = texelFetch(t_texture,ivec2((TextCoords/1.015625+offset+0.0078125)*zoom),0);
		ivec2 pos = ivec2((TextCoords/1.015625+offset+0.0078125)*zoom-0.5);
		vec4 p1,p2,p3,p4;
		if(int(texture(map,block_pos+ivec2(0,0),0).r*255.0)==block_id){
			p1 = texelFetch(t_texture,pos+ivec2(0,0),0);
		}
		else{
			p1 = source_light;
		}
		if(int(texture(map,block_pos+ivec2(1,0),0).r*255.0)==block_id){
			p2 = texelFetch(t_texture,pos+ivec2(1,0),0);
		}
		else{
			p2 = source_light;
		}
		if(int(texture(map,block_pos+ivec2(0,1),0).r*255.0)==block_id){
			p3 = texelFetch(t_texture,pos+ivec2(0,1),0);
		}
		else{
			p3 = source_light;
		}
		if(int(texture(map,block_pos+ivec2(1,1),0).r*255.0)==block_id){
			p4 = texelFetch(t_texture,pos+ivec2(1,1),0);
		}
		else{
			p4 = source_light;
		}
		vec4 m1 = mix(p1,p2,fpos.x);
		vec4 m2 = mix(p3,p4,fpos.x);
		FragColor += mix(m1,m2,fpos.y);
		break;
	}
	}
}