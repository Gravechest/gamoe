#include "player.h"
#include "source.h"
#include "ray.h"
#include "entity_light.h"
#include "opengl.h"
#include "chunk.h"
#include "entity_dark.h"
#include "tmath.h"
#include "entity_item.h"
#include "inventory.h"

PLAYER player;

void playerHurt(u4 ammount){
	player.health -= ammount;
	if(player.health<0){
		player.health = 0;
		player.respawn_countdown = 120;
	}
}

VEC2 playerLookDirection(){
	return VEC2subVEC2R(getCursorPosMap(),VEC2subVEC2R(player.pos,camera.pos));
}

void playerAttack(){
	if(!player.weapon_cooldown){
		switch(inventory.item_primary.type){
		case ITEM_NOTHING:
			player.melee_progress = 20;
			player.weapon_cooldown = PLAYER_FIST_COOLDOWN;
			player.fist_side ^= TRUE;
			break;
		case ITEM_BOMB:
			entity_light.state[entity_light.cnt].pos = player.pos;
			entity_light.state[entity_light.cnt].type = ENTITY_BOMB;
			entity_light.state[entity_light.cnt].color = BOMB_COLOR;
			entity_light.state[entity_light.cnt].size = 0.4f;
			entity_light.state[entity_light.cnt].health = 180;
			entity_light.state[entity_light.cnt++].vel = VEC2divR(playerLookDirection(),20.0f);
			//inventory.item_primary = 0;
			break;
		case ITEM_MELEE:
			player.melee_progress = 60;
			player.weapon_cooldown = PLAYER_MELEE_COOLDOWN;
			break;
		case ITEM_LASER:
			if(player.energy > WEAPON_LASER_COST){
				player.energy -= WEAPON_LASER_COST;
				VEC2 direction = playerLookDirection();
				RAY2D ray = ray2dCreate(player.pos,direction);
				while(ray.square_pos.x >= 0 && ray.square_pos.x < SIM_SIZE && ray.square_pos.y >= 0 && ray.square_pos.y < SIM_SIZE){
					if(map[coordToMap(ray.square_pos.x,ray.square_pos.y)].type==BLOCK_NORMAL) break;
					ray2dIterate(&ray);
				}
				f4 min_dst = 99999.0f;
				i4 id = -1;
				for(u4 i = 0;i < entity_dark.cnt;i++){
					if(rayIntersectSquare(entity_dark.state[i].pos,direction,player.pos,ENEMY_SIZE/2.0f) != -1.0f){
						u4 iterations = (u4)tAbsf(player.pos.x-entity_dark.state[i].pos.x)+(u4)tAbsf(player.pos.y-entity_dark.state[i].pos.y);
						f4 dst = VEC2distance(player.pos,entity_dark.state[i].pos);
						if(lineOfSight(player.pos,direction,iterations) && min_dst > dst){
							min_dst = dst;
							id = i;
						}
					}
				}
				if(id!=-1){
					VEC2 ray_end_pos = VEC2addVEC2R(player.pos,VEC2mulR(VEC2normalizeR(direction),min_dst));
					for(u4 j = 0;j < 5;j++){
						entity_light.state[entity_light.cnt].size = 0.5f;
						entity_light.state[entity_light.cnt].type = PARTICLE_NORMAL;
						entity_light.state[entity_light.cnt].color = VEC3mulR(LASER_LUMINANCE,0.5f);
						entity_light.state[entity_light.cnt].vel = VEC2mulR(VEC2rotR(direction,(tRnd()-1.5f)*0.5f),(tRnd()-0.5f)*0.01f);
						entity_light.state[entity_light.cnt].health = 30+tRnd()*30.0f;
						entity_light.state[entity_light.cnt++].pos = ray_end_pos;
					}
					while(tRnd()<1.5f){
						VEC2 orb_direction = VEC2mulR(VEC2rotR(direction,(tRnd()-1.5f)*0.5f),(tRnd()-0.5f)*0.01f);
						spawnLootOrb(LOOT_ENERGY,orb_direction,ray_end_pos);
					}
					while(tRnd()<1.5f){
						VEC2 orb_direction = VEC2mulR(VEC2rotR(direction,(tRnd()-1.5f)*0.5f),(tRnd()-0.5f)*0.01f);
						spawnLootOrb(LOOT_HEALTH,orb_direction,ray_end_pos);
					}
					while(tRnd()<1.5f){
						VEC2 orb_direction = VEC2mulR(VEC2rotR(direction,(tRnd()-1.5f)*0.5f),(tRnd()-0.5f)*0.01f);
						spawnLootOrb(LOOT_SCRAP,orb_direction,ray_end_pos);
					}
					laser.state[laser.cnt].pos_dst = ray_end_pos;
					for(u4 i = 0;i < entity_dark.cnt;i++){
						if(id==i) continue;
						VEC2subVEC2(&entity_dark.state[i].vel,entityPull(entity_dark.state[i].pos,entity_dark.state[id].pos,1.0f));
					}
					ENTITY_REMOVE(entity_dark,id);
				}
				else{
					laser.state[laser.cnt].pos_dst = ray2dGetCoords(ray);
				}
				laser.state[laser.cnt].pos_org = player.pos;
				laser.state[laser.cnt++].health = 5;
				player.weapon_cooldown = PLAYER_LASER_COOLDOWN;
			}
			break;
		}
	}
}

VEC2 meleeHitPos(){
	VEC2 hit_pos = VEC2normalizeR(playerLookDirection());
	switch(inventory.item_primary.type){
	case ITEM_NOTHING:
		VEC2 offset = VEC2rotR(hit_pos,PI*0.5f);
		VEC2div(&offset,2.4f);
		if(player.fist_side) VEC2addVEC2(&hit_pos,offset);
		else                 VEC2subVEC2(&hit_pos,offset);
		VEC2mul(&hit_pos,sinf((f4)player.melee_progress/PLAYER_FIST_COOLDOWN*PI)*1.8f);
		break;
	case ITEM_MELEE:
		VEC2mul(&hit_pos,cosf((f4)player.melee_progress/PLAYER_MELEE_COOLDOWN*PI+PI*0.5f)*3.0f);
		break;
	}
	VEC2addVEC2(&hit_pos,player.pos);
	return hit_pos;
}

u4 blockHit(VEC2 hit_pos,u4 m_pos,u4 damage){
	IVEC2 tile_crd = posToTileTextureCoord(hit_pos);
	tile_texture_data[coordToTileTexture(tile_crd.x,tile_crd.y)].r >>= 1;
	tile_texture_data[coordToTileTexture(tile_crd.x,tile_crd.y)].g >>= 1;
	tile_texture_data[coordToTileTexture(tile_crd.x,tile_crd.y)].b >>= 1;
	if(map[m_pos].health < damage){
		map[m_pos].type = BLOCK_AIR;
		gl_queue.message[gl_queue.cnt].id = GLMESSAGE_SINGLE_MAPEDIT;
		gl_queue.message[gl_queue.cnt++].pos = (IVEC2){hit_pos.x,hit_pos.y};
		tileTextureGen((VEC2){2.0f,64.0f},(VEC2){2.0f,64.0f},(VEC2){2.0f,64.0f},m_pos);
		gl_queue.message[gl_queue.cnt].id = GLMESSAGE_WHOLE_TILEEDIT;
		gl_queue.message[gl_queue.cnt++].pos = (IVEC2){m_pos/SIM_SIZE,m_pos%SIM_SIZE};
		return BLOCKHIT_DESTROY;
	}
	else map[m_pos].health -= damage;
	player.melee_progress = 0;
	gl_queue.message[gl_queue.cnt].id = GLMESSAGE_SINGLE_TILEEDIT;
	gl_queue.message[gl_queue.cnt++].pos = (IVEC2){tile_crd.x,tile_crd.y};
	for(u4 i = 0;i < 5;i++){
		VEC2 rel_pos = VEC2divR(VEC2subVEC2R(player.pos,hit_pos),tRnd()*25.0f);
		blockParticleSpawn(hit_pos,VEC2rotR(rel_pos,tRnd()-1.5f));
	}
	return BLOCKHIT_NORMAL;
}

#include <stdio.h>

void playerGameTick(){
	if(!player.health){
		if(!player.respawn_countdown--){
			player.pos = (VEC2)PLAYER_SPAWN;
			player.health = HEALTH_MAX;
			player.energy = ENERGY_MAX;
			worldLoadSpawn();
			entity_dark.cnt = 0;
			entity_light.cnt = 0;
			laser.cnt = 0;
			chunk.cnt = 0;
			gl_queue.message[gl_queue.cnt++].id = GLMESSAGE_WHOLE_MAPEDIT;
		}
	}
	else{
		if(key_pressed.w){
			if(key_pressed.d || key_pressed.a) player.vel.y+=PLAYER_SPEED * 0.7f;
			else                               player.vel.y+=PLAYER_SPEED;
		}
		if(key_pressed.s){
			if(key_pressed.d || key_pressed.a) player.vel.y-=PLAYER_SPEED * 0.7f;
			else                               player.vel.y-=PLAYER_SPEED;
		}
		if(key_pressed.d){
			if(key_pressed.s || key_pressed.w) player.vel.x+=PLAYER_SPEED * 0.7f;
			else                               player.vel.x+=PLAYER_SPEED;
		}
		if(key_pressed.a){
			if(key_pressed.s || key_pressed.w) player.vel.x-=PLAYER_SPEED * 0.7f;
			else                               player.vel.x-=PLAYER_SPEED;
		}
		if(player.energy && key_pressed.mouse_right & 0x80){
			player.flashlight = TRUE;
			player.energy--;
		}
		else{
			player.flashlight = FALSE;
		}
		VEC2addVEC2(&player.pos,player.vel);
		collision(&player.pos,player.vel,PLAYER_SIZE/2.0f);
		VEC2mul(&player.vel,PR_FRICTION);
		if(player.weapon_cooldown) player.weapon_cooldown--;
		if(player.melee_progress){
			player.melee_progress--;
			VEC2 hit_pos = meleeHitPos();
			u4 m_pos = coordToMap(hit_pos.x,hit_pos.y);
			switch(inventory.item_primary.type){
			case ITEM_NOTHING:
				for(u4 i = 0;i < entity_dark.cnt;i++){
					if(pointAABBcollision(hit_pos,entity_dark.state[i].pos,(VEC2){ENEMY_SIZE/2.0f,ENEMY_SIZE/2.0f})){
						ENTITY_REMOVE(entity_dark,i);
					}
				}
				switch(map[m_pos].type){
				case BLOCK_NORMAL:
					if(blockHit(hit_pos,m_pos,30)==BLOCKHIT_DESTROY){
						itemEntitySpawn((VEC2){(f4)(u4)hit_pos.x+0.5f,(f4)(u4)hit_pos.y+0.5f},VEC2_ZERO,ITEM_STONEDUST);
					}
					break;
				case BLOCK_TREE:
					if(blockHit(hit_pos,m_pos,100)==BLOCKHIT_DESTROY){
						itemEntitySpawn((VEC2){(f4)(u4)hit_pos.x+0.5f,(f4)(u4)hit_pos.y+0.5f},VEC2_ZERO,ITEM_LOG);
					}
					break;
				}
				break;
			case ITEM_MELEE:
				for(u4 i = 0;i < entity_dark.cnt;i++){
					if(pointAABBcollision(hit_pos,entity_dark.state[i].pos,(VEC2){ENEMY_SIZE/2.0f,ENEMY_SIZE/2.0f})){
						ENTITY_REMOVE(entity_dark,i);
					}
				}
				switch(map[m_pos].type){
				case BLOCK_NORMAL:
					blockHit(hit_pos,m_pos,30);
					break;
				case BLOCK_TREE:
					blockHit(hit_pos,m_pos,30);
					break;
				}
				break;
			}
		}
	}
}