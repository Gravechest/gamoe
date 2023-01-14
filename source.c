#include <windows.h>
#include <GL/gl.h>
#include <intrin.h>
#include <stdio.h>
#include <math.h>

#include "source.h"
#include "chunk.h"
#include "tmath.h"

i4 proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);

PIXELFORMATDESCRIPTOR pfd = {sizeof(PIXELFORMATDESCRIPTOR), 1,
PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
24,0, 0, 0, 0, 0, 0,0,0,0,
0,0,0,0,32,0,0,PFD_MAIN_PLANE,
0,0,0,0	};

HINSTANCE hinstance;
WNDCLASS wndclass = {.lpfnWndProc = proc,.lpszClassName = "class",.lpszMenuName = "class"};
HWND window;
HDC dc;
MSG Msg;
u4 VBO;
u4 texture,map_texture,sprite_texture;

QUAD quad = {
	.tc1={0.0f,0.0f},
	.tc2={0.0f,1.0f},
	.tc3={1.0f,0.0f},
	.tc4={1.0f,1.0f},
	.tc5={0.0f,1.0f},
	.tc6={1.0f,0.0f}
};

QUAD sprite_quad;

QUAD map_quad = {
	.p1={-1.0f  ,-1.0f  },.tc1={0.0f,0.0f},
	.p2={-1.0f  , 1.0f  },.tc2={0.0f,1.0f},
	.p3={ 0.125f,-1.0f  },.tc3={1.0f,0.0f},
	.p4={ 0.125f, 1.0f  },.tc4={1.0f,1.0f},
	.p5={-1.0f  , 1.0f  },.tc5={0.0f,1.0f},
	.p6={ 0.125f,-1.0f  },.tc6={1.0f,0.0f}
};

VEC3* vramf;
RGB*  vram;

u1* map;
PLAYER player = {.pos = {RES/2+RES,RES/2+RES}};

BULLETHUB bullet;
ENEMYHUB  enemy;
LASERHUB  laser;
PARTICLEHUB particle;
VEC2 camera = {RES,RES};
OPENGLQUEUE gl_queue;
RGB* texture16;

u4 spriteShader,mapShader,enemyShader,colorShader;

RAY2D ray2dCreate(VEC2 pos,VEC2 dir){
	RAY2D ray;

	ray.pos = pos;
	ray.dir = dir;
	ray.delta = VEC2absR(VEC2divFR(ray.dir,1.0f));

	if(ray.dir.x < 0.0f){
		ray.step.x = -1;
		ray.side.x = (ray.pos.x-(int)ray.pos.x) * ray.delta.x;
	}
	else{
		ray.step.x = 1;
		ray.side.x = ((int)ray.pos.x + 1.0f - ray.pos.x) * ray.delta.x;
	}
	if(ray.dir.y < 0.0f){
		ray.step.y = -1;
		ray.side.y = (ray.pos.y-(int)ray.pos.y) * ray.delta.y;
	}
	else{
		ray.step.y = 1;
		ray.side.y = ((int)ray.pos.y + 1.0f - ray.pos.y) * ray.delta.y;
	}
	ray.roundPos.x = ray.pos.x;
	ray.roundPos.y = ray.pos.y;
	return ray;
}

void ray2dIterate(RAY2D *ray){
	if(ray->side.x < ray->side.y){
		ray->roundPos.x += ray->step.x;
		ray->side.x += ray->delta.x;
		ray->hitSide = 0;
	}
	else{
		ray->roundPos.y += ray->step.y;
		ray->side.y += ray->delta.y;
		ray->hitSide = 1;
	}
}

VEC2 ray2dGetCoords(RAY2D ray){
	f4 wall;
	switch(ray.hitSide){
	case 0:
		wall = ray.pos.y + (ray.side.x - ray.delta.x) * ray.dir.y;
		if(ray.dir.x > 0.0f) return (VEC2){ray.roundPos.x     ,wall};
		else                 return (VEC2){ray.roundPos.x+1.0f,wall};
	case 1:
		wall = ray.pos.x + (ray.side.y - ray.delta.y) * ray.dir.x;
		if(ray.dir.y > 0.0f) return (VEC2){wall,ray.roundPos.y     };
		else                 return (VEC2){wall,ray.roundPos.y+1.0f};
	}
}

VEC3 VEC3mulR(VEC3 p,f4 m){
	return (VEC3){p.x*m,p.y*m,p.z*m};
}

VEC3 VEC3addVEC3(VEC3* p,VEC3 p2){
	p->x += p2.x;
	p->y += p2.y;
	p->z += p2.z;
}

f4 VEC3length(VEC3 p){
	return sqrtf(p.x*p.x+p.y*p.y+p.z*p.z);
}

VEC3 VEC3normalizeR(VEC3 p){
	f4 l = VEC3length(p);
	return (VEC3){p.x/l,p.y/l,p.z/l};
}

f4 invSqrtf(f4 p){
	return 1.0f/sqrtf(p);
}

VEC2 mapCrdToRenderCrd(VEC2 p){
	return (VEC2){((p.x-camera.x)/(RES/2.0f/RD_CMP)-1.0f),(p.y-camera.y)/(RES/2.0f)-1.0f};
}

VEC2 screenCrdToRenderCrd(VEC2 p){
	return (VEC2){((p.x)/(RES/2.0f/RD_CMP)-1.0f),(p.y)/(RES/2.0f)-1.0f};
}

void genMap(IVEC2 crd,u4 offset,u4 depth,f4 value){
	if(!depth){
		if(value > 0.0f) map[crd.x*MAP+crd.y+offset] = 1;
		return;
	}
	else{
		crd.x *= 2;
		crd.y *= 2;
		genMap((IVEC2){crd.x  ,crd.y  },offset,depth-1,value+tRnd()-1.5f);
		genMap((IVEC2){crd.x+1,crd.y  },offset,depth-1,value+tRnd()-1.5f);
		genMap((IVEC2){crd.x  ,crd.y+1},offset,depth-1,value+tRnd()-1.5f);
		genMap((IVEC2){crd.x+1,crd.y+1},offset,depth-1,value+tRnd()-1.5f);
	}
}

f4 iSquare(VEC2 ro,VEC2 rd,f4 size){
	VEC2sub(&ro,size);
	VEC2 delta = VEC2divFR(rd,1.0f);
	VEC2 n = VEC2mulVEC2R(delta,ro);
	VEC2 k = VEC2mulR(VEC2absR(delta),size);
	VEC2 t1= VEC2subVEC2R(VEC2negR(n),k);
	VEC2 t2= VEC2addVEC2R(VEC2negR(n),k);
	f4 tN = tMaxf(t1.x,t1.y);
	f4 tF = tMinf(t2.x,t2.y);
	if(tN>tF||tF<0.0f) return -1.0f;
	return tN;
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

VEC2 getCursorPos(){
	POINT cursor;
	GetCursorPos(&cursor);
	ScreenToClient(window,&cursor);
	return PLAYER_MOUSE(cursor);
}

i4 proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam){
	switch(msg){
	case WM_KEYDOWN:
		switch(wParam){
		case VK_L:
			map[(u4)player.pos.y*MAP+(u4)player.pos.x] = 2;
			gl_queue.message[gl_queue.cnt].id = 0;
			gl_queue.message[gl_queue.cnt++].pos = (IVEC2){player.pos.x,player.pos.y};
			break;
		}
		break;
	case WM_RBUTTONDOWN:
		if(player.lightBulletCnt){
			VEC2 direction = VEC2subVEC2R(getCursorPos(),VEC2subVEC2R(player.pos,camera));
			bullet.state[bullet.cnt].pos = player.pos;
			bullet.state[bullet.cnt++].vel = VEC2divR(VEC2normalizeR(direction),5.0f);
			player.lightBulletCnt--;
		}
		break;
	case WM_LBUTTONDOWN:
		if(!player.weaponCooldown){
			VEC2 direction = VEC2subVEC2R(getCursorPos(),VEC2subVEC2R(player.pos,camera));
			RAY2D ray = ray2dCreate(player.pos,direction);
			while(ray.roundPos.x >= 0 && ray.roundPos.x < MAP && ray.roundPos.y >= 0 && ray.roundPos.y < MAP){
				if(map[ray.roundPos.y*MAP+ray.roundPos.x]==1){
					break;
				}
				for(u4 i = 0;i < enemy.cnt;i++){
					if((u4)enemy.state[i].pos.x == ray.roundPos.x && (u4)enemy.state[i].pos.y == ray.roundPos.y){
						for(u4 j = 0;j < 64;j++){
							particle.state[particle.cnt].color = (VEC3){(tRnd()-1.0f)*0.125f,(tRnd()-1.0f)*0.125f,(tRnd()-1.0f)*0.125f};
							particle.state[particle.cnt].vel = (VEC2){(tRnd()-1.5f)*0.25f,(tRnd()-1.5f)*0.25f};
							particle.state[particle.cnt].health = tRnd()*200.0f;
							particle.state[particle.cnt++].pos = enemy.state[i].pos;
						}
						for(u4 j = i;j < enemy.cnt;j++) enemy.state[j] = enemy.state[j+1];
						enemy.cnt--;
						goto end;
					}
				}
				ray2dIterate(&ray);
			}
		end:
			laser.state[laser.cnt].pos_org = player.pos;
			laser.state[laser.cnt].health = 5;
			laser.state[laser.cnt++].pos_dst = ray2dGetCoords(ray);
			player.weaponCooldown = PLAYER_WEAPON_COOLDOWN;
		}
		break;
	}
	return DefWindowProcA(hwnd,msg,wParam,lParam);
}

void illuminateMap(VEC3 color,VEC2 pos,u4 ammount){
	VEC2subVEC2(&pos,(VEC2){(u4)camera.x,(u4)camera.y});
	f4 offset = tRnd();
	if(pos.x < 1.0f || pos.y < 1.0f || pos.x > RES || pos.y > RES){
		for(f4 i = offset;i < 1.0f+offset;i+=1.0f/ammount){
			VEC2 direction = (VEC2){cosf(i*PI*2.0f),sinf(i*PI*2.0f)};
			f4 dst = iSquare(pos,direction,RES/2.0f);
			if(dst > 0.0f){
				RAY2D ray = ray2dCreate(pos,direction);
				while((ray.roundPos.x < 0 || ray.roundPos.x >= RES || ray.roundPos.y < 0 || ray.roundPos.y >= RES)){
					IVEC2 block = {(ray.roundPos.x+(u4)camera.x),(ray.roundPos.y+(u4)camera.y)};
					if(block.y*MAP+block.x<0||block.y*MAP+block.x>MAP*MAP||map[block.y*MAP+block.x]==1){
						break;
					}
					ray2dIterate(&ray);
				}
				while(ray.roundPos.x >= 0 && ray.roundPos.x < RES && ray.roundPos.y >= 0 && ray.roundPos.y < RES){
					if(map[(ray.roundPos.y+(u4)camera.y)*MAP+(ray.roundPos.x+(u4)camera.x)]==1){
						vramf[ray.roundPos.y*RES+ray.roundPos.x].r+=color.r*4.0f;
						vramf[ray.roundPos.y*RES+ray.roundPos.x].g+=color.g*4.0f;
						vramf[ray.roundPos.y*RES+ray.roundPos.x].b+=color.b*4.0f;
						break;
					}
					vramf[ray.roundPos.y*RES+ray.roundPos.x].r+=color.r;
					vramf[ray.roundPos.y*RES+ray.roundPos.x].g+=color.g;
					vramf[ray.roundPos.y*RES+ray.roundPos.x].b+=color.b;
					ray2dIterate(&ray);
				}
			}
		}
	}
	else{
		for(f4 i = offset;i < 1.0f+offset;i+=1.0f/ammount){
			RAY2D ray = ray2dCreate(pos,(VEC2){cosf(i*PI*2.0f),sinf(i*PI*2.0f)});
			while(ray.roundPos.x > 0.0f && ray.roundPos.x < RES && ray.roundPos.y > 0.0f && ray.roundPos.y < RES){
				if(map[(ray.roundPos.y+(u4)camera.y)*MAP+(ray.roundPos.x+(u4)camera.x)]==1){
					vramf[ray.roundPos.y*RES+ray.roundPos.x].r+=color.r*4.0f;
					vramf[ray.roundPos.y*RES+ray.roundPos.x].g+=color.g*4.0f;
					vramf[ray.roundPos.y*RES+ray.roundPos.x].b+=color.b*4.0f;
					break;
				}
				vramf[ray.roundPos.y*RES+ray.roundPos.x].r+=color.r;
				vramf[ray.roundPos.y*RES+ray.roundPos.x].g+=color.g;
				vramf[ray.roundPos.y*RES+ray.roundPos.x].b+=color.b;
				ray2dIterate(&ray);
			}
		}	
	}
}

void collision(VEC2* pos,VEC2 vel,f4 size){
	f4 inc = size;
	while(inc > 1.0f) inc *= 0.5f;
	if(vel.x < 0.0f){
		for(f4 i = pos->y - size;i <= pos->y + size;i+=inc){
			if(map[(u4)i*MAP+(u4)(pos->x-size)]){
				pos->x -= vel.x;
				vel.x = 0.0f;
				break;
			}
		}
	}
	else{
		for(f4 i = pos->y - size;i <= pos->y + size;i+=inc){
			if(map[(u4)i*MAP+(u4)(pos->x+size)]){
				pos->x -= vel.x;
				vel.x = 0.0f;
				break;
			}
		}
	}
	if(vel.y < 0.0f){
		for(f4 i = pos->x - size;i <= pos->x + size;i+=inc){
			if(map[(u4)(pos->y-size)*MAP+(u4)i]){
				pos->y -= vel.y;
				vel.y = 0.0f;
				break;
			}
		}
	}
	else{
		for(f4 i = pos->x - size;i <= pos->x + size;i+=inc){
			if(map[(u4)(pos->y+size)*MAP+(u4)i]){
				pos->y -= vel.y;
				vel.y = 0.0f;
				break;
			}
		}
	}
}

u1 lineOfSight(VEC2 pos,VEC2 dir,u4 iterations){
	RAY2D ray = ray2dCreate(pos,dir);
	for(u4 i = 0;i < iterations;i++){
		if(map[(u4)(ray.roundPos.y)*MAP+(u4)(ray.roundPos.x)]){
			return 0;	
		}
		ray2dIterate(&ray);
	}
	return 1;
}

void physics(){
	for(;;){
		u1 key_w = GetKeyState(VK_W) & 0x80;
		u1 key_a = GetKeyState(VK_A) & 0x80;
		u1 key_s = GetKeyState(VK_S) & 0x80;
		u1 key_d = GetKeyState(VK_D) & 0x80;
		if(key_w){
			if(key_d || key_a) player.vel.y+=0.04f * 0.7f;
			else               player.vel.y+=0.04f;
		}
		if(key_s){
			if(key_d || key_a) player.vel.y-=0.04f * 0.7f;
			else               player.vel.y-=0.04f;
		}
		if(key_d){
			if(key_s || key_w) player.vel.x+=0.04f * 0.7f;
			else               player.vel.x+=0.04f;
		}
		if(key_a){
			if(key_s || key_w) player.vel.x-=0.04f * 0.7f;
			else               player.vel.x-=0.04f;
		}
		VEC2addVEC2(&player.pos,player.vel);
		collision(&player.pos,player.vel,PLAYER_SIZE/2.0f);
		VEC2mul(&player.vel,PR_FRICTION);
		if(tRnd()<1.02f && enemy.cnt < 8){
			enemy.state[enemy.cnt++].pos = (VEC2){tRnd()*RES,tRnd()*RES};
		}
		for(u4 i = 0;i < enemy.cnt;i++){
			if(enemy.state[i].pos.x < 1.0f || enemy.state[i].pos.x > RES*3-1.0f ||
			enemy.state[i].pos.y < 1.0f || enemy.state[i].pos.y > RES*3-1.0f){
				for(u4 j = i;j < enemy.cnt;j++) enemy.state[j] = enemy.state[j+1];
				enemy.cnt--;
				i--;
				continue;
			}
			u4 iterations = (u4)fabsf(player.pos.x-enemy.state[i].pos.x)+(u4)fabsf(player.pos.y-enemy.state[i].pos.y);
			VEC2 toPlayer = VEC2normalizeR(VEC2subVEC2R(player.pos,enemy.state[i].pos));
			if(lineOfSight(enemy.state[i].pos,toPlayer,iterations)){
				VEC2div(&toPlayer,50.0f);
				VEC2addVEC2(&enemy.state[i].vel,toPlayer);
			}
			else if(tRnd() < 1.05f){
				VEC2addVEC2(&enemy.state[i].vel,(VEC2){(tRnd()-1.5f)/2.5f,(tRnd()-1.5f)/2.5f});
			}
			VEC2addVEC2(&enemy.state[i].pos,enemy.state[i].vel);
			collision(&enemy.state[i].pos,enemy.state[i].vel,ENEMY_SIZE/2.0f);
			VEC2mul(&enemy.state[i].vel,PR_FRICTION);	
		}
		for(u4 i = 0;i < bullet.cnt;i++){
			VEC2addVEC2(&bullet.state[i].pos,bullet.state[i].vel);
			if(bullet.state[i].pos.x < 0.0f || bullet.state[i].pos.x > MAP ||
			bullet.state[i].pos.y < 0.0f || bullet.state[i].pos.y > MAP ||
			map[(u4)bullet.state[i].pos.y*MAP+(u4)bullet.state[i].pos.x]){
				for(u4 j = i;j < bullet.cnt;j++) bullet.state[j] = bullet.state[j+1];
				bullet.cnt--;
			}
		}
		for(u4 i = 0;i < laser.cnt;i++){
			if(laser.state[i].health--){
				VEC2addVEC2(&laser.state[i].pos_org,player.vel);
			}
			else{
				for(u4 j = i;j < laser.cnt;j++) laser.state[j] = laser.state[j+1];
				laser.cnt--;
			}
		}
		for(u4 i = 0;i < particle.cnt;i++){
			if(particle.state[i].health--){
				VEC2addVEC2(&particle.state[i].pos,particle.state[i].vel);
			}
			else{
				for(u4 j = i;j < particle.cnt;j++) particle.state[j] = particle.state[j+1];
				particle.cnt--;
			}
		}
		if(player.weaponCooldown) player.weaponCooldown--;
		Sleep(15);
	}
}

void drawRect(VEC2 pos,VEC2 size,VEC3 color){
	quad.p1 = (VEC2){pos.x-size.x,pos.y-size.y};
	quad.p2 = (VEC2){pos.x-size.x,pos.y+size.y};
	quad.p3 = (VEC2){pos.x+size.x,pos.y-size.y};
	quad.p4 = (VEC2){pos.x+size.x,pos.y+size.y};
	quad.p5 = (VEC2){pos.x-size.x,pos.y+size.y};
	quad.p6 = (VEC2){pos.x+size.x,pos.y-size.y};
	glUniform3f(glGetUniformLocation(colorShader,"color"),color.r,color.g,color.b);
	glBufferData(GL_ARRAY_BUFFER,24 * sizeof(float),&quad,GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLES,0,24);
}

void drawSprite(VEC2 pos,VEC2 size,VEC2 texture_pos){
	sprite_quad.p1 = (VEC2){pos.x-size.x,pos.y-size.y};
	sprite_quad.p2 = (VEC2){pos.x-size.x,pos.y+size.y};
	sprite_quad.p3 = (VEC2){pos.x+size.x,pos.y-size.y};
	sprite_quad.p4 = (VEC2){pos.x+size.x,pos.y+size.y};
	sprite_quad.p5 = (VEC2){pos.x-size.x,pos.y+size.y};
	sprite_quad.p6 = (VEC2){pos.x+size.x,pos.y-size.y};
	sprite_quad.tc1 = (VEC2){texture_pos.x     ,texture_pos.y     };
	sprite_quad.tc2 = (VEC2){texture_pos.x     ,texture_pos.y+0.5f};
	sprite_quad.tc3 = (VEC2){texture_pos.x+0.5f,texture_pos.y     };
	sprite_quad.tc4 = (VEC2){texture_pos.x+0.5f,texture_pos.y+0.5f};
	sprite_quad.tc5 = (VEC2){texture_pos.x     ,texture_pos.y+0.5f};
	sprite_quad.tc6 = (VEC2){texture_pos.x+0.5f,texture_pos.y     };
	glBufferData(GL_ARRAY_BUFFER,24 * sizeof(float),&sprite_quad,GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLES,0,24);
}

void drawEnemy(VEC2 pos,VEC2 size,VEC3 luminance){
	quad.p1 = (VEC2){pos.x-size.x,pos.y-size.y};
	quad.p2 = (VEC2){pos.x-size.x,pos.y+size.y};
	quad.p3 = (VEC2){pos.x+size.x,pos.y-size.y};
	quad.p4 = (VEC2){pos.x+size.x,pos.y+size.y};
	quad.p5 = (VEC2){pos.x-size.x,pos.y+size.y};
	quad.p6 = (VEC2){pos.x+size.x,pos.y-size.y};
	glUniform3f(glGetUniformLocation(enemyShader,"luminance"),luminance.r,luminance.g,luminance.b);
	glBufferData(GL_ARRAY_BUFFER,24 * sizeof(float),&quad,GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLES,0,24);
}

void drawMap(){
	glUseProgram(mapShader);
	glUniform2f(glGetUniformLocation(mapShader,"offset"),tFract(camera.x)/RES,tFract(camera.y)/RES);
	glUniform2f(glGetUniformLocation(mapShader,"camera"),camera.x/MAP,camera.y/MAP);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,RES,RES,0,GL_RGB,GL_UNSIGNED_BYTE,vram);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBufferData(GL_ARRAY_BUFFER,24 * sizeof(float),&map_quad,GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLES,0,24);
}

void drawLaser(VEC2 origin,VEC2 destination,VEC3 color){
	VEC2 direction = VEC2normalizeR(VEC2subVEC2R(destination,origin));
	VEC2rot(&direction,PI/2.0f);
	VEC2div(&direction,512.0f);
	quad.p1 = (VEC2){origin.x-     direction.x,origin.y-     direction.y};
	quad.p2 = (VEC2){origin.x+     direction.x,origin.y+     direction.y};
	quad.p3 = (VEC2){destination.x+direction.x,destination.y+direction.y};
	quad.p4 = (VEC2){destination.x-direction.x,destination.y-direction.y};
	quad.p5 = (VEC2){destination.x+direction.x,destination.y+direction.y};
	quad.p6 = (VEC2){origin.x-     direction.x,origin.y-     direction.y};
	glUniform3f(glGetUniformLocation(colorShader,"color"),color.r,color.g,color.b);
	glBufferData(GL_ARRAY_BUFFER,24 * sizeof(float),&quad,GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLES,0,24);
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

VEC3 entityLuminance(VEC2 pos,VEC2 pos2,VEC3 luminance){
	u4 iterations = (u4)fabsf(pos.x-pos2.x)+(u4)fabsf(pos.y-pos2.y);
	VEC2 toBullet = VEC2normalizeR(VEC2subVEC2R(pos2,pos));
	if(lineOfSight(pos,toBullet,iterations)){
		f4 dst = VEC2length(VEC2subVEC2R(pos,pos2));
		return VEC3mulR(luminance,invSqrtf(dst));
	}
	return (VEC3){0.0f,0.0f,0.0f};
}

VEC3 calculateLuminance(VEC2 pos){
	VEC3 color = {0.0f,0.0f,0.0f};
	VEC3addVEC3(&color,entityLuminance(pos,player.pos,PLAYER_LUMINANCE));
	for(u4 i = 0;i < bullet.cnt;i++){
		VEC3addVEC3(&color,entityLuminance(pos,bullet.state[i].pos,BULLET_LUMINANCE));
	}
	for(u4 i = 0;i < particle.cnt;i++){
		VEC3addVEC3(&color,entityLuminance(pos,particle.state[i].pos,particle.state[i].color));
	}
	for(u4 i = 0;i < laser.cnt;i++){
		VEC2 normalize_pos = VEC2subVEC2R(laser.state[i].pos_dst,laser.state[i].pos_org);
		VEC2 direction = VEC2normalizeR(normalize_pos);
		u4 dst = VEC2length(normalize_pos);
		VEC2 laser_pos = laser.state[i].pos_org;
		for(u4 j = 0;j < dst;j++){
			VEC3addVEC3(&color,entityLuminance(pos,laser_pos,ENT_LASER_LUMINANCE));
			VEC2addVEC2(&laser_pos,direction);
		}
	}
	return color;
}

void render(){
	SetPixelFormat(dc,ChoosePixelFormat(dc,&pfd),&pfd);
	wglMakeCurrent(dc,wglCreateContext(dc));

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

	wglSwapIntervalEXT(0);

	spriteShader = loadShader("shader/sprite.frag","shader/vertex.vert");
	mapShader    = loadShader("shader/map.frag"   ,"shader/vertex.vert");
	enemyShader  = loadShader("shader/enemy.frag" ,"shader/vertex.vert");
	colorShader  = loadShader("shader/color.frag" ,"shader/vertex.vert");

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

	glUseProgram(colorShader);
	drawRect((VEC2){0.1f,0.0f},(VEC2){0.03f,1.0f},(VEC3){1.0f,0.0f,0.0f});
	SwapBuffers(dc);

	for(;;){
		if(player.pos.x - RES/2 - CAM_AREA > camera.x){
			camera.x += player.pos.x - RES/2 - CAM_AREA - camera.x;
			if(camera.x > RES+RES/2) worldLoadEast();
		}
		if(player.pos.y - RES/2 - CAM_AREA > camera.y){
			camera.y += player.pos.y - RES/2 - CAM_AREA - camera.y;
			if(camera.y > RES+RES/2) worldLoadNorth();
		}
		if(player.pos.x - RES/2 + CAM_AREA < camera.x){
			camera.x += player.pos.x - RES/2 + CAM_AREA - camera.x;
			if(camera.x < RES-RES/2) worldLoadWest();
		}
		if(player.pos.y - RES/2 + CAM_AREA < camera.y){
			camera.y += player.pos.y - RES/2 + CAM_AREA - camera.y;
			if(camera.y < RES-RES/2) worldLoadSouth();
		}
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
		illuminateMap(PLAYER_LUMINANCE,player.pos,1024);
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
		for(u4 i = 0;i < RES*RES;i++){
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
		drawSprite(screenCrdToRenderCrd(getCursorPos()),RD_SQUARE(2.5f),CROSSHAIR_SPRITE);
		glUseProgram(enemyShader);
		for(u4 i = 0;i < enemy.cnt;i++){
			VEC3 luminance = calculateLuminance(enemy.state[i].pos);
			drawEnemy(mapCrdToRenderCrd(enemy.state[i].pos),RD_SQUARE(ENEMY_SIZE),luminance);
		}
		for(u4 i = 0;i < particle.cnt;i++){
			drawEnemy(mapCrdToRenderCrd(particle.state[i].pos),RD_SQUARE(1.0f),VEC3normalizeR(particle.state[i].color));
		}
		glUseProgram(colorShader);
		drawRect((VEC2){0.13f,0.0f},(VEC2){0.01f,1.0f},(VEC3){0.7f,0.0f,0.0f});
		for(u4 i = 0;i < laser.cnt;i++){
			drawLaser(mapCrdToRenderCrd(laser.state[i].pos_org),mapCrdToRenderCrd(laser.state[i].pos_dst),RD_LASER_LUMINANCE);
		}
		if(player.weaponCooldown){
			f4 progress = 0.01f-0.01f*player.weaponCooldown/60.0f;
			VEC2 pos = mapCrdToRenderCrd(player.pos);
			pos.x += progress - 0.01f;
			pos.y += 0.05f;
			drawRect(pos,(VEC2){progress,0.005f},(VEC3){0.2f,0.5f,0.0f});
		}
		memset(vramf,0,sizeof(VEC3)*RES*RES);
		SwapBuffers(dc);
		glClear(GL_COLOR_BUFFER_BIT);
	}
}

void main(){
	timeBeginPeriod(1);
	ShowCursor(0);
	vram   = HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(RGB)*RES*RES);
	vramf  = HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(VEC3)*RES*RES);
	map    = HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,MAP*MAP);
	gl_queue.message = HeapAlloc(GetProcessHeap(),0,sizeof(OPENGLMESSAGE)*255);
	bullet.state = HeapAlloc(GetProcessHeap(),0,sizeof(BULLET)*255);
	enemy.state  = HeapAlloc(GetProcessHeap(),0,sizeof(ENEMY)*255);
	chunk.state  = HeapAlloc(GetProcessHeap(),0,sizeof(CHUNK)*255);
	laser.state  = HeapAlloc(GetProcessHeap(),0,sizeof(LASER)*255);
	particle.state = HeapAlloc(GetProcessHeap(),0,sizeof(PARTICLE)*255);
	texture16 = HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(RGB)*TEXTURE16_SIZE*TEXTURE16_SIZE);
	wndclass.hInstance = GetModuleHandleA(0);
	RegisterClassA(&wndclass);
	window = CreateWindowExA(0,"class","hello",WS_VISIBLE | WS_POPUP,WNDOFFX,WNDOFFY,WNDY,WNDX,0,0,wndclass.hInstance,0);
	dc = GetDC(window);
	for(u4 x = 0;x <= RES*2;x+=RES){
		for(u4 y = 0;y <= MAP*RES*2;y+=MAP*RES){
			genMap((IVEC2){0,0},x+y,7,-1.0f);
		}
	}
	for(u4 x = player.pos.x - 3.0f;x < player.pos.x + 3.0f;x++){
		for(u4 y = player.pos.y - 3.0f;y < player.pos.y + 3.0f;y++){
			map[x*MAP+y] = 0;
		}
	}
	for(u4 x = 0;x < 16;x++){
		for(u4 y = 0;y < 16;y++){
			texture16[x*TEXTURE16_SIZE+y].r = 255 - VEC2length((VEC2){fabsf(7.5f-x),fabsf(7.5f-y)}) * 16.0f;
			texture16[x*TEXTURE16_SIZE+y+16].g = tMaxf(255 - VEC2length((VEC2){fabsf(7.5f-x),fabsf(7.5f-y)}) * 32.0f,0);
		}
	}
	//generate crosshair
	for(u4 i = 0;i < 16;i++){
		texture16[i*TEXTURE16_SIZE+7+TEXTURE16_SIZE*16].r = 255;
		texture16[i*TEXTURE16_SIZE+8+TEXTURE16_SIZE*16].r = 255;
		texture16[7*TEXTURE16_SIZE+i+TEXTURE16_SIZE*16].r = 255;
		texture16[8*TEXTURE16_SIZE+i+TEXTURE16_SIZE*16].r = 255;
	}
	CreateThread(0,0,render,0,0,0);
	CreateThread(0,0,physics,0,0,0);
	while(GetMessageA(&Msg,window,0,0)){
		TranslateMessage(&Msg);
		DispatchMessageA(&Msg);
	}
}