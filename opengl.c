#include <Windows.h>
#include <GL/gl.h>
#include <stdio.h>
#include <math.h>

#include "opengl.h"
#include "source.h"
#include "draw.h"
#include "tmath.h"
#include "chunk.h"
#include "ivec2.h"	
#include "lighting.h"
#include "entity_light.h"
#include "player.h"
#include "entity_item.h"
#include "entity_dark.h"
#include "entity_togui.h"
#include "gui.h"
#include "inventory.h"

u4 VBO;
u4 texture,map_texture,sprite_texture,font_texture,tile_texture;
u4 sprite_shader,map_shader,entity_dark_shader,color_shader,particle_shader,font_shader,enemy_shader;
OPENGLQUEUE gl_queue;
RGB* font_texture_data;

VEC2 mapCrdToRenderCrd(VEC2 p){
	return (VEC2){((p.x-camera.pos.x)/(camera.zoom/2.0f/RD_CMP)*1.015625f-0.0078125f-1.0f),((p.y-camera.pos.y))/(camera.zoom/2.0f)*1.015625f-0.015625f-1.0f};
}

VEC2 screenCrdToRenderCrd(VEC2 p){
	return (VEC2){((p.x)/(CHUNK_SIZE/2.0f/RD_CMP)-1.0f),(p.y)/(CHUNK_SIZE/2.0f)-1.0f};
}

VEC2 texture16Render(u4 number){
	return (VEC2){(f4)(number%TEXTURE16_ROW_COUNT)/TEXTURE16_ROW_COUNT,(f4)(number/TEXTURE16_ROW_COUNT)/TEXTURE16_ROW_COUNT};
}

u4 loadShader(u1* fragment,u1* vertex){
	u1*	fragmentSource = loadFile(fragment);
	u1* vertexSource   = loadFile(vertex);
	u4  shaderProgram  = glCreateProgram();
	u4  vertexShader   = glCreateShader(GL_VERTEX_SHADER);
	u4  fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader,1,(i1**)&fragmentSource,0);
	glShaderSource(vertexShader,1,(i1**)&vertexSource,0);
	glCompileShader(vertexShader);
	glCompileShader(fragmentShader);
	glAttachShader(shaderProgram,vertexShader);
	glAttachShader(shaderProgram,fragmentShader);
	glLinkProgram(shaderProgram);

	HeapFree(GetProcessHeap(),0,fragmentSource);
	HeapFree(GetProcessHeap(),0,vertexSource);
	return shaderProgram;
}

void openglInit(){
	glCreateProgram		      = wglGetProcAddress("glCreateProgram");
	glCreateShader		      = wglGetProcAddress("glCreateShader");
	glShaderSource		      = wglGetProcAddress("glShaderSource");
	glGetShaderInfoLog        = wglGetProcAddress("glGetShaderInfoLog");
	glCompileShader		      = wglGetProcAddress("glCompileShader");
	glAttachShader		      = wglGetProcAddress("glAttachShader");
	glLinkProgram		      = wglGetProcAddress("glLinkProgram");
	glUseProgram		      = wglGetProcAddress("glUseProgram");
	glEnableVertexAttribArray = wglGetProcAddress("glEnableVertexAttribArray");
	glVertexAttribPointer     = wglGetProcAddress("glVertexAttribPointer");
	glBufferData           	  = wglGetProcAddress("glBufferData");
	glCreateBuffers           = wglGetProcAddress("glCreateBuffers");
	glBindBuffer              = wglGetProcAddress("glBindBuffer");
	glGenerateMipmap          = wglGetProcAddress("glGenerateMipmap");
	glGetUniformLocation      = wglGetProcAddress("glGetUniformLocation");
	glActiveTexture           = wglGetProcAddress("glActiveTexture");
	glUniform1i               = wglGetProcAddress("glUniform1i");
	glUniform1f               = wglGetProcAddress("glUniform1f");
	glUniform2f               = wglGetProcAddress("glUniform2f");
	glUniform3f               = wglGetProcAddress("glUniform3f");
	wglSwapIntervalEXT        = wglGetProcAddress("wglSwapIntervalEXT");

	wglSwapIntervalEXT(VSYNC);

	sprite_shader       = loadShader("shader/sprite.frag","shader/vertex.vert");
	map_shader          = loadShader("shader/map.frag","shader/vertex.vert");
	entity_dark_shader  = loadShader("shader/entity_dark.frag","shader/vertex.vert");
	color_shader        = loadShader("shader/color.frag","shader/vertex.vert");
	particle_shader     = loadShader("shader/particle.frag","shader/vertex.vert");
	font_shader         = loadShader("shader/font.frag","shader/vertex.vert");

	font_texture_data = loadBMP("img/font.bmp");
	texture16         = loadBMP("img/texture16.bmp");

	for(u4 i = 0;i < TEXTURE16_ROW_SIZE*TEXTURE16_ROW_SIZE;i++){
		u1 r = texture16[i].r;
		texture16[i].r = texture16[i].b;
		texture16[i].b = r;
	}

	glCreateBuffers(1,&VBO);
	glBindBuffer(GL_ARRAY_BUFFER,VBO);
	
	glGenTextures(1,&texture);
	glGenTextures(1,&map_texture);
	glGenTextures(1,&font_texture);
	glGenTextures(1,&sprite_texture);
	glGenTextures(1,&tile_texture);

	glActiveTexture(GL_TEXTURE0);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D,font_texture);
	glUseProgram(font_shader);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,80,80,0,GL_RGB,GL_UNSIGNED_BYTE,font_texture_data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glUniform1i(glGetUniformLocation(font_shader,"font_texture"),3);
	HeapFree(GetProcessHeap(),0,font_texture_data);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D,map_texture);
	glUseProgram(map_shader);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D,0,GL_R8,SIM_SIZE,SIM_SIZE,0,GL_RED,GL_UNSIGNED_BYTE,map.type);
	glGenerateMipmap(GL_TEXTURE_2D);
	glUniform1i(glGetUniformLocation(map_shader,"map"),1);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D,tile_texture);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,SIM_SIZE*TILE_TEXTURE_SIZE,SIM_SIZE*TILE_TEXTURE_SIZE,0,GL_RGB,GL_UNSIGNED_BYTE,tile_texture_data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glUniform1i(glGetUniformLocation(map_shader,"tile"),4);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D,sprite_texture);
	glUseProgram(entity_dark_shader);
	glUniform1i(glGetUniformLocation(entity_dark_shader,"t_texture"),2);
	glUseProgram(sprite_shader);
	glUniform1i(glGetUniformLocation(sprite_shader,"t_texture"),2);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,TEXTURE16_ROW_SIZE,TEXTURE16_ROW_SIZE,0,GL_RGB,GL_UNSIGNED_BYTE,texture16);
	glGenerateMipmap(GL_TEXTURE_2D);
	HeapFree(GetProcessHeap(),0,texture16);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,2,GL_FLOAT,0,4 * sizeof(float),(void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1,2,GL_FLOAT,0,4 * sizeof(float),(void*)(2 * sizeof(float)));
}

void drawItem(VEC2 i_pos,SLOT item){
	if(!item.visible) return;
	if(!item.type)    return;
	f4 durability = item.durability/256.0f;
	f4 durability_r = 1.0f-item.durability/256.0f;
	VEC2 pos = i_pos;
	VEC2 pos2 = i_pos;
	VEC2 size = RD_GUI(GUI_ITEM_SIZE);
	VEC2 size2 = size;
	pos.y -= size.y*durability_r;
	pos2.y += size.y*durability;
	size.y *= durability;
	size2.y *= durability_r;
	VEC2 t_pos = texture16Render(item.type+ITEM_SPRITE_OFFSET);
	drawItemPiece(pos,size,t_pos,(VEC2){TEXTURE16_RD_SIZE,TEXTURE16_RD_SIZE*durability},(VEC3){1.0f,1.0f,1.0f});
	t_pos.y += TEXTURE16_RD_SIZE*durability;
	drawItemPiece(pos2,size2,t_pos,(VEC2){TEXTURE16_RD_SIZE,TEXTURE16_RD_SIZE*durability_r},(VEC3){0.5f,0.5f,0.5f});
}

void opengl(){
	camera = camera_new;
	if(player.pos.x - camera.zoom/2.0f - CAM_AREA > camera.pos.x){
		camera.pos.x += player.pos.x - camera.zoom/2.0f - CAM_AREA - camera.pos.x;
		if(player.pos.x > CHUNK_SIZE*2){ 
			worldLoadEast();
			glActiveTexture(GL_TEXTURE1);
			glTexSubImage2D(GL_TEXTURE_2D,0,0,0,SIM_SIZE,SIM_SIZE,GL_R,GL_UNSIGNED_BYTE,map.type);
			glActiveTexture(GL_TEXTURE4);
			glTexSubImage2D(GL_TEXTURE_2D,0,0,0,SIM_SIZE*TILE_TEXTURE_SIZE,SIM_SIZE*TILE_TEXTURE_SIZE,GL_RGB,GL_UNSIGNED_BYTE,tile_texture_data);
		}
	}
	if(player.pos.y - camera.zoom/2.0f - CAM_AREA > camera.pos.y){
		camera.pos.y += player.pos.y - camera.zoom/2.0f - CAM_AREA - camera.pos.y;
		if(player.pos.y > CHUNK_SIZE*2){
			worldLoadNorth();
			glActiveTexture(GL_TEXTURE1);
			glTexSubImage2D(GL_TEXTURE_2D,0,0,0,SIM_SIZE,SIM_SIZE,GL_R,GL_UNSIGNED_BYTE,map.type);
			glActiveTexture(GL_TEXTURE4);
			glTexSubImage2D(GL_TEXTURE_2D,0,0,0,SIM_SIZE*TILE_TEXTURE_SIZE,SIM_SIZE*TILE_TEXTURE_SIZE,GL_RGB,GL_UNSIGNED_BYTE,tile_texture_data);
		}
	}
	if(player.pos.x - camera.zoom/2.0f + CAM_AREA < camera.pos.x){
		camera.pos.x += player.pos.x - camera.zoom/2.0f + CAM_AREA - camera.pos.x;
		if(player.pos.x < CHUNK_SIZE){
			worldLoadWest();
			glActiveTexture(GL_TEXTURE1);
			glTexSubImage2D(GL_TEXTURE_2D,0,0,0,SIM_SIZE,SIM_SIZE,GL_R,GL_UNSIGNED_BYTE,map.type);
			glActiveTexture(GL_TEXTURE4);
			glTexSubImage2D(GL_TEXTURE_2D,0,0,0,SIM_SIZE*TILE_TEXTURE_SIZE,SIM_SIZE*TILE_TEXTURE_SIZE,GL_RGB,GL_UNSIGNED_BYTE,tile_texture_data);
		}
	}
	if(player.pos.y - camera.zoom/2.0f + CAM_AREA < camera.pos.y){
		camera.pos.y += player.pos.y - camera.zoom/2.0f + CAM_AREA - camera.pos.y;
		if(player.pos.y < CHUNK_SIZE){
			worldLoadSouth();
			glActiveTexture(GL_TEXTURE1);
			glTexSubImage2D(GL_TEXTURE_2D,0,0,0,SIM_SIZE,SIM_SIZE,GL_R,GL_UNSIGNED_BYTE,map.type);
			glActiveTexture(GL_TEXTURE4);
			glTexSubImage2D(GL_TEXTURE_2D,0,0,0,SIM_SIZE*TILE_TEXTURE_SIZE,SIM_SIZE*TILE_TEXTURE_SIZE,GL_RGB,GL_UNSIGNED_BYTE,tile_texture_data);
		}
	}
	VEC2addVEC2(&camera.pos,(VEC2){(tRnd()-1.5f)*camera.shake,(tRnd()-1.5f)*camera.shake});
	glClear(GL_COLOR_BUFFER_BIT);
	while(gl_queue.cnt){
		switch(gl_queue.message[--gl_queue.cnt].id){
		case GLMESSAGE_BLOCK_TILEEDIT:{
			glActiveTexture(GL_TEXTURE4);
			IVEC2 p = gl_queue.message[gl_queue.cnt].pos;
			glTexSubImage2D(GL_TEXTURE_2D,0,p.x*TILE_TEXTURE_SIZE,p.y*TILE_TEXTURE_SIZE,TILE_TEXTURE_SIZE,TILE_TEXTURE_SIZE,GL_RGB,GL_UNSIGNED_BYTE,tile_texture_data+p.x*SIM_SIZE*TILE_TEXTURE_SURFACE+p.y*TILE_TEXTURE_SIZE);
			break;
		}
		case GLMESSAGE_WHOLE_TILEEDIT:
			glActiveTexture(GL_TEXTURE4);
			glTexSubImage2D(GL_TEXTURE_2D,0,0,0,SIM_SIZE*TILE_TEXTURE_SIZE,SIM_SIZE*TILE_TEXTURE_SIZE,GL_RGB,GL_UNSIGNED_BYTE,tile_texture_data);
			break;
		case GLMESSAGE_SINGLE_TILEEDIT:{
			glActiveTexture(GL_TEXTURE4);
			IVEC2 p = gl_queue.message[gl_queue.cnt].pos;
			glTexSubImage2D(GL_TEXTURE_2D,0,p.x,p.y,1,1,GL_RGB,GL_UNSIGNED_BYTE,tile_texture_data+p.x*SIM_SIZE*TILE_TEXTURE_SIZE+p.y);
			break;
		}
		case GLMESSAGE_SINGLE_MAPEDIT:{
			glActiveTexture(GL_TEXTURE1);
			IVEC2 p = gl_queue.message[gl_queue.cnt].pos;
			glTexSubImage2D(GL_TEXTURE_2D,0,p.x,p.y,1,1,GL_RED,GL_UNSIGNED_BYTE,map.type+p.x*SIM_SIZE+p.y);
			break;
		}
		case GLMESSAGE_WND_SIZECHANGE:
			glViewport(0,0,gl_queue.message[gl_queue.cnt].pos.y,gl_queue.message[gl_queue.cnt].pos.x);
			break;
		case GLMESSAGE_WHOLE_MAPEDIT:
			glActiveTexture(GL_TEXTURE1);
			glTexSubImage2D(GL_TEXTURE_2D,0,0,0,SIM_SIZE,SIM_SIZE,GL_RED,GL_UNSIGNED_BYTE,map.type);
			break;
		}
	}
	lighting();
	for(u4 i = 0;i < camera.zoom*camera.zoom;i++){
		vram[i].r = tMinf(vramf[i].r,255.0f);
		vram[i].g = tMinf(vramf[i].g,255.0f);
		vram[i].b = tMinf(vramf[i].b,255.0f);
	}
	drawMap();
	
	glUseProgram(color_shader);

	GUIdrawInventorySlots();

	drawRect((VEC2){0.13f,0.0f},(VEC2){0.01f,1.0f},(VEC3){0.7f,0.0f,0.0f});
	for(u4 i = 0;i < laser.cnt;i++){
		VEC2 pos_1 = mapCrdToRenderCrd(laser.state[i].pos_org);
		VEC2 pos_2 = mapCrdToRenderCrd(laser.state[i].pos_dst);
		VEC3 color = RD_LASER_LUMINANCE;
		drawLine(pos_1,pos_2,color,1.0f);
	}
	if(player.weapon_cooldown){
		if(player.melee_progress){
			VEC2 pos_dst = VEC2normalizeR(playerLookDirection());
			VEC2 pos_src = player.pos;
			switch(inventory.item_primary.type){
			case ITEM_NOTHING:{
				VEC2 offset = VEC2rotR(pos_dst,PI*0.5f);
				VEC2div(&offset,2.4f);
				VEC2mul(&pos_dst,sinf((f4)player.melee_progress/PLAYER_FIST_ATTACKDURATION*PI)*1.8f);
				if(player.fist_side){
					VEC2subVEC2(&pos_dst,offset);
					VEC2subVEC2(&pos_src,offset);
				}
				else{
					VEC2addVEC2(&pos_dst,offset);
					VEC2addVEC2(&pos_src,offset);
				}
				VEC2addVEC2(&pos_dst,player.pos);
				drawLine(mapCrdToRenderCrd(pos_src),mapCrdToRenderCrd(pos_dst),(VEC3){0.7f,0.1f,0.1f},RD_MAP_CONVERT(0.2f));
				break;
			}
			case ITEM_MELEE:
				VEC2mul(&pos_dst,cosf((f4)player.melee_progress/PLAYER_MELEE_ATTACKDURATION*PI+PI*0.5f)*3.0f);
				VEC2addVEC2(&pos_dst,player.pos);
				drawLine(mapCrdToRenderCrd(pos_src),mapCrdToRenderCrd(pos_dst),(VEC3){0.7f,0.1f,0.1f},RD_MAP_CONVERT(1.0f));
				break;
			}
		}
		f4 progress;
		switch(inventory.item_primary.type){
		case ITEM_NOTHING:
			progress = 1.25f-1.25f*player.weapon_cooldown/PLAYER_FIST_COOLDOWN;
			break;
		case ITEM_LASER:
			progress = 1.25f-1.25f*player.weapon_cooldown/PLAYER_LASER_COOLDOWN;
			break;
		case ITEM_MELEE:
			progress = 1.25f-1.25f*player.weapon_cooldown/PLAYER_MELEE_COOLDOWN;
			break;
		}
		VEC2 pos = player.pos;
		pos.x += progress - 1.25f;
		pos.y += 2.0f;
		drawRect(mapCrdToRenderCrd(pos),((VEC2){progress/camera.zoom,0.5f/camera.zoom}),(VEC3){0.2f,0.5f,0.0f});
	}

	glUseProgram(sprite_shader);
	if(player.health) drawSprite(mapCrdToRenderCrd(player.pos),RD_SQUARE(PLAYER_SIZE),TEXTURE16_RENDER(SPRITE_PLAYER));
	glUseProgram(entity_dark_shader);
	for(u4 i = 0;i < 9;i++){
		VEC2 draw_pos = VEC2addVEC2R(VEC2mulVEC2R((VEC2){i/3,i%3},GUI_INVENTORY_SLOT_OFFSET),GUI_INVENTORY);
		drawItem(draw_pos,inventory.item[i]);
	}
	drawItem(GUI_PRIMARY,inventory.item_primary);
	drawItem(GUI_SECUNDARY,inventory.item_secundary);
	for(u4 i = 0;i < entity_item.cnt;i++){
		VEC3mul(&entity_item.state[i].luminance,0.008f);
		VEC2 draw_pos = mapCrdToRenderCrd(entity_item.state[i].pos);
		VEC2 draw_size = RD_SQUARE(entity_item.state[i].size);
		switch(entity_item.state[i].type){
		case ENTITY_BLOCK_PARTICLE:
			drawEnemy(draw_pos,draw_size,TEXTURE16_RENDER(SPRITE_PLAYER),entity_item.state[i].luminance);
			break;
		case ENTITY_ITEM:
			VEC3add(&entity_item.state[i].luminance,(f4)entity_item.state[i].pickup_countdown/50.0f);
			drawEnemy(draw_pos,draw_size,texture16Render(ITEM_SPRITE_OFFSET+entity_item.state[i].item.type),entity_item.state[i].luminance);
			break;
		}
		entity_item.state[i].luminance = VEC3_ZERO;
	}
	for(u4 i = 0;i < entity_dark.cnt;i++){
		VEC2 relative_pos = VEC2subVEC2R(entity_dark.state[i].pos,camera.pos);
		if(relative_pos.x>0.0f&&relative_pos.x<camera.zoom&&relative_pos.y>0.0f&&relative_pos.y<camera.zoom){
			VEC3mul(&entity_dark.state[i].luminance,0.025f);
			drawEnemy(mapCrdToRenderCrd(entity_dark.state[i].pos),RD_SQUARE(ENEMY_SIZE),TEXTURE16_RENDER(SPRITE_ENEMY),entity_dark.state[i].luminance);
			entity_dark.state[i].luminance = VEC3_ZERO;
		}
	}

	glUseProgram(particle_shader);
	for(u4 i = 0;i < entity_light.cnt;i++){
		VEC2 pos   = mapCrdToRenderCrd(entity_light.state[i].pos);
		VEC2 size  = RD_SQUARE(entity_light.state[i].size);
		VEC3 color = VEC3mulR(VEC3normalizeR(entity_light.state[i].color),1.5f);
		drawParticle(pos,size,color);
	}

	glUseProgram(font_shader);
	GUIdraw();

	for(u4 i = 0;i < entity_togui.cnt;i++){
		drawSprite(entity_togui.state[i].pos,RD_GUI(entity_togui.state[i].size),texture16Render(entity_togui.state[i].item.type+ITEM_SPRITE_OFFSET));
	}

	memset(vramf,0,sizeof(VEC3)*camera.zoom*camera.zoom);
}