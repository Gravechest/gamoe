#include "entity_light.h"
#include "source.h"

PARTICLEHUB entity_light;

void entityLightTick(){
	for(u4 i = 0;i < entity_light.cnt;i++){
		switch(entity_light.state[i].type){
		case PARTICLE_NORMAL:
			if(entity_light.state[i].health--){
				VEC2addVEC2(&entity_light.state[i].pos,entity_light.state[i].vel);
				collision(&entity_light.state[i].pos,entity_light.state[i].vel,0.5f);
				if(entity_light.state[i].health<15){
					VEC3mul(&entity_light.state[i].color,0.8f);
					entity_light.state[i].size *= 0.8;
				}	
			}
			else{
				ENTITY_REMOVE(entity_light,i);
			}
			break;
		case LOOT_INFANT:
			if(entity_light.state[i].health--){
				switch(entity_light.state[i].loot_type){
				case LOOT_ENERGY:
					VEC3addVEC3(&entity_light.state[i].color,LOOT_ENERGY_COLOR_INFANT);
					break;
				case LOOT_HEALTH:
					VEC3addVEC3(&entity_light.state[i].color,LOOT_HEALTH_COLOR_INFANT);
					break;
				}
				entity_light.state[i].size += 0.02f;
			}
			else{
				entity_light.state[i].vel = VEC2rndR();
				entity_light.state[i].type = LOOT_PARENT;
				entity_light.state[i].health = 60*60;
			}
			break;
		case LOOT_PARENT:
			if(entity_light.state[i].health--){
				VEC2addVEC2(&entity_light.state[i].pos,entity_light.state[i].vel);
				collision(&entity_light.state[i].pos,entity_light.state[i].vel,entity_light.state[i].size/2.0f);
				if(VEC2distance(entity_light.state[i].pos,player.pos)<1.0f){
					switch(entity_light.state[i].loot_type){
					case LOOT_SCRAP:
						player.scrap++;
						if(player.energy>SCRAP_MAX) player.energy = SCRAP_MAX;
						break;
					case LOOT_ENERGY:
						player.energy += 200;
						if(player.energy>ENERGY_MAX) player.energy = ENERGY_MAX;
						break;
					case LOOT_HEALTH:
						player.health += 10;
						if(player.health>HEALTH_MAX) player.health = HEALTH_MAX;
						break;
					}
					ENTITY_REMOVE(entity_light,i);
					break;
				}	
				VEC2mul(&entity_light.state[i].vel,PR_FRICTION);
				VEC2addVEC2(&entity_light.state[i].vel,entityPull(entity_light.state[i].pos,player.pos,1.0f));
				if(entity_light.state[i].health<15){
					VEC3mul(&entity_light.state[i].color,0.8f);
					entity_light.state[i].size *= 0.8;
				}	
			}
			else{
				ENTITY_REMOVE(entity_light,i);
			}
			break;
		}
	}
}