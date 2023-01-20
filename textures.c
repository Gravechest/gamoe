#include "textures.h"
#include "source.h"
#include "tmath.h"
#include "ray.h"

#define COLOR_WHITE (RGB){255,255,255}
#define COLOR_GREY  (RGB){127,127,127}

#include <stdio.h>

VEC2 texture16Render(u4 number){
	return (VEC2){(f4)(number%TEXTURE16_ROW_COUNT)/TEXTURE16_ROW_COUNT,(f4)(number/TEXTURE16_ROW_COUNT)/TEXTURE16_ROW_COUNT};
}

void fillSquare(IVEC2 pos,IVEC2 size,RGB color,u4 offset){
	for(u4 x = pos.x;x < pos.x+size.x;x++){
		for(u4 y = pos.y;y < pos.y+size.y;y++){
			texture16[x*TEXTURE16_ROW_SIZE+y+offset] = color;
		}
	}
}

void fillLine(VEC2 pos1,VEC2 pos2,RGB color,u4 offset){
	VEC2 direction = VEC2subVEC2R(pos2,pos1);
	u4 iterations = tAbsf(direction.x)+tAbsf(direction.y);
	RAY2D ray = ray2dCreate(pos1,direction);
	for(u4 i = 0;i < iterations;i++){
		texture16[ray.square_pos.x*TEXTURE16_ROW_SIZE+ray.square_pos.y+offset] = color;
		ray2dIterate(&ray);
	}
}

void genTextures(){
	for(u4 x = 0;x < 16;x++){
		for(u4 y = 0;y < 16;y++){
			texture16[x*TEXTURE16_ROW_SIZE+y].r = 255 - VEC2length((VEC2){tAbsf(7.5f-x),tAbsf(7.5f-y)}) * 16.0f;
			texture16[x*TEXTURE16_ROW_SIZE+y+TEXTURE16_OFFSET(1)].g = tMaxf(255 - VEC2length((VEC2){tAbsf(7.5f-x),tAbsf(7.5f-y)}) * 32.0f,0);
		}
	}
	//generate crosshair
	for(u4 i = 0;i < 16;i++){
		texture16[i*TEXTURE16_ROW_SIZE+7+TEXTURE16_OFFSET(2)].r = 200;
		texture16[i*TEXTURE16_ROW_SIZE+8+TEXTURE16_OFFSET(2)].r = 200;
		texture16[7*TEXTURE16_ROW_SIZE+i+TEXTURE16_OFFSET(2)].r = 200;
		texture16[8*TEXTURE16_ROW_SIZE+i+TEXTURE16_OFFSET(2)].r = 200;
	}
	//generate entity_dark
	fillSquare(IVEC2_ZERO,(IVEC2){16,16},(RGB){50,50,50},TEXTURE16_OFFSET(3));
	fillSquare((IVEC2){10,2},(IVEC2){4,4},COLOR_WHITE,TEXTURE16_OFFSET(3));
	fillSquare((IVEC2){10,10},(IVEC2){4,4},COLOR_WHITE,TEXTURE16_OFFSET(3));
	fillSquare((IVEC2){2,2},(IVEC2){4,12},COLOR_WHITE,TEXTURE16_OFFSET(3));
	//generate melee_icon
	fillSquare((IVEC2){1,1},(IVEC2){15,15},(RGB){50,10,10},TEXTURE16_OFFSET(4));
	fillLine((VEC2){0.0f,0.0f},(VEC2){16.0f,0.0f},COLOR_GREY,TEXTURE16_OFFSET(4));
	fillLine((VEC2){0.0f,0.0f},(VEC2){0.0f,16.0f},COLOR_GREY,TEXTURE16_OFFSET(4));
	fillLine((VEC2){15.0f,15.0f},(VEC2){16.0f,0.0f},COLOR_GREY,TEXTURE16_OFFSET(4));
	fillLine((VEC2){15.0f,15.0f},(VEC2){0.0f,16.0f},COLOR_GREY,TEXTURE16_OFFSET(4));
	fillLine((VEC2){2.0f,2.0f},(VEC2){13.0f,13.0f},(RGB){180,0,0},TEXTURE16_OFFSET(4));
	fillLine((VEC2){3.0f,2.0f},(VEC2){14.0f,13.0f},(RGB){220,20,20},TEXTURE16_OFFSET(4));
}