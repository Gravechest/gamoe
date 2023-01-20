#pragma once

#pragma comment(lib,"opengl32.lib")

#include "small_types.h"
#include "vec2.h"
#include "vec3.h"
#include "ivec2.h"

#define VSYNC 0

#define GUI_ENERGY (VEC2){0.18f,0.9f}
#define GUI_HEALTH (VEC2){0.18f,0.8f}
#define GUI_SCRAP  (VEC2){0.18f,0.7f}
#define GUI_EQUIPED (VEC2){GUI_INVENTORY.x,GUI_INVENTORY.y-0.3f}
#define GUI_INVENTORY (VEC2){0.24f,-0.55f}

#define RD_CMP 0.5625f
#define RD_CONVERT(X) (X*0.0072f)
#define RD_GUI(X) (VEC2){RD_CONVERT(X)*RD_CMP,RD_CONVERT(X)}
#define RD_SQUARE(X) VEC2mulR((VEC2){RD_CONVERT(X)*RD_CMP,RD_CONVERT(X)},128.0f/camera.zoom)

#define GL_ARRAY_BUFFER 0x8892

#define GL_FRAGMENT_SHADER 0x8B30
#define GL_VERTEX_SHADER 0x8B31

#define GL_TEXTURE0 0x84C0
#define GL_TEXTURE1 0x84C1
#define GL_TEXTURE2 0x84C2
#define GL_TEXTURE3 0x84C3
#define GL_TEXTURE4 0x84C4
#define GL_TEXTURE5 0x84C5
#define GL_TEXTURE6 0x84C6

#define GL_R8 0x8229

enum{
	GLMESSAGE_SINGLE_MAPEDIT,
	GLMESSAGE_WND_SIZECHANGE,
	GLMESSAGE_WHOLE_MAPEDIT
};

typedef struct{
	VEC2 pos;
	VEC2 size;
	VEC3 color;
}COLORRECT;

typedef struct{
	u4 id;
	union{
		IVEC2 pos;
	};
}OPENGLMESSAGE;

typedef struct{
	u4 cnt;
	OPENGLMESSAGE* message;
}OPENGLQUEUE;

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
void (*glUniform1f)(i4 loc,f4 v1);
void (*glUniform2f)(i4 loc,f4 v1,f4 v2);
void (*glUniform3f)(i4 loc,f4 v1,f4 v2,f4 v3);

void openglInit();
void opengl();

extern u4 sprite_shader,map_shader,entity_dark_shader,color_shader,particle_shader;
extern OPENGLQUEUE gl_queue;