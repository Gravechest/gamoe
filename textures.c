#include "textures.h"
#include "vec2.h"
#include "source.h"
#include "tmath.h"

void genTextures(){
	for(u4 x = 0;x < 16;x++){
		for(u4 y = 0;y < 16;y++){
			texture16[x*TEXTURE16_SIZE+y].r = 255 - VEC2length((VEC2){tAbsf(7.5f-x),tAbsf(7.5f-y)}) * 16.0f;
			texture16[x*TEXTURE16_SIZE+y+16].g = tMaxf(255 - VEC2length((VEC2){tAbsf(7.5f-x),tAbsf(7.5f-y)}) * 32.0f,0);
			
			texture16[x*TEXTURE16_SIZE+y+TEXTURE16_SIZE*16+16].r = 127;
			texture16[x*TEXTURE16_SIZE+y+TEXTURE16_SIZE*16+16].g = 127;
			texture16[x*TEXTURE16_SIZE+y+TEXTURE16_SIZE*16+16].b = 127;
		}
	}
	//generate crosshair
	for(u4 i = 0;i < 16;i++){
		texture16[i*TEXTURE16_SIZE+7+TEXTURE16_SIZE*16].r = 200;
		texture16[i*TEXTURE16_SIZE+8+TEXTURE16_SIZE*16].r = 200;
		texture16[7*TEXTURE16_SIZE+i+TEXTURE16_SIZE*16].r = 200;
		texture16[8*TEXTURE16_SIZE+i+TEXTURE16_SIZE*16].r = 200;
	}
	//generate entity_dark
	for(u4 x = 10;x < 14;x++){
		for(u4 y = 2;y < 6;y++){
			texture16[x*TEXTURE16_SIZE+y+TEXTURE16_SIZE*16+16].r = 255;
			texture16[x*TEXTURE16_SIZE+y+TEXTURE16_SIZE*16+16].g = 255;
			texture16[x*TEXTURE16_SIZE+y+TEXTURE16_SIZE*16+16].b = 255;

			texture16[x*TEXTURE16_SIZE+y+TEXTURE16_SIZE*16+16+8].r = 255;
			texture16[x*TEXTURE16_SIZE+y+TEXTURE16_SIZE*16+16+8].g = 255;
			texture16[x*TEXTURE16_SIZE+y+TEXTURE16_SIZE*16+16+8].b = 255;
		}
	}
}