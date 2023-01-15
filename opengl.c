#include <windows.h>
#include <GL/gl.h>

#include "opengl.h"
#include "source.h"
#include "textures.h"
#include "draw.h"
#include "tmath.h"
#include "ray.h"
#include "chunk.h"

u4 VBO;
u4 texture,map_texture,sprite_texture;
u4 spriteShader,mapShader,enemyShader,colorShader,particleShader;
OPENGLQUEUE gl_queue;

VEC2 mapCrdToRenderCrd(VEC2 p){
	return (VEC2){((p.x-camera.pos.x)/(camera.zoom/2.0f/RD_CMP)-1.0f),(p.y-camera.pos.y)/(camera.zoom/2.0f)-1.0f};
}

VEC2 screenCrdToRenderCrd(VEC2 p){
	return (VEC2){((p.x)/(RES/2.0f/RD_CMP)-1.0f),(p.y)/(RES/2.0f)-1.0f};
}

u1* loadFile(u1* name){
	HANDLE h = CreateFileA(name,GENERIC_READ,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
	u4 fsize = GetFileSize(h,0);
	u1* r = HeapAlloc(GetProcessHeap(),0,fsize+1);
	ReadFile(h,r,fsize,0,0);
	r[fsize] = 0;
	CloseHandle(h);
	return r;
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

void illuminateOutside(VEC3 color,VEC2 pos,f4 angle){
	VEC2 direction = (VEC2){cosf(angle*PI*2.0f),sinf(angle*PI*2.0f)};
	if(iSquare(pos,direction,RES/2.0f)){
		RAY2D ray = ray2dCreate(pos,direction);
		while((ray.roundPos.x < 0 || ray.roundPos.x >= camera.zoom || ray.roundPos.y < 0 || ray.roundPos.y >= camera.zoom)){
			IVEC2 block = {(ray.roundPos.x+(u4)camera.pos.x),(ray.roundPos.y+(u4)camera.pos.y)};
			if(block.y*MAP+block.x<0||block.y*MAP+block.x>MAP*MAP||map[block.y*MAP+block.x]==1){
				break;
			}
			ray2dIterate(&ray);
		}
		while(ray.roundPos.x >= 0 && ray.roundPos.x < camera.zoom && ray.roundPos.y >= 0 && ray.roundPos.y < camera.zoom){
			if(map[(ray.roundPos.y+(u4)camera.pos.y)*MAP+(ray.roundPos.x+(u4)camera.pos.x)]==1){
				vramf[ray.roundPos.y*(u4)camera.zoom+ray.roundPos.x].r+=color.r*4.0f;
				vramf[ray.roundPos.y*(u4)camera.zoom+ray.roundPos.x].g+=color.g*4.0f;
				vramf[ray.roundPos.y*(u4)camera.zoom+ray.roundPos.x].b+=color.b*4.0f;
				break;
			}
			vramf[ray.roundPos.y*(u4)camera.zoom+ray.roundPos.x].r+=color.r;
			vramf[ray.roundPos.y*(u4)camera.zoom+ray.roundPos.x].g+=color.g;
			vramf[ray.roundPos.y*(u4)camera.zoom+ray.roundPos.x].b+=color.b;
			ray2dIterate(&ray);
		}
	}
}

void illuminateInside(VEC3 color,VEC2 pos,f4 angle){
	RAY2D ray = ray2dCreate(pos,(VEC2){cosf(angle*PI*2.0f),sinf(angle*PI*2.0f)});
	while(ray.roundPos.x > 0.0f && ray.roundPos.x < camera.zoom && ray.roundPos.y > 0.0f && ray.roundPos.y < camera.zoom){
		if(map[(ray.roundPos.y+(u4)camera.pos.y)*MAP+(ray.roundPos.x+(u4)camera.pos.x)]==1){
			vramf[ray.roundPos.y*(u4)camera.zoom+ray.roundPos.x].r+=color.r*4.0f;
			vramf[ray.roundPos.y*(u4)camera.zoom+ray.roundPos.x].g+=color.g*4.0f;
			vramf[ray.roundPos.y*(u4)camera.zoom+ray.roundPos.x].b+=color.b*4.0f;
			break;
		}
		vramf[ray.roundPos.y*(u4)camera.zoom+ray.roundPos.x].r+=color.r;
		vramf[ray.roundPos.y*(u4)camera.zoom+ray.roundPos.x].g+=color.g;
		vramf[ray.roundPos.y*(u4)camera.zoom+ray.roundPos.x].b+=color.b;
		ray2dIterate(&ray);
	}
}

void illuminateMapPart(VEC3 color,VEC2 pos,VEC2 angle,u4 ammount,f4 wideness){
	f4 tan_angle = atan2f(angle.y,angle.x) / (PI*2.0f);
	tan_angle -= wideness/2.0f;
	VEC2subVEC2(&pos,(VEC2){(u4)camera.pos.x,(u4)camera.pos.y});
	if(pos.x < 1.0f || pos.y < 1.0f || pos.x > camera.zoom || pos.y > camera.zoom){
		for(f4 i = tan_angle;i < wideness+tan_angle;i+=wideness/ammount){
			illuminateOutside(color,pos,i);
		}
	}
	else{
		for(f4 i = tan_angle;i < wideness+tan_angle;i+=wideness/ammount){
			illuminateInside(color,pos,i);
		}	
	}
}

void illuminateMap(VEC3 color,VEC2 pos,u4 ammount){
	VEC2subVEC2(&pos,(VEC2){(u4)camera.pos.x,(u4)camera.pos.y});
	f4 offset = tRnd();
	if(pos.x < 1.0f || pos.y < 1.0f || pos.x > camera.zoom || pos.y > camera.zoom){
		for(f4 i = offset;i < 1.0f+offset;i+=1.0f/ammount){
			illuminateOutside(color,pos,i);
		}
	}
	else{
		for(f4 i = offset;i < 1.0f+offset;i+=1.0f/ammount){
			illuminateInside(color,pos,i);
		}	
	}
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
	glUniform2f               = wglGetProcAddress("glUniform2f");
	glUniform3f               = wglGetProcAddress("glUniform3f");
	wglSwapIntervalEXT        = wglGetProcAddress("wglSwapIntervalEXT");

	wglSwapIntervalEXT(VSYNC);

	spriteShader   = loadShader("shader/sprite.frag"  ,"shader/vertex.vert");
	mapShader      = loadShader("shader/map.frag"     ,"shader/vertex.vert");
	enemyShader    = loadShader("shader/enemy.frag"   ,"shader/vertex.vert");
	colorShader    = loadShader("shader/color.frag"   ,"shader/vertex.vert");
	particleShader = loadShader("shader/particle.frag","shader/vertex.vert");

	glCreateBuffers(1,&VBO);
	glBindBuffer(GL_ARRAY_BUFFER,VBO);
	
	glGenTextures(1,&texture);
	glGenTextures(1,&map_texture);
	glGenTextures(1,&sprite_texture);
	glBindTexture(GL_TEXTURE_2D,map_texture);

	glUseProgram(mapShader);
	glActiveTexture(GL_TEXTURE1);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D,0,GL_R8,MAP,MAP,0,GL_RED,GL_UNSIGNED_BYTE,map);
	glGenerateMipmap(GL_TEXTURE_2D);
	glUniform1i(glGetUniformLocation(mapShader,"map"),1);

	glBindTexture(GL_TEXTURE_2D,sprite_texture);
	glUseProgram(enemyShader);
	glUniform1i(glGetUniformLocation(enemyShader,"t_texture"),2);
	glUseProgram(spriteShader);
	glUniform1i(glGetUniformLocation(spriteShader,"t_texture"),2);
	glActiveTexture(GL_TEXTURE2);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,TEXTURE16_SIZE,TEXTURE16_SIZE,0,GL_RGB,GL_UNSIGNED_BYTE,texture16);
	glGenerateMipmap(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D,texture);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,2,GL_FLOAT,0,4 * sizeof(float),(void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1,2,GL_FLOAT,0,4 * sizeof(float),(void*)(2 * sizeof(float)));
}

void opengl(){
	camera = camera_new;
	if(player.pos.x - camera.zoom/2.0f - CAM_AREA > camera.pos.x){
		camera.pos.x += player.pos.x - camera.zoom/2.0f - CAM_AREA - camera.pos.x;
		if(camera.pos.x > RES+RES/2) worldLoadEast();
	}
	if(player.pos.y - camera.zoom/2.0f - CAM_AREA > camera.pos.y){
		camera.pos.y += player.pos.y - camera.zoom/2.0f - CAM_AREA - camera.pos.y;
		if(camera.pos.y > RES+RES/2) worldLoadNorth();
	}
	if(player.pos.x - camera.zoom/2.0f + CAM_AREA < camera.pos.x){
		camera.pos.x += player.pos.x - camera.zoom/2.0f + CAM_AREA - camera.pos.x;
		if(camera.pos.x < RES-RES/2) worldLoadWest();
	}
	if(player.pos.y - camera.zoom/2.0f + CAM_AREA < camera.pos.y){
		camera.pos.y += player.pos.y - camera.zoom/2.0f + CAM_AREA - camera.pos.y;
		if(camera.pos.y < RES-RES/2) worldLoadSouth();
	}
	glClear(GL_COLOR_BUFFER_BIT);
	while(gl_queue.cnt){
		switch(gl_queue.message[--gl_queue.cnt].id){
		case 0:{
			glActiveTexture(GL_TEXTURE1);
			IVEC2 p = gl_queue.message[gl_queue.cnt].pos;
			glTexSubImage2D(GL_TEXTURE_2D,0,p.x,p.y,1,1,GL_RGB,GL_UNSIGNED_BYTE,map+p.y*MAP+p.x);
			glActiveTexture(GL_TEXTURE0);
			break;
		}
		case 1:
			drawRect(gl_queue.message[gl_queue.cnt].rect.pos,
					    gl_queue.message[gl_queue.cnt].rect.size,
						gl_queue.message[gl_queue.cnt].rect.color);
			break;
		}
	}
	for(u4 i = 0;i < MAP*MAP;i++){
		if(map[i]==2){
			illuminateMap((VEC3){0.02f,0.02f,0.2f},(VEC2){(f4)(i%MAP)+0.5f,(f4)(i/MAP)+0.5f},1024*4);
		}
	}
	VEC2 angle = VEC2subVEC2R(getCursorPosMap(),VEC2subVEC2R(player.pos,camera.pos));
	illuminateMapPart(PLAYER_LUMINANCE,player.pos,angle,128,0.15f);
	for(u4 i = 0;i < bullet.cnt;i++){
		illuminateMap(BULLET_LUMINANCE,bullet.state[i].pos,1024);
	}
	for(u4 i = 0;i < particle.cnt;i++){
		illuminateMap(particle.state[i].color,particle.state[i].pos,1024);
	}
	for(u4 i = 0;i < laser.cnt;i++){
		VEC2 normalize_pos = VEC2subVEC2R(laser.state[i].pos_dst,laser.state[i].pos_org);
		VEC2 direction = VEC2normalizeR(normalize_pos);
		u4 dst = VEC2length(normalize_pos);
		VEC2 pos = laser.state[i].pos_org;
		for(u4 j = 0;j < dst;j++){
			illuminateMap(LASER_LUMINANCE,pos,128);
			VEC2addVEC2(&pos,direction);
		}
	}
	for(u4 i = 0;i < camera.zoom*camera.zoom;i++){
		vram[i].r = tMinf(vramf[i].r,255.0f);
		vram[i].g = tMinf(vramf[i].g,255.0f);
		vram[i].b = tMinf(vramf[i].b,255.0f);
	}
	drawMap();
	glUseProgram(spriteShader);
	drawSprite(mapCrdToRenderCrd(player.pos),RD_SQUARE(PLAYER_SIZE),PLAYER_SPRITE);
	for(u4 i = 0;i < bullet.cnt;i++){
		drawSprite(mapCrdToRenderCrd(bullet.state[i].pos),RD_SQUARE(BULLET_SIZE),BULLET_SPRITE);
	}
	drawSprite(screenCrdToRenderCrd(getCursorPos()),RD_GUI(2.5f),CROSSHAIR_SPRITE);
	glUseProgram(enemyShader);
	for(u4 i = 0;i < enemy.cnt;i++){
		VEC2 relative_pos = VEC2subVEC2R(enemy.state[i].pos,camera.pos);
		if(relative_pos.x>0.0f&&relative_pos.x<camera.zoom&&relative_pos.y>0.0f&&relative_pos.y<camera.zoom){
			VEC3 luminance = vramf[(u4)relative_pos.y*camera.zoom+(u4)relative_pos.x];
			VEC3mul(&luminance,0.03f);
			drawEnemy(mapCrdToRenderCrd(enemy.state[i].pos),RD_SQUARE(ENEMY_SIZE),ENEMY_SPRITE,luminance);
		}
	}
	glUseProgram(particleShader);
	for(u4 i = 0;i < particle.cnt;i++){
		drawParticle(mapCrdToRenderCrd(particle.state[i].pos),RD_SQUARE(1.0f),VEC3normalizeR(particle.state[i].color));
	}
	glUseProgram(colorShader);
	drawRect((VEC2){0.13f,0.0f},(VEC2){0.01f,1.0f},(VEC3){0.7f,0.0f,0.0f});
	for(u4 i = 0;i < laser.cnt;i++){
		drawLaser(mapCrdToRenderCrd(laser.state[i].pos_org),mapCrdToRenderCrd(laser.state[i].pos_dst),RD_LASER_LUMINANCE);
	}
	if(player.weapon_cooldown){
		f4 progress = 0.01f-0.01f*player.weapon_cooldown/60.0f;
		VEC2 pos = mapCrdToRenderCrd(player.pos);
		pos.x += progress - 0.01f;
		pos.y += 0.05f;
		drawRect(pos,(VEC2){progress,0.005f},(VEC3){0.2f,0.5f,0.0f});
	}
	memset(vramf,0,sizeof(VEC3)*camera.zoom*camera.zoom);
}