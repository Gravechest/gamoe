#pragma once

#include "vec2.h"
#include "small_types.h"
#include "ivec2.h"

#pragma comment(lib,"Winmm.lib")
#pragma comment(lib,"opengl32.lib")

#define PLAYER_SIZE 1.25f
#define CAM_AREA 0.0f

#define PI 3.141f

#define RD_CMP 0.562500f

#define RD_SQUARE(X) (VEC2){X*RD_CMP,X}

#define GL_FRAGMENT_SHADER 0x8B30
#define GL_VERTEX_SHADER 0x8B31
#define GL_ARRAY_BUFFER 0x8892
#define GL_DYNAMIC_DRAW 0x88E8

#define GL_TEXTURE0 0x84C0
#define GL_TEXTURE1 0x84C1
#define GL_TEXTURE2 0x84C2
#define GL_TEXTURE3 0x84C3
#define GL_TEXTURE4 0x84C4
#define GL_TEXTURE5 0x84C5
#define GL_TEXTURE6 0x84C6

#define GL_R8 0x8229

#define PR_FRICTION 0.9f

#define VK_W 0x57
#define VK_S 0x53
#define VK_A 0x41
#define VK_D 0x44
#define VK_L 0x4C

#define WNDOFFX 0
#define WNDOFFY 0

#define WNDX 1080
#define WNDY 1920

#define RES 128

#define MAP (RES*3)

u4 (*glCreateProgram)();
u4 (*glCreateShader)(u4 shader);
u4 (*wglSwapIntervalEXT)(u4 status);

i4 (*glGetUniformLocation)(u4 program,i1* name);

void (*glShaderSource)(u4 shader,i4 count,i1** string,i4* length);
void (*glCompileShader)(u4 shader);
void (*glAttachShader)(u4 program,u4 shader);
void (*glLinkProgram)(u4 program);
void (*glUseProgram)(u4 program);
void (*glEnableVertexAttribArray)(u4 index);
void (*glVertexAttribPointer)(u4 index,i4 size,u4 type,u1 normalized,u4 stride,void* pointer);
void (*glBufferData)(u4 target,u4 size,void* data,u4 usage);
void (*glCreateBuffers)(u4 n,u4 *buffers);
void (*glBindBuffer)(u4 target,u4 buffer);
void (*glGetShaderInfoLog)(u4 shader,u4 maxlength,u4 *length,u1 *infolog);
void (*glGenerateMipmap)(u4 target);
void (*glActiveTexture)(u4 texture);
void (*glUniform1i)(i4 loc,i4 v1);
void (*glUniform2f)(i4 loc,f4 v1,f4 v2);
void (*glUniform3f)(i4 loc,f4 v1,f4 v2,f4 v3);

typedef struct{
	union{
		f4 r;
		f4 x;
	};
	union{
		f4 g;
		f4 y;
	};
	union{
		f4 b;
		f4 z;
	};
}VEC3;

typedef struct{
	VEC2 p1;
	VEC2 tc1;
	VEC2 p2;
	VEC2 tc2;
	VEC2 p3;
	VEC2 tc3;
	VEC2 p4;
	VEC2 tc4;
	VEC2 p5;
	VEC2 tc5;
	VEC2 p6;
	VEC2 tc6;
}QUAD;

typedef struct{
	u1 r;
	u1 g;
	u1 b;
}RGB;

typedef struct{
	VEC2 vel;
	VEC2 pos;
	RGB* texture;
}PLAYER;

typedef struct{
	VEC2 vel;
	VEC2 pos;
}BULLET;

typedef struct{
	u4 cnt;
	BULLET* state;
	RGB* texture;
}BULLETHUB;

typedef struct{
	VEC2 vel;
	VEC2 pos;
}ENEMY;

typedef struct{
	u4 cnt;
	ENEMY* state;
}ENEMYHUB;

typedef struct{
	VEC2 pos;
	VEC2 dir;
	VEC2 delta;
	VEC2 side;

	IVEC2 step;
	IVEC2 roundPos;

	i4 hitSide;
}RAY2D;

typedef struct{
	VEC2 pos;
	VEC2 size;
	VEC3 color;
}COLORRECT;

typedef struct{
	u4 id;
	union{
		IVEC2 pos;
		COLORRECT rect;
	};
}OPENGLMESSAGE;

typedef struct{
	u4 cnt;
	OPENGLMESSAGE* message;
}OPENGLQUEUE;

extern u1* map;
extern BULLETHUB bullet;
extern ENEMYHUB  enemy;
extern PLAYER player;
extern VEC2 camera;