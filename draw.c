#include <windows.h>
#include <GL/gl.h>

#include "draw.h"
#include "tmath.h"
#include "opengl.h"
#include "source.h"

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

void drawSprite(VEC2 pos,VEC2 size,VEC2 texture_pos){
	sprite_quad.p1 = (VEC2){pos.x-size.x,pos.y-size.y};
	sprite_quad.p2 = (VEC2){pos.x-size.x,pos.y+size.y};
	sprite_quad.p3 = (VEC2){pos.x+size.x,pos.y-size.y};
	sprite_quad.p4 = (VEC2){pos.x+size.x,pos.y+size.y};
	sprite_quad.p5 = (VEC2){pos.x-size.x,pos.y+size.y};
	sprite_quad.p6 = (VEC2){pos.x+size.x,pos.y-size.y};
	sprite_quad.tc1 = (VEC2){texture_pos.x                  ,texture_pos.y                  };
	sprite_quad.tc2 = (VEC2){texture_pos.x                  ,texture_pos.y+TEXTURE16_RD_SIZE};
	sprite_quad.tc3 = (VEC2){texture_pos.x+TEXTURE16_RD_SIZE,texture_pos.y                  };
	sprite_quad.tc4 = (VEC2){texture_pos.x+TEXTURE16_RD_SIZE,texture_pos.y+TEXTURE16_RD_SIZE};
	sprite_quad.tc5 = (VEC2){texture_pos.x                  ,texture_pos.y+TEXTURE16_RD_SIZE};
	sprite_quad.tc6 = (VEC2){texture_pos.x+TEXTURE16_RD_SIZE,texture_pos.y                  };
	glBufferData(GL_ARRAY_BUFFER,24 * sizeof(float),&sprite_quad,GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLES,0,24);
}

void drawString(VEC2 pos,VEC2 size,u1* string){
	for(u4 i = 0;string[i] != '\0';i++){
		VEC2 texture_pos = {(f4)((string[i]-0x17)%10)/10.0f,1.0f-(f4)((string[i]-0x17)/10)/10.0f};
		sprite_quad.p1 = (VEC2){pos.x+size.x*i*2.0f-size.x,pos.y-size.y};
		sprite_quad.p2 = (VEC2){pos.x+size.x*i*2.0f-size.x,pos.y+size.y};
		sprite_quad.p3 = (VEC2){pos.x+size.x*i*2.0f+size.x,pos.y-size.y};
		sprite_quad.p4 = (VEC2){pos.x+size.x*i*2.0f+size.x,pos.y+size.y};
		sprite_quad.p5 = (VEC2){pos.x+size.x*i*2.0f-size.x,pos.y+size.y};
		sprite_quad.p6 = (VEC2){pos.x+size.x*i*2.0f+size.x,pos.y-size.y};
		sprite_quad.tc1 = (VEC2){texture_pos.x     ,texture_pos.y     };
		sprite_quad.tc2 = (VEC2){texture_pos.x     ,texture_pos.y+0.1f};
		sprite_quad.tc3 = (VEC2){texture_pos.x+0.1f,texture_pos.y     };
		sprite_quad.tc4 = (VEC2){texture_pos.x+0.1f,texture_pos.y+0.1f};
		sprite_quad.tc5 = (VEC2){texture_pos.x     ,texture_pos.y+0.1f};
		sprite_quad.tc6 = (VEC2){texture_pos.x+0.1f,texture_pos.y     };
		glBufferData(GL_ARRAY_BUFFER,24 * sizeof(float),&sprite_quad,GL_DYNAMIC_DRAW);
		glDrawArrays(GL_TRIANGLES,0,24);
	}
}

void drawItemPiece(VEC2 pos,VEC2 size,VEC2 texture_pos,VEC2 texture_size,VEC3 luminance){
	sprite_quad.p1 = (VEC2){pos.x-size.x,pos.y-size.y};
	sprite_quad.p2 = (VEC2){pos.x-size.x,pos.y+size.y};
	sprite_quad.p3 = (VEC2){pos.x+size.x,pos.y-size.y};
	sprite_quad.p4 = (VEC2){pos.x+size.x,pos.y+size.y};
	sprite_quad.p5 = (VEC2){pos.x-size.x,pos.y+size.y};
	sprite_quad.p6 = (VEC2){pos.x+size.x,pos.y-size.y};
	sprite_quad.tc1 = (VEC2){texture_pos.x                  ,texture_pos.y               };
	sprite_quad.tc2 = (VEC2){texture_pos.x                  ,texture_pos.y+texture_size.y};
	sprite_quad.tc3 = (VEC2){texture_pos.x+texture_size.x,texture_pos.y                  };
	sprite_quad.tc4 = (VEC2){texture_pos.x+texture_size.x,texture_pos.y+texture_size.y};
	sprite_quad.tc5 = (VEC2){texture_pos.x                  ,texture_pos.y+texture_size.y};
	sprite_quad.tc6 = (VEC2){texture_pos.x+texture_size.x,texture_pos.y                  };
	glUniform3f(glGetUniformLocation(entity_dark_shader,"luminance"),luminance.r,luminance.g,luminance.b);
	glBufferData(GL_ARRAY_BUFFER,24 * sizeof(float),&sprite_quad,GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLES,0,24);
}

void drawEnemy(VEC2 pos,VEC2 size,VEC2 texture_pos,VEC3 luminance){
	if(luminance.r == 0.0f && luminance.g == 0.0f && luminance.b == 0.0f) return;
	sprite_quad.p1 = (VEC2){pos.x-size.x,pos.y-size.y};
	sprite_quad.p2 = (VEC2){pos.x-size.x,pos.y+size.y};
	sprite_quad.p3 = (VEC2){pos.x+size.x,pos.y-size.y};
	sprite_quad.p4 = (VEC2){pos.x+size.x,pos.y+size.y};
	sprite_quad.p5 = (VEC2){pos.x-size.x,pos.y+size.y};
	sprite_quad.p6 = (VEC2){pos.x+size.x,pos.y-size.y};
	sprite_quad.tc1 = (VEC2){texture_pos.x                  ,texture_pos.y                  };
	sprite_quad.tc2 = (VEC2){texture_pos.x                  ,texture_pos.y+TEXTURE16_RD_SIZE};
	sprite_quad.tc3 = (VEC2){texture_pos.x+TEXTURE16_RD_SIZE,texture_pos.y                  };
	sprite_quad.tc4 = (VEC2){texture_pos.x+TEXTURE16_RD_SIZE,texture_pos.y+TEXTURE16_RD_SIZE};
	sprite_quad.tc5 = (VEC2){texture_pos.x                  ,texture_pos.y+TEXTURE16_RD_SIZE};
	sprite_quad.tc6 = (VEC2){texture_pos.x+TEXTURE16_RD_SIZE,texture_pos.y                  };
	glUniform3f(glGetUniformLocation(entity_dark_shader,"luminance"),luminance.r,luminance.g,luminance.b);
	glBufferData(GL_ARRAY_BUFFER,24 * sizeof(float),&sprite_quad,GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLES,0,24);
}

void drawMap(){
	glUseProgram(map_shader);
	glUniform2f(glGetUniformLocation(map_shader,"offset"),tFract(camera.pos.x)/camera.zoom,tFract(camera.pos.y)/camera.zoom);
	glUniform2f(glGetUniformLocation(map_shader,"camera"),(u4)camera.pos.x,(u4)camera.pos.y);
	glUniform1f(glGetUniformLocation(map_shader,"zoom"),camera.zoom);
	glActiveTexture(GL_TEXTURE0);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,camera.zoom,camera.zoom,0,GL_RGB,GL_UNSIGNED_BYTE,vram);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBufferData(GL_ARRAY_BUFFER,24 * sizeof(float),&map_quad,GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLES,0,24);
}

void drawLine(VEC2 origin,VEC2 destination,VEC3 color,f4 thickness){
	VEC2 direction = VEC2normalizeR(VEC2subVEC2R(destination,origin));
	VEC2rot(&direction,PI/2.0f);
	VEC2mul(&direction,thickness);
	quad.p1 = (VEC2){origin.x-     direction.x,origin.y-     direction.y};
	quad.p2 = (VEC2){origin.x+     direction.x,origin.y+     direction.y};
	quad.p3 = (VEC2){destination.x+direction.x,destination.y+direction.y};
	quad.p4 = (VEC2){destination.x-direction.x,destination.y-direction.y};
	quad.p5 = (VEC2){destination.x+direction.x,destination.y+direction.y};
	quad.p6 = (VEC2){origin.x-     direction.x,origin.y-     direction.y};
	glUniform3f(glGetUniformLocation(color_shader,"color"),color.r,color.g,color.b);
	glBufferData(GL_ARRAY_BUFFER,24 * sizeof(float),&quad,GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLES,0,24);
}

void drawParticle(VEC2 pos,VEC2 size,VEC3 luminance){
	quad.p1 = (VEC2){pos.x-size.x,pos.y-size.y};
	quad.p2 = (VEC2){pos.x-size.x,pos.y+size.y};
	quad.p3 = (VEC2){pos.x+size.x,pos.y-size.y};
	quad.p4 = (VEC2){pos.x+size.x,pos.y+size.y};
	quad.p5 = (VEC2){pos.x-size.x,pos.y+size.y};
	quad.p6 = (VEC2){pos.x+size.x,pos.y-size.y};
	glUniform3f(glGetUniformLocation(particle_shader,"luminance"),luminance.r,luminance.g,luminance.b);
	glBufferData(GL_ARRAY_BUFFER,24 * sizeof(float),&quad,GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLES,0,24);
}

void drawRect(VEC2 pos,VEC2 size,VEC3 color){
	quad.p1 = (VEC2){pos.x-size.x,pos.y-size.y};
	quad.p2 = (VEC2){pos.x-size.x,pos.y+size.y};
	quad.p3 = (VEC2){pos.x+size.x,pos.y-size.y};
	quad.p4 = (VEC2){pos.x+size.x,pos.y+size.y};
	quad.p5 = (VEC2){pos.x-size.x,pos.y+size.y};
	quad.p6 = (VEC2){pos.x+size.x,pos.y-size.y};
	glUniform3f(glGetUniformLocation(color_shader,"color"),color.r,color.g,color.b);
	glBufferData(GL_ARRAY_BUFFER,24 * sizeof(float),&quad,GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLES,0,24);
}