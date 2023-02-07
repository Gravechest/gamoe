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

void playerAttack(u1 hand){
	if(!player.weapon_cooldown){
		switch(inventory.item_primary.type){
		case ITEM_NOTHING:
			player.melee_progress = PLAYER_FIST_ATTACKDURATION;
			player.weapon_cooldown = PLAYER_FIST_COOLDOWN;
			player.fist_side = hand;
			break;
		case ITEM_BOMB:
			entity_light.state[entity_light.cnt].pos = player.pos;
			entity_light.state[entity_light.cnt].type = ENTITY_BOMB;
			entity_light.state[entity_light.cnt].color = BOMB_COLOR;
			entity_light.state[entity_light.cnt].size = 0.4f;
			entity_light.state[entity_light.cnt].health = 180;
			entity_light.state[entity_light.cnt++].vel = VEC2divR(playerLookDirection(),24.0f);
			//inventory.item_primary.type = ITEM_NOTHING;
			//inventory.item_ammount[ITEM_BOMB]--;
			break;
		case ITEM_MELEE:
			player.melee_progress = PLAYER_MELEE_ATTACKDURATION;
			player.weapon_cooldown = PLAYER_MELEE_COOLDOWN;
			break;
		case ITEM_LASER:
			if(player.energy > WEAPON_LASER_COST){
				player.energy -= WEAPON_LASER_COST;
				VEC2 direction = playerLookDirection();
				RAY2D ray = ray2dCreate(player.pos,direction);
				while(ray.square_pos.x >= 0 && ray.square_pos.x < SIM_SIZE && ray.square_pos.y >= 0 && ray.square_pos.y < SIM_SIZE){
					if(map.type[coordToMap(ray.square_pos.x,ray.square_pos.y)]==BLOCK_NORMAL) break;
					ray2dIterate(&ray);
				}
				f4 min_dst = 99999.0f;
				i4 id = -1;
				for(u4 i = 0;i < entity_dark.cnt;i++){
					if(rayIntersectSquare(entity_dark.state[i].pos,direction,player.pos,ENEMY_SIZE/2.0f) != -1.0f){
						f4 dst = VEC2distance(player.pos,entity_dark.state[i].pos);
						if(lineOfSight(player.pos,entity_dark.state[i].pos) && min_dst > dst){
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
		if(player.fist_side) VEC2subVEC2(&hit_pos,offset);
		else                 VEC2addVEC2(&hit_pos,offset);
		VEC2mul(&hit_pos,sinf((f4)player.melee_progress/PLAYER_FIST_COOLDOWN*PI)*1.8f);
		break;
	case ITEM_MELEE:
		VEC2mul(&hit_pos,cosf((f4)player.melee_progress/PLAYER_MELEE_COOLDOWN*PI+PI*0.5f)*3.0f);
		break;
	}
	VEC2addVEC2(&hit_pos,player.pos);
	return hit_pos;
}

u4 blockHitParticle(VEC2 hit_pos,u4 m_pos,u4 damage){
	IVEC2 tile_crd = posToTileTextureCoord(hit_pos);
	u4 tile_pos = coordTileTextureToTileTexture(tile_crd.x,tile_crd.y);
	tile_texture_data[tile_pos].r >>= 1;
	tile_texture_data[tile_pos].g >>= 1;
	tile_texture_data[tile_pos].b >>= 1;
	if(map.data[m_pos].health < damage){
		map.type[m_pos] = BLOCK_AIR;
		gl_queue.message[gl_queue.cnt].id = GLMESSAGE_SINGLE_MAPEDIT;
		gl_queue.message[gl_queue.cnt++].pos = (IVEC2){hit_pos.x,hit_pos.y};
		tileTextureGen((VEC2){2.0f,64.0f},(VEC2){2.0f,64.0f},(VEC2){2.0f,64.0f},m_pos);
		gl_queue.message[gl_queue.cnt].id = GLMESSAGE_WHOLE_TILEEDIT;
		gl_queue.message[gl_queue.cnt++].pos = (IVEC2){m_pos/SIM_SIZE,m_pos%SIM_SIZE};
		return BLOCKHIT_DESTROY;
	}
	else map.data[m_pos].health -= damage;
	player.melee_progress = 0;
	gl_queue.message[gl_queue.cnt].id = GLMESSAGE_SINGLE_TILEEDIT;
	gl_queue.message[gl_queue.cnt++].pos = (IVEC2){tile_crd.x,tile_crd.y};
	for(u4 i = 0;i < 5;i++){
		VEC2 rel_pos = VEC2divR(VEC2subVEC2R(player.pos,hit_pos),tRnd()*25.0f);
		blockParticleSpawn(hit_pos,VEC2rotR(rel_pos,tRnd()-1.5f));
	}
	return BLOCKHIT_NORMAL;
}

u4 blockHit(VEC2 hit_pos,u4 m_pos,u4 damage){
	if(map.data[m_pos].health < damage){
		map.type[m_pos] = BLOCK_AIR;
		tileTextureGen((VEC2){2.0f,64.0f},(VEC2){2.0f,64.0f},(VEC2){2.0f,64.0f},m_pos);
		gl_queue.message[gl_queue.cnt].id = GLMESSAGE_SINGLE_MAPEDIT;
		gl_queue.message[gl_queue.cnt++].pos = (IVEC2){hit_pos.x,hit_pos.y};
		gl_queue.message[gl_queue.cnt].id = GLMESSAGE_BLOCK_TILEEDIT;
		gl_queue.message[gl_queue.cnt++].pos = (IVEC2){m_pos/SIM_SIZE,m_pos%SIM_SIZE};
		return BLOCKHIT_DESTROY;
	}
	IVEC2 tile_crd = posToTileTextureCoord(hit_pos);
	u4 tile_pos = coordTileTextureToTileTexture(tile_crd.x,tile_crd.y);
	tile_texture_data[tile_pos].r >>= 1;
	tile_texture_data[tile_pos].g >>= 1;
	tile_texture_data[tile_pos].b >>= 1;
	map.data[m_pos].health -= damage;
	player.melee_progress = 0;
	gl_queue.message[gl_queue.cnt].id = GLMESSAGE_SINGLE_TILEEDIT;
	gl_queue.message[gl_queue.cnt++].pos = (IVEC2){tile_crd.x,tile_crd.y};
	return BLOCKHIT_NORMAL;
}

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
		if(menu_select == MENU_GAME){
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
				switch(map.type[m_pos]){
				case BLOCK_NORMAL:
					if(blockHitParticle(hit_pos,m_pos,30)==BLOCKHIT_DESTROY){
						itemEntitySpawnNew((VEC2){(f4)(u4)hit_pos.x+0.5f,(f4)(u4)hit_pos.y+0.5f},VEC2_ZERO,ITEM_STONEDUST);
					}
					break;
				case BLOCK_TREE:
					if(blockHitParticle(hit_pos,m_pos,100)==BLOCKHIT_DESTROY){
						itemEntitySpawnNew((VEC2){(f4)(u4)hit_pos.x+0.5f,(f4)(u4)hit_pos.y+0.5f},VEC2_ZERO,ITEM_LOG);
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
				switch(map.type[m_pos]){
				case BLOCK_NORMAL:
					blockHitParticle(hit_pos,m_pos,30);
					break;
				case BLOCK_TREE:
					blockHitParticle(hit_pos,m_pos,30);
					break;
				}
				break;
			}
		}
	}
}