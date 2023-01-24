#version 460 core

out vec4 FragColor;

in vec2 TextCoords;

uniform sampler2D t_texture;
uniform sampler2D map;
uniform sampler2D tile;
uniform vec2 offset;
uniform vec2 camera;

uniform float zoom;

uint hash( uint x ) {
    x += ( x << 10u );
    x ^= ( x >>  6u );
    x += ( x <<  3u );
    x ^= ( x >> 11u );
    x += ( x << 15u );
    return x;
}

uint hash( uvec2 v ) { return hash( v.x ^ hash(v.y)                         ); }
uint hash( uvec3 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z)             ); }
uint hash( uvec4 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z) ^ hash(v.w) ); }

float floatConstruct( uint m ) {
    const uint ieeeMantissa = 0x007FFFFFu; 
    const uint ieeeOne      = 0x3F800000u; 

    m &= ieeeMantissa;                   
    m |= ieeeOne;                      

    float  f = uintBitsToFloat( m );      
    return f - 1.0;                       
}

float random( float x ) { return floatConstruct(hash(floatBitsToUint(x))); }
float random( vec2  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
float random( vec3  v ) { return floatConstruct(hash(floatBitsToUint(v))); }

void main(){
	vec2 fpos = fract((TextCoords/1.015625+offset+0.0078125)*zoom-0.5);
	vec2 fpos_n = fract((TextCoords/1.015625+offset+0.0078125)*zoom);
	ivec2 block_pos = ivec2(((TextCoords/1.015625+offset+0.0078125)*zoom+camera-0.5));
	vec2 block = texelFetch(map,ivec2(((TextCoords/1.015625+offset+0.0078125)*zoom+camera)),0).rg;
	int block_id = int(block.r*255.0);
	vec4 tile_texture = texelFetch(tile,ivec2(((TextCoords/1.015625+offset+0.0078125)*zoom+camera)*4.0),0);
	switch(block_id){
	case 5:
		if(distance(fpos_n,vec2(0.5)) > 0.5){
			block_id = 0;
			tile_texture.rgb = vec3(0.2,0.8,0.1);
		}
	case 1:
		if(block.g + 0.25 < random(ivec2(fpos_n*4.0))) block_id = 0;
		break;
	}
	ivec2 pos = ivec2((TextCoords/1.015625+offset+0.0078125)*zoom-0.5);
	vec4 p1,p2,p3,p4;
	if(int(texelFetch(map,block_pos+ivec2(0,0),0).r*255.0)==block_id){
		p1 = texelFetch(t_texture,pos+ivec2(0,0),0);
	}
	else{
		p1 = vec4(0.0);
	}
	if(int(texelFetch(map,block_pos+ivec2(1,0),0).r*255.0)==block_id){
		p2 = texelFetch(t_texture,pos+ivec2(1,0),0);
	}
	else{
		p2 = vec4(0.0);
	}
	if(int(texelFetch(map,block_pos+ivec2(0,1),0).r*255.0)==block_id){
		p3 = texelFetch(t_texture,pos+ivec2(0,1),0);
	}
	else{
		p3 = vec4(0.0);
	}
	if(int(texelFetch(map,block_pos+ivec2(1,1),0).r*255.0)==block_id){
		p4 = texelFetch(t_texture,pos+ivec2(1,1),0);
	}
	else{
		p4 = vec4(0.0);
	}
	vec4 m1 = mix(p1,p2,fpos.x);
	vec4 m2 = mix(p3,p4,fpos.x);
	FragColor += mix(m1,m2,fpos.y) * tile_texture;
}