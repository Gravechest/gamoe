#include <Windows.h>
#include <GL/gl.h>
#include <stdio.h>
#include <math.h>

#include "opengl.h"
#include "source.h"
#include "textures.h"
#include "draw.h"
#include "tmath.h"
#include "chunk.h"
#include "ivec2.h"	
#include "lighting.h"

u4 VBO;
u4 texture,map_texture,sprite_texture,font_texture;
u4 sprite_shader,map_shader,entity_dark_shader,color_shader,particle_shader,font_shader;
OPENGLQUEUE gl_queue;
RGB* font_texture_data;

u4 entity_shadows_cnt;
IVEC2 entity_shadows[255];

VEC2 mapCrdToRenderCrd(VEC2 p){
	return (VEC2){((p.x-camera.pos.x)/(camera.zoom/2.0f/RD_CMP)-1.0f),(p.y-camera.pos.y)/(camera.zoom/2.0f)-1.0f};
}

VEC2 screenCrdToRenderCrd(VEC2 p){
	return (VEC2){((p.x)/(CHUNK_SIZE/2.0f/RD_CMP)-1.0f),(p.y)/(CHUNK_SIZE/2.0f)-1.0f};
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

	glCreateBuffers(1,&VBO);
	glBindBuffer(GL_ARRAY_BUFFER,VBO);
	
	glGenTextures(1,&texture);
	glGenTextures(1,&map_texture);
	glGenTextures(1,&font_texture);
	glGenTextures(1,&sprite_texture);

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
	glTexImage2D(GL_TEXTURE_2D,0,GL_R8,SIM_SIZE,SIM_SIZE,0,GL_RED,GL_UNSIGNED_BYTE,map);
	glGenerateMipmap(GL_TEXTURE_2D);
	glUniform1i(glGetUniformLocation(map_shader,"map"),1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D,sprite_texture);
	glUseProgram(entity_dark_shader);
	glUniform1i(glGetUniformLocation(entity_dark_shader,"t_texture"),2);
	glUseProgram(sprite_shader);
	glUniform1i(glGetUniformLocation(sprite_shader,"t_texture"),2);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,TEXTURE16_SIZE,TEXTURE16_SIZE,0,GL_RGB,GL_UNSIGNED_BYTE,texture16);
	glGenerateMipmap(GL_TEXTURE_2D);
	HeapFree(GetProcessHeap(),0,texture16);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,2,GL_FLOAT,0,4 * sizeof(float),(void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1,2,GL_FLOAT,0,4 * sizeof(float),(void*)(2 * sizeof(float)));
}

void opengl(){
	camera = camera_new;
	if(player.pos.x - camera.zoom/2.0f - CAM_AREA > camera.pos.x){
		camera.pos.x += player.pos.x - camera.zoom/2.0f - CAM_AREA - camera.pos.x;
		if(player.pos.x > CHUNK_SIZE*2){ 
			worldLoadEast();
			glActiveTexture(GL_TEXTURE1);
			glTexSubImage2D(GL_TEXTURE_2D,0,0,0,SIM_SIZE,SIM_SIZE,GL_RED,GL_UNSIGNED_BYTE,map);
		}
	}
	if(player.pos.y - camera.zoom/2.0f - CAM_AREA > camera.pos.y){
		camera.pos.y += player.pos.y - camera.zoom/2.0f - CAM_AREA - camera.pos.y;
		if(player.pos.y > CHUNK_SIZE*2){
			worldLoadNorth();
			glActiveTexture(GL_TEXTURE1);
			glTexSubImage2D(GL_TEXTURE_2D,0,0,0,SIM_SIZE,SIM_SIZE,GL_RED,GL_UNSIGNED_BYTE,map);
		}
	}
	if(player.pos.x - camera.zoom/2.0f + CAM_AREA < camera.pos.x){
		camera.pos.x += player.pos.x - camera.zoom/2.0f + CAM_AREA - camera.pos.x;
		if(player.pos.x < CHUNK_SIZE){
			worldLoadWest();
			glActiveTexture(GL_TEXTURE1);
			glTexSubImage2D(GL_TEXTURE_2D,0,0,0,SIM_SIZE,SIM_SIZE,GL_RED,GL_UNSIGNED_BYTE,map);
		}
	}
	if(player.pos.y - camera.zoom/2.0f + CAM_AREA < camera.pos.y){
		camera.pos.y += player.pos.y - camera.zoom/2.0f + CAM_AREA - camera.pos.y;
		if(player.pos.y < CHUNK_SIZE){
			worldLoadSouth();
			glActiveTexture(GL_TEXTURE1);
			glTexSubImage2D(GL_TEXTURE_2D,0,0,0,SIM_SIZE,SIM_SIZE,GL_RED,GL_UNSIGNED_BYTE,map);
		}
	}
	glClear(GL_COLOR_BUFFER_BIT);
	while(gl_queue.cnt){
		switch(gl_queue.message[--gl_queue.cnt].id){
		case GLMESSAGE_SINGLE_MAPEDIT:{
			glActiveTexture(GL_TEXTURE1);
			IVEC2 p = gl_queue.message[gl_queue.cnt].pos;
			glTexSubImage2D(GL_TEXTURE_2D,0,p.x,p.y,1,1,GL_RGB,GL_UNSIGNED_BYTE,map+p.y*SIM_SIZE+p.x);
			glActiveTexture(GL_TEXTURE0);
			break;
		}
		case GLMESSAGE_WND_SIZECHANGE:
			glViewport(0,0,gl_queue.message[gl_queue.cnt].pos.y,gl_queue.message[gl_queue.cnt].pos.x);
			break;
		case GLMESSAGE_WHOLE_MAPEDIT:
			glActiveTexture(GL_TEXTURE1);
			glTexSubImage2D(GL_TEXTURE_2D,0,0,0,SIM_SIZE,SIM_SIZE,GL_RED,GL_UNSIGNED_BYTE,map);
			break;
		}
	}
	entity_shadows_cnt = 0;
	for(u4 i = 0;i < entity_dark.cnt;i++){
		f4 dv = ENEMY_SIZE/(u4)(ENEMY_SIZE+1);
		for(f4 x = entity_dark.state[i].pos.x-ENEMY_SIZE*0.5f;x <= entity_dark.state[i].pos.x+ENEMY_SIZE*0.5f;x+=dv){
			for(f4 y = entity_dark.state[i].pos.y-ENEMY_SIZE*0.5f;y <= entity_dark.state[i].pos.y+ENEMY_SIZE*0.5f;y+=dv){
				if(map[(u4)y*SIM_SIZE+(u4)x]==BLOCK_AIR){
					map[(u4)y*SIM_SIZE+(u4)x] = BLOCK_ENTITY;
					entity_shadows[entity_shadows_cnt++] = (IVEC2){x,y};
				}
			}
		}
	}
	if(player.flashlight){
		VEC2 angle = VEC2subVEC2R(getCursorPosMap(),VEC2subVEC2R(player.pos,camera.pos));
		lightsourcePartEmit(PLAYER_LUMINANCE,player.pos,angle,128,0.15f);
	}
	for(u4 i = 0;i < entity_light.cnt;i++){
		lightsourceEmit(entity_light.state[i].color,entity_light.state[i].pos,1024);
	}
	for(u4 i = 0;i < laser.cnt;i++){
		VEC2 normalize_pos = VEC2subVEC2R(laser.state[i].pos_dst,laser.state[i].pos_org);
		VEC2 direction = VEC2normalizeR(normalize_pos);
		u4 dst = VEC2length(normalize_pos);
		VEC2 pos = laser.state[i].pos_org;
		for(u4 j = 0;j < dst;j++){
			lightsourceEmit(LASER_LUMINANCE,pos,1024);
			VEC2addVEC2(&pos,direction);
		}
		if(laser.state[i].health == 1) laser.state[i].health = 0;
	}
	for(u4 i = 0;i < entity_shadows_cnt;i++){
		map[entity_shadows[i].y*SIM_SIZE+entity_shadows[i].x] = BLOCK_AIR;
	}
	for(u4 i = 0;i < camera.zoom*camera.zoom;i++){
		vram[i].r = tMinf(vramf[i].r,255.0f);
		vram[i].g = tMinf(vramf[i].g,255.0f);
		vram[i].b = tMinf(vramf[i].b,255.0f);
	}
	drawMap();
	glUseProgram(sprite_shader);
	if(player.health) drawSprite(mapCrdToRenderCrd(player.pos),RD_SQUARE(PLAYER_SIZE),PLAYER_SPRITE);
	drawSprite(screenCrdToRenderCrd(getCursorPos()),RD_GUI(2.5f),CROSSHAIR_SPRITE);
	glUseProgram(entity_dark_shader);
	for(u4 i = 0;i < entity_dark.cnt;i++){
		VEC2 relative_pos = VEC2subVEC2R(entity_dark.state[i].pos,camera.pos);
		if(relative_pos.x>0.0f&&relative_pos.x<camera.zoom&&relative_pos.y>0.0f&&relative_pos.y<camera.zoom){
			VEC3mul(&entity_dark.state[i].luminance,0.025f);
			drawEnemy(mapCrdToRenderCrd(entity_dark.state[i].pos),RD_SQUARE(ENEMY_SIZE),ENEMY_SPRITE,entity_dark.state[i].luminance);
			entity_dark.state[i].luminance = (VEC3){0.0f,0.0f,0.0f};
		}
	}
	glUseProgram(particle_shader);
	for(u4 i = 0;i < entity_light.cnt;i++){
		VEC2 pos   = mapCrdToRenderCrd(entity_light.state[i].pos);
		VEC2 size  = RD_SQUARE(entity_light.state[i].size);
		VEC3 color = VEC3mulR(VEC3normalizeR(entity_light.state[i].color),1.5f);
		drawParticle(pos,size,color);
	}
	glUseProgram(color_shader);
	drawRect((VEC2){0.13f,0.0f},(VEC2){0.01f,1.0f},(VEC3){0.7f,0.0f,0.0f});
	for(u4 i = 0;i < laser.cnt;i++){
		VEC2 pos_1 = mapCrdToRenderCrd(laser.state[i].pos_org);
		VEC2 pos_2 = mapCrdToRenderCrd(laser.state[i].pos_dst);
		VEC3 color = RD_LASER_LUMINANCE;
		drawLaser(pos_1,pos_2,color);
	}
	if(player.weapon_cooldown){
		f4 progress = 0.01f-0.01f*player.weapon_cooldown/60.0f;
		VEC2 pos = mapCrdToRenderCrd(player.pos);
		pos.x += progress - 0.01f;
		pos.y += 0.05f;
		drawRect(pos,(VEC2){progress,0.005f},(VEC3){0.2f,0.5f,0.0f});
	}
	f4 energy = (f4)player.energy/5.0f/ENERGY_MAX;
	drawRect((VEC2){GUI_ENERGY.x+0.16f+energy,GUI_ENERGY.y},(VEC2){energy,0.04f},(VEC3){0.6f,0.6f,0.12f});

	f4 health = (f4)player.health/5.0f/HEALTH_MAX;
	drawRect((VEC2){GUI_HEALTH.x+0.16f+health,GUI_HEALTH.y},(VEC2){health,0.04f},(VEC3){0.7f,0.2f,0.2f});

	glActiveTexture(GL_TEXTURE3);
	glUseProgram(font_shader);
	drawString(GUI_ENERGY,RD_GUI(2.5f),"energy=");
	drawString(GUI_HEALTH,RD_GUI(2.5f),"health=");
	u1 str[80];
	sprintf(str,"chunk: %i,%i",current_chunk.x,current_chunk.y);
	drawString((VEC2){0.3f,0.0f},RD_GUI(2.5f),str);
	sprintf(str,"player: %f,%f",player.pos.x,player.pos.y);
	drawString((VEC2){0.3f,0.1f},RD_GUI(2.5f),str);
	sprintf(str,"camera: %f,%f",camera.pos.x,camera.pos.y);
	drawString((VEC2){0.3f,0.2f},RD_GUI(2.5f),str);
	static u8 t,t2;
	QueryPerformanceCounter(&t2);
	sprintf(str,"ns/frame: %i",(t2-t)/10);
	QueryPerformanceCounter(&t);
	drawString((VEC2){0.3f,0.3f},RD_GUI(2.5f),str);
	memset(vramf,0,sizeof(VEC3)*camera.zoom*camera.zoom);
}