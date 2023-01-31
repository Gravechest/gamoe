#include <math.h>

#include "entity_dark.h"
#include "player.h"
#include "entity_light.h"
#include "source.h"
#include "tmath.h"

ENEMYHUB entity_dark;

void entityDarkHurt(u4 id,u4 ammount){
	if(entity_dark.state[id].health<ammount){
		ENTITY_REMOVE(entity_dark,id);
		return;
	}
	entity_dark.state[id].health -= ammount;
}

void entityDarkTick(){
	if(/*tRnd()<1.003f && */entity_dark.cnt < 16){
		VEC2 spawn = (VEC2){(tRnd()-1.0f)*CHUNK_SIZE,(tRnd()-1.0f)*CHUNK_SIZE};
		if(tRnd()<1.5f) spawn.x += CHUNK_SIZE;
		if(tRnd()<1.5f) spawn.y += CHUNK_SIZE;
		entity_dark.state[entity_dark.cnt].pos = spawn;
		entity_dark.state[entity_dark.cnt].health = 10;
		entity_dark.state[entity_dark.cnt++].vel = VEC2_ZERO;
	}
	for(u4 i = 0;i < entity_dark.cnt;i++){
		u4 iterations = (u4)tAbsf(player.pos.x-entity_dark.state[i].pos.x)+(u4)tAbsf(player.pos.y-entity_dark.state[i].pos.y);
		VEC2 toPlayer = VEC2normalizeR(VEC2subVEC2R(player.pos,entity_dark.state[i].pos));
		if(player.health){
			if(lineOfSight(entity_dark.state[i].pos,toPlayer,iterations)){
				if(entity_dark.state[i].aggressive){
					VEC2div(&toPlayer,50.0f);
					VEC2addVEC2(&entity_dark.state[i].vel,toPlayer);
				}
				else{
					f4 distance = VEC2distance(player.pos,entity_dark.state[i].pos)/50.0f;
					if(sqrtf(distance)+tRnd()<2.0f) entity_dark.state[i].aggressive = TRUE;
				}
			}
			else if(tRnd()<1.02f) entity_dark.state[i].aggressive = FALSE;
		}
		if(tRnd() < 1.05f) VEC2addVEC2(&entity_dark.state[i].vel,(VEC2){(tRnd()-1.5f)/2.5f,(tRnd()-1.5f)/2.5f});
		VEC2addVEC2(&entity_dark.state[i].pos,entity_dark.state[i].vel);
		collision(&entity_dark.state[i].pos,entity_dark.state[i].vel,ENEMY_SIZE/2.0f);
		for(u4 j = 0;j < entity_dark.cnt;j++){
			if(i==j) continue;
			if(AABBcollision(entity_dark.state[i].pos,entity_dark.state[j].pos,ENEMY_SIZE,ENEMY_SIZE)){
				VEC2 pushaway = VEC2mulR(VEC2normalizeR(VEC2subVEC2R(entity_dark.state[j].pos,entity_dark.state[i].pos)),0.1f);
				VEC2subVEC2(&entity_dark.state[i].vel,pushaway);
			}
		}
		if(player.health && AABBcollision(entity_dark.state[i].pos,player.pos,ENEMY_SIZE/1.3f,PLAYER_SIZE/1.3f)){
			VEC2 pushaway = VEC2mulR(VEC2normalizeR(VEC2subVEC2R(player.pos,entity_dark.state[i].pos)),0.1f);
			VEC2subVEC2(&entity_dark.state[i].vel,pushaway);
			playerHurt(2);
			VEC2 rel_pos = VEC2subVEC2R(entity_dark.state[i].pos,player.pos);
			VEC2 velocity = VEC2mulR(rel_pos,(tRnd()-1.0f)*0.1f); 
			VEC2rot(&velocity,(tRnd()-1.5f)*0.5f);
			particleSpawn((VEC3){0.07f,0.02f,0.02f},VEC2addVEC2R(player.pos,rel_pos),velocity,1.0f,40);
		}
		VEC2mul(&entity_dark.state[i].vel,PR_FRICTION);	
		if(entity_dark.state[i].pos.x < ENEMY_SIZE || entity_dark.state[i].pos.x > SIM_SIZE-ENEMY_SIZE-1.0f ||
		entity_dark.state[i].pos.y < ENEMY_SIZE || entity_dark.state[i].pos.y > SIM_SIZE-ENEMY_SIZE-1.0f){
			ENTITY_REMOVE(entity_dark,i);
		}
	}
}