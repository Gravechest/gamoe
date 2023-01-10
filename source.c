#include <windows.h>
#include <GL/gl.h>
#include <intrin.h>
#include <stdio.h>
#include <math.h>

#include "source.h"
#include "vec2.h"

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
u4 texture,map_texture;

QUAD quad = {
	.tc1={0.0f,0.0f},
	.tc2={0.0f,1.0f},
	.tc3={1.0f,0.0f},
	.tc4={1.0f,1.0f},
	.tc5={0.0f,1.0f},
	.tc6={1.0f,0.0f}
};

QUAD map_quad = {
	.p1={-1.0f,-1.0f},.tc1={0.0f,0.0f},
	.p2={-1.0f, 1.0f},.tc2={0.0f,1.0f},
	.p3={ 1.0f,-1.0f},.tc3={1.0f,0.0f},
	.p4={ 1.0f, 1.0f},.tc4={1.0f,1.0f},
	.p5={-1.0f, 1.0f},.tc5={0.0f,1.0f},
	.p6={ 1.0f,-1.0f},.tc6={1.0f,0.0f}
};

VEC3* vramf;
RGB*  vram;

u1* map;
PLAYER player = {.pos = {RES/2+RES,RES/2+RES}};

BULLETHUB bullet;
ENEMYHUB  enemy;
VEC2 camera = {RES,RES};
IVEC2 chunkPointer;
OPENGLQUEUE gl_queue;

u4 spriteShader,mapShader,enemyShader;

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

i4 hash(i4 x) {
	x += (x << 10);
	x ^= (x >> 6);
	x += (x << 3);
	x ^= (x >> 11);
	x += (x << 15);
	return x;
}

f4 rnd(){
	union p {
		f4 f;
		i4 u;
	}r;
	r.u = hash(__rdtsc());
	r.u &= 0x007fffff;
	r.u |= 0x3f800000;
	return r.f;
}

i4 irnd(){
	return hash(__rdtsc());
}

f4 fract(f4 p){
	return p - floorf(p);
}

i4 t_max(i4 p,i4 p2){
	return p > p2 ? p : p2;
}

i4 t_min(i4 p,i4 p2){
	return p < p2 ? p : p2;
}

f4 t_maxf(f4 p,f4 p2){
	return p > p2 ? p : p2;
}

f4 t_minf(f4 p,f4 p2){
	return p < p2 ? p : p2;
}

VEC2 mapCoordsToRenderCoords(VEC2 p){
	return (VEC2){(p.x-camera.x)/(RES/2.0f)-1.0f,(p.y-camera.y)/(RES/2.0f)-1.0f};
}

f4 iSquare(VEC2 ro,VEC2 rd,VEC2 boxSize) {
    VEC2 m = VEC2divFR(rd,1.0f);
    VEC2 n = VEC2mulVEC2R(m,ro);
    VEC2 k = VEC2mulVEC2R(VEC2absR(m),boxSize);
    VEC2 t1 = VEC2subVEC2R(VEC2subVEC2R(n,VEC2mulR(n,2.0f)),k);
    VEC2 t2 = VEC2addVEC2R(VEC2subVEC2R(n,VEC2mulR(n,2.0f)),k);
    f4 tN = t_maxf(t1.x,t1.y);
    f4 tF = t_minf(t2.x,t2.y);
    if(tN > tF || tF < 0.0f) {
        return -1.0f;
    }
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
	case WM_LBUTTONDOWN:{
	    POINT cursor;
		GetCursorPos(&cursor);
		ScreenToClient(window,&cursor);
		VEC2 vel = VEC2subVEC2R((VEC2){cursor.x/8.4375f,RES-cursor.y/8.4375f},VEC2subVEC2R(player.pos,camera));
		bullet.state[bullet.cnt].pos = player.pos;
		bullet.state[bullet.cnt++].vel = VEC2divR(VEC2normalizeR(vel),5.0f);
		break;
	}
	}
	return DefWindowProcA(hwnd,msg,wParam,lParam);
}

void illuminateMap(VEC3 color,VEC2 pos,u4 ammount){
	VEC2subVEC2(&pos,camera);
	if(pos.x < 0.0f || pos.y < 0.0f || pos.x > RES-1.0f || pos.y > RES-1.0f){
		for(u4 i = 0;i < ammount;i++){
			VEC2 direction = VEC2normalizeR((VEC2){cosf(i),sinf(i)});
			f4 dst = iSquare(pos,direction,(VEC2){RES,RES});
			if(dst != -1.0f){
				VEC2 ray_pos = VEC2addVEC2R(pos,VEC2mulR(direction,dst));
				RAY2D ray = ray2dCreate(ray_pos,direction);
				ray2dIterate(&ray);
				while(ray.roundPos.x > 0.0f && ray.roundPos.x < RES && ray.roundPos.y > 0.0f && ray.roundPos.y < RES){
					if(map[(ray.roundPos.y+(u4)camera.y)*MAP+(ray.roundPos.x+(u4)camera.x)]==1){
						vramf[ray.roundPos.y*RES+ray.roundPos.x].r+=color.r;
						vramf[ray.roundPos.y*RES+ray.roundPos.x].g+=color.g;
						vramf[ray.roundPos.y*RES+ray.roundPos.x].b+=color.b;
						break;
					}
					ray2dIterate(&ray);
				}
			}
		}
	}
	else{
		for(u4 i = 0;i < ammount;i++){
			RAY2D ray = ray2dCreate(pos,(VEC2){cosf(i),sinf(i)});
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
		if(map[(u4)(ray.roundPos.y)*RES+(u4)(ray.roundPos.x)]){
			return 0;	
		}
		ray2dIterate(&ray);
	}
	return 1;
}

void genMap(IVEC2 crd,u4 offset,u4 depth,f4 value){
	if(!depth){
		if(value > 0.0f) map[crd.x*MAP+crd.y+offset] = 1;
		return;
	}
	else{
		crd.x *= 2;
		crd.y *= 2;
		genMap((IVEC2){crd.x  ,crd.y  },offset,depth-1,value+rnd()-1.5f);
		genMap((IVEC2){crd.x+1,crd.y  },offset,depth-1,value+rnd()-1.5f);
		genMap((IVEC2){crd.x  ,crd.y+1},offset,depth-1,value+rnd()-1.5f);
		genMap((IVEC2){crd.x+1,crd.y+1},offset,depth-1,value+rnd()-1.5f);
	}
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
		collision(&player.pos,player.vel,1.25f);
		VEC2mul(&player.vel,PR_FRICTION);
		/*
		if(rnd()<1.02f && enemy.cnt < 8){
			enemy.state[enemy.cnt++].pos = (VEC2){(rnd()-1.0f)*RES,(rnd()-1.0f)*RES};
		}
		*/
		for(u4 i = 0;i < enemy.cnt;i++){
			if(enemy.state[i].pos.x < 1.0f || enemy.state[i].pos.x > RES-1.0f ||
			enemy.state[i].pos.y < 1.0f || enemy.state[i].pos.y > RES-1.0f){
				for(u4 j = i;j < enemy.cnt;j++){
					enemy.state[j] = enemy.state[j+1];
				}
				enemy.cnt--;
			}
			u4 iterations = (u4)fabsf(player.pos.x-enemy.state[i].pos.x)+(u4)fabsf(player.pos.y-enemy.state[i].pos.y);
			VEC2 toPlayer = VEC2normalizeR(VEC2subVEC2R(player.pos,enemy.state[i].pos));
			if(lineOfSight(enemy.state[i].pos,toPlayer,iterations)){
				VEC2div(&toPlayer,50.0f);
				VEC2addVEC2(&enemy.state[i].vel,toPlayer);
			}
			else if(rnd() < 1.05f){
				VEC2addVEC2(&enemy.state[i].vel,(VEC2){(rnd()-1.5f)/2.5f,(rnd()-1.5f)/2.5f});
			}
			VEC2addVEC2(&enemy.state[i].pos,enemy.state[i].vel);
			collision(&enemy.state[i].pos,enemy.state[i].vel,0.5f);
			VEC2mul(&enemy.state[i].vel,PR_FRICTION);	
		}
		for(u4 i = 0;i < bullet.cnt;i++){
			VEC2addVEC2(&bullet.state[i].pos,bullet.state[i].vel);
			if(bullet.state[i].pos.x < 0.0f || bullet.state[i].pos.x > MAP ||
			bullet.state[i].pos.y < 0.0f || bullet.state[i].pos.y > MAP ||
			map[(u4)bullet.state[i].pos.y*MAP+(u4)bullet.state[i].pos.x]){
				for(u4 j = i;j < bullet.cnt;j++){
					bullet.state[j] = bullet.state[j+1];
				}
				bullet.cnt--;
			}
		}
		Sleep(15);
	}
}

void drawSprite(VEC2 pos,VEC2 size,IVEC2 textureSize,RGB* texture){
	quad.p1 = (VEC2){pos.x-size.x,pos.y-size.y};
	quad.p2 = (VEC2){pos.x-size.x,pos.y+size.y};
	quad.p3 = (VEC2){pos.x+size.x,pos.y-size.y};
	quad.p4 = (VEC2){pos.x+size.x,pos.y+size.y};
	quad.p5 = (VEC2){pos.x-size.x,pos.y+size.y};
	quad.p6 = (VEC2){pos.x+size.x,pos.y-size.y};
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,textureSize.x,textureSize.y,0,GL_RGB,GL_UNSIGNED_BYTE,texture);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBufferData(GL_ARRAY_BUFFER,24 * sizeof(float),&quad,GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLES,0,24);
}

void drawEnemy(VEC2 pos,VEC2 size){
	quad.p1 = (VEC2){pos.x-size.x,pos.y-size.y};
	quad.p2 = (VEC2){pos.x-size.x,pos.y+size.y};
	quad.p3 = (VEC2){pos.x+size.x,pos.y-size.y};
	quad.p4 = (VEC2){pos.x+size.x,pos.y+size.y};
	quad.p5 = (VEC2){pos.x-size.x,pos.y+size.y};
	quad.p6 = (VEC2){pos.x+size.x,pos.y-size.y};
	glBufferData(GL_ARRAY_BUFFER,24 * sizeof(float),&quad,GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLES,0,24);
}

void drawMap(){
	glUseProgram(mapShader);
	glUniform2f(glGetUniformLocation(mapShader,"offset"),fract(camera.x)/RES,fract(camera.y)/RES);
	glUniform2f(glGetUniformLocation(mapShader,"camera"),camera.x/MAP,camera.y/MAP);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,RES,RES,0,GL_RGB,GL_UNSIGNED_BYTE,vram);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBufferData(GL_ARRAY_BUFFER,24 * sizeof(float),&map_quad,GL_DYNAMIC_DRAW);
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
	wglSwapIntervalEXT        = wglGetProcAddress("wglSwapIntervalEXT");

	wglSwapIntervalEXT(1);

	spriteShader = loadShader("shader/sprite.frag","shader/vertex.vert");
	mapShader    = loadShader("shader/map.frag"   ,"shader/vertex.vert");
	enemyShader  = loadShader("shader/enemy.frag" ,"shader/vertex.vert");

	glCreateBuffers(1,&VBO);
	glBindBuffer(GL_ARRAY_BUFFER,VBO);
	
	glGenTextures(1,&texture);
	glGenTextures(1,&map_texture);
	glBindTexture(GL_TEXTURE_2D,map_texture);
	glBindTexture(GL_TEXTURE_2D,texture);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glUseProgram(mapShader);
	glActiveTexture(GL_TEXTURE1);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D,0,GL_R8,MAP,MAP,0,GL_RED,GL_UNSIGNED_BYTE,map);
	glGenerateMipmap(GL_TEXTURE_2D);
	glUniform1i(glGetUniformLocation(mapShader,"map"),1);
	glActiveTexture(GL_TEXTURE0);

	glUseProgram(spriteShader);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,2,GL_FLOAT,0,4 * sizeof(float),(void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1,2,GL_FLOAT,0,4 * sizeof(float),(void*)(2 * sizeof(float)));

	for(;;){
		if(player.pos.x - RES/2 - 30.0f > camera.x){
			camera.x += player.pos.x - RES/2 - 30.0f - camera.x;
			if(camera.x > RES*2){
				printf("yeet\n");
				camera.x -= RES;
				player.pos.x -= RES;
				for(u4 x = 0;x < MAP;x++){
					for(u4 y = 0;y < RES*2;y++){
						map[x*MAP+y] = map[x*MAP+y+RES];
					}
				}
				for(u4 y = 0;y <= MAP*RES*2;y+=MAP*RES){
					genMap((IVEC2){0,0},y+RES*2,7,-1.0f);
				}
			}
		}
		if(player.pos.y - RES/2 - 30.0f > camera.y){
			camera.y += player.pos.y - RES/2 - 30.0f - camera.y;
			if(camera.y > RES*2){
				camera.y -= RES;
				player.pos.y -= RES;
				memcpy(map,map+MAP*RES,MAP*RES);
				memcpy(map+MAP*RES,map+MAP*RES*2,MAP*RES);
				for(u4 x = 0;x <= RES*2;x+=RES){
					genMap((IVEC2){0,0},x+MAP*RES*2,7,-1.0f);
				}
			}
		}
		if(player.pos.x - RES/2 + 30.0f < camera.x){
			camera.x += player.pos.x - RES/2 + 30.0f - camera.x;
			if(camera.x < 0.0f){
				camera.x += RES;
				player.pos.x += RES;
				for(u4 i = MAP*RES*2;i > 0;i--){
					map[i+RES] = map[i];
				}
				for(u4 y = 0;y <= MAP*RES*2;y+=MAP*RES){
					genMap((IVEC2){0,0},y+RES*2,7,-1.0f);
				}
			}
		}
		if(player.pos.y - RES/2 + 30.0f < camera.y){
			camera.y += player.pos.y - RES/2 + 30.0f - camera.y;
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
			}
		}
		for(u4 i = 0;i < MAP*MAP;i++){
			if(map[i]==2){
				illuminateMap((VEC3){0.1f,0.1f,1.0f},(VEC2){(f4)(i%MAP)+0.5f,(f4)(i/MAP)+0.5f},1024*4);
			}
		}
		illuminateMap((VEC3){0.2f,0.02f,0.02f},player.pos,1024);
		for(u4 i = 0;i < bullet.cnt;i++){
			illuminateMap((VEC3){0.1f,0.3f,0.1f},bullet.state[i].pos,1024);
		}
		for(u4 i = 0;i < RES*RES;i++){
			vram[i].r = t_minf(vramf[i].r,255.0f);
			vram[i].g = t_minf(vramf[i].g,255.0f);
			vram[i].b = t_minf(vramf[i].b,255.0f);
		}
		drawMap();
		glUseProgram(spriteShader);
		drawSprite(mapCoordsToRenderCoords(player.pos),(VEC2){0.018f,0.018f},(IVEC2){16,16},player.texture);
		for(u4 i = 0;i < bullet.cnt;i++){
			drawSprite(mapCoordsToRenderCoords(bullet.state[i].pos),(VEC2){0.01f,0.01f},(IVEC2){16,16},bullet.texture);
		}
		glUseProgram(enemyShader);
		for(u4 i = 0;i < enemy.cnt;i++){
			drawEnemy(mapCoordsToRenderCoords(enemy.state[i].pos),(VEC2){0.01f,0.01f});
		}
		memset(vramf,0,sizeof(VEC3)*RES*RES);
		SwapBuffers(dc);
	}
}

void main(){
	timeBeginPeriod(1);
	vram   = HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(RGB)*RES*RES);
	vramf  = HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(VEC3)*RES*RES);
	map    = HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,MAP*MAP);
	gl_queue.message = HeapAlloc(GetProcessHeap(),0,sizeof(OPENGLMESSAGE)*255);
	bullet.state   = HeapAlloc(GetProcessHeap(),0,sizeof(BULLET)*255);
	bullet.texture = HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(RGB)*16*16);
	player.texture = HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(RGB)*16*16);
	enemy.state    = HeapAlloc(GetProcessHeap(),0,sizeof(ENEMY)*255);
	wndclass.hInstance = GetModuleHandleA(0);
	RegisterClassA(&wndclass);
	window = CreateWindowExA(0,"class","hello",WS_VISIBLE | WS_POPUP,WNDOFFX,WNDOFFY,WNDY,WNDX,0,0,wndclass.hInstance,0);
	dc = GetDC(window);
	for(u4 x = 0;x <= RES*2;x+=RES){
		for(u4 y = 0;y <= MAP*RES*2;y+=MAP*RES){
			genMap((IVEC2){0,0},x+y,7,-1.0f);
		}
	}
	for(u4 x = 0;x < 16;x++){
		for(u4 y = 0;y < 16;y++){
			bullet.texture[x*16+y].g = t_max(255 - VEC2length((VEC2){fabsf(7.5f-x),fabsf(7.5f-y)}) * 32.0f,0);
			player.texture[x*16+y].r = 255 - VEC2length((VEC2){fabsf(7.5f-x),fabsf(7.5f-y)}) * 16.0f;
		}
	}
	CreateThread(0,0,render,0,0,0);
	CreateThread(0,0,physics,0,0,0);
	while(GetMessageA(&Msg,window,0,0)){
		TranslateMessage(&Msg);
		DispatchMessageA(&Msg);
	}
}