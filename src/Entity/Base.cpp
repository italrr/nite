#include "../Engine/Graphics.hpp"
#include "../Engine/Texture.hpp"

#include "../Core/Network.hpp"
#include "../Core/Server.hpp"

#include "Base.hpp"

static void notifyEntityDeath(Game::EntityBase *ent){
    if(ent != NULL && ent->sv != NULL){
        if(auto cl = ent->sv->getClientByEntityId(ent->id)){
            ent->sv->notifyDeath(cl->clientId);
        }
    }
}

void Game::EntityBase::entityMove(float angle, float mod, bool holdStance){ 
	if(healthStat.dead){
		return;
	}
	if(!holdStance && (nite::toDegrees(angle) <= 45 || nite::toDegrees(angle) >= 315)){
		faceDirection = EntityFacing::Right;
	}
	if(!holdStance && (nite::toDegrees(angle) >= 135 && nite::toDegrees(angle) <= 225)){
		faceDirection = EntityFacing::Left;
	}
	// isWalking = true;
	// walkingTimeout = nite::getTicks();
	if(state[EntityStateSlot::BOTTOM] != EntityState::WALKING){
		setState(EntityState::WALKING, EntityStateSlot::BOTTOM, 0);
	}
	isWalking = true;
	move(angle, mod + 0.25f * complexStat.walkRate);
}

void Game::EntityBase::kill(){
	nite::print("killed entity id "+nite::toStr(id)+" '"+name+"'");
	if(healthStat.dead){
		return;
	}
	healthStat.dead = true;
	onDeath();
}

void Game::EntityBase::onCreate(){
    unmovable = false;
    solid = true;
    friction = 0.25f; 
    mass = 2.8f;
    healthStat.dead = false;
    size.set(128, 184);
    walkPushRate = 5.0f;
    name = "Base Entity Type";    
}

void Game::EntityBase::printInfo(){
	#define str nite::toStr
	String ms = str(GAME_MAX_STAT);
	nite::print("ID "+str(id)+" | Name: "+name+" | LV "+str(healthStat.lv)+" | Carry "+str(complexStat.maxCarry));
	nite::print("HP "+str(healthStat.maxHealth)+" | Mana "+str(healthStat.maxMana)+" | Stamina "+str(healthStat.maxStamina)+" | StatPoints "+str(baseStat.statPoints));
	nite::print("Str "+str(baseStat.strAdd)+"/"+ms+" | Agi "+str(baseStat.agiAdd)+"/"+ms+
	" | Dex "+str(baseStat.dexAdd)+"/"+ms+" | Endurance "+str(baseStat.enduAdd)+"/"+ms+
	"| Luk "+str(baseStat.lukAdd)+"/"+ms+" | Int "+str(baseStat.intelAdd)+"/"+ms+
	" | Char "+str(baseStat.charmAdd)+"/"+ms);
	nite::print("Atk "+str(complexStat.atk)+" "+"MAtk "+str(complexStat.magicAtk)+" "+"Def "+str(complexStat.def)+" "+"MDef "+str(complexStat.magicDef));
	nite::print("WalkRate "+str(complexStat.walkRate));
	nite::print("CritRate "+str(complexStat.critRate));
	nite::print("PrecsRate "+str(complexStat.precsRate));
	nite::print("AtkRate "+str(complexStat.atkRate));
	nite::print("CharmRate "+str(complexStat.charmRate));
	nite::print("PersuasionRate "+str(complexStat.persuasionRate));    
	#undef str    
}

void Game::EntityBase::draw(){
	UInt8 bot = EntityState::stateToAnimType[state[EntityStateSlot::BOTTOM]][EntityStateSlot::BOTTOM];
	UInt8 mid = EntityState::stateToAnimType[state[EntityStateSlot::MID]][EntityStateSlot::MID];
	UInt8 anims[AnimPart::total] = {bot, mid, AnimType::TOP_NEUTRAL};
	UInt8 numbs[AnimPart::total] = {stNum[EntityStateSlot::BOTTOM], stNum[EntityStateSlot::MID], 0};
	anim.setState(anims, numbs);

    nite::setRenderTarget(nite::RenderTargetGame);
	nite::setColor(1.0f, 1.0f, 1.0f, 1.0f);
	nite::setDepth(nite::DepthMiddle);
	float reversed = faceDirection == EntityFacing::Left ? -1.0f : 1.0f;
	auto ref = anim.batch.draw(position.x, position.y, anim.frameSize.x * reversed, anim.frameSize.y, 0.5f, 0.5f, 0.0f);	
}

void Game::EntityBase::entityStep(){
	if(healthStat.health == 0 && !healthStat.dead){
		healthStat.dead = true;
		notifyEntityDeath(this);
		onDeath();
	}
	updateStance();
}

void Game::EntityBase::setState(UInt8 nstate, UInt8 slot, UInt8 n){
	if(nstate > EntityState::total || slot > EntityStateSlot::total){
		return;
	}
	auto nanim = anim.getAnim(EntityState::stateToAnimType[nstate][slot]);
	if(nanim != NULL && nanim->n > 0 && n > nanim->n){
		n = n % nanim->n;
	}
	bool update = false;
	if(state[slot] != nstate || stNum[slot] != n){
		update = true;
	}
	state[slot] = nstate;
	lastStateTime[slot] = nite::getTicks();	
	stNum[slot] = n;
	// issue anim update to clients
	if(update && sv != NULL){
		nite::Packet stance;
		stance.setHeader(Game::PacketType::SV_UPDATE_ENTITY_STANCE_STATE);
		stance.write(&this->id, sizeof(this->id));
		stance.write(&faceDirection, sizeof(faceDirection));
		stance.write(&state[EntityStateSlot::BOTTOM], sizeof(UInt8));
		stance.write(&stNum[EntityStateSlot::BOTTOM], sizeof(UInt8));
		stance.write(&state[EntityStateSlot::MID], sizeof(UInt8));
		stance.write(&stNum[EntityStateSlot::MID], sizeof(UInt8));
		// TODO: filter by whether this entity is actually in the clients view
		sv->sendAll(stance);
	}
}

void Game::EntityBase::throwMelee(float x, float y){
	// setState(EntityState::MELEE_NOWEAP, EntityStateSlot::MID, 0);
}

void Game::EntityBase::updateStance(){
	// this is for humanoid basically
	for(int i = 0; i < EntityStateSlot::total; ++i){
		auto &part = i;
		switch(state[i]){
            case EntityState::IDLE: {
			} break;
            case EntityState::WALKING: {
				if(part != EntityStateSlot::BOTTOM){
					break;
				}
				auto canim = this->anim.getAnim(EntityState::stateToAnimType[EntityState::WALKING][EntityStateSlot::BOTTOM]);
				if(canim == NULL){
					break;
				}
				UInt64 walkRateDiff = complexStat.walkRate * 8;
				UInt64 walkAnimTime = canim->spd - (walkRateDiff > walkAnimTime ? ((UInt64)canim->spd*0.05f) : walkRateDiff);
				UInt64 currentTime = nite::getTicks()-lastStateTime[EntityStateSlot::BOTTOM];
				if(!isWalking && currentTime > walkAnimTime){
					setState(EntityState::IDLE, EntityStateSlot::BOTTOM, 0);
				}else
				if(currentTime > walkAnimTime){
					setState(EntityState::WALKING, EntityStateSlot::BOTTOM, stNum[EntityStateSlot::BOTTOM] + 1);
				}
			} break;
			case EntityState::JUMPING: {
			} break; 
			case EntityState::SHOOTING_HANDGUN: {
			} break;
			case EntityState::SHOOTING_BOW: {
			} break;
			case EntityState::IDLE_HANDGUN: {
			} break;
			case EntityState::IDLE_BOW: {
			} break;
			case EntityState::IDLE_SWORD: {
			} break;
            case EntityState::IDLE_KNIFE: {
			} break;
            case EntityState::WAVING_KNIFE: {
			} break;
			case EntityState::WAVING_SWORD: {
			} break;
            case EntityState::MELEE_NOWEAP: {
				if(part != EntityStateSlot::MID){
					break;
				}		
				auto canim = this->anim.getAnim(EntityState::stateToAnimType[EntityState::MELEE_NOWEAP][EntityStateSlot::MID]);		
				switch(stNum[EntityStateSlot::MID]){
					case 0:{
						if(nite::getTicks()-lastStateTime[EntityStateSlot::MID] > canim->spd){
							setState(EntityState::MELEE_NOWEAP, EntityStateSlot::MID, stNum[EntityStateSlot::MID] + 1);
						}
					} break;
					case 1: {
						throwMelee(0.0f, 0.0f);
						if(nite::getTicks()-lastStateTime[EntityStateSlot::MID] > 200){ // 200 hard coded for now
							setState(EntityState::IDLE_FIST, EntityStateSlot::MID, 0);
						}						
					} break;					
				}
			} break;
			case EntityState::IDLE_FIST: {
				if(nite::getTicks()-lastStateTime[EntityStateSlot::MID] > 1500){
					setState(EntityState::IDLE, EntityStateSlot::MID, 0);
				}	
			} break;
			case EntityState::CASTING: {
			} break;                
		}		
	}
	isWalking = false;
}

void Game::EntityBase::invokeUse(UInt16 targetId, UInt8 type, UInt32 id, float x, float y){
	if(this->sv == NULL){
		return;
	}
	auto target = this->sv->getEntity(targetId);
	switch(type){
		case ActionableType::Skill: {
			auto sendSkillState = [&](Game::Skill *sk){
				auto cl = sv->getClientByEntityId(this->id);
				if(cl == NULL){
					return;
				}
				nite::Packet update(++cl->svOrder);
				update.setHeader(Game::PacketType::SV_UPDATE_SKILL_STATE);
				update.write(&this->id, sizeof(UInt16));
				update.write(&sk->id, sizeof(UInt16));
				sk->writeUpdate(update);
				sv->persSend(cl->cl, update, 1000, -1);
			};
			auto sk = skillStat.get(id);
			if(sk != NULL && sk->isReady(this)){ 
				if(sk->castDelay == 0){
					if(sk->use(this, target, nite::Vec2(x, y))){
						sendSkillState(sk);
					}
				}else{
					// TODO: set ent to cast
				}
			}
		} break;
		case ActionableType::Item: {
			nite::print("USE ITEM TODO");
		} break;		
	}
}

void Game::EntityBase::recalculateStats(){

	if(this->healthStat.dead){
		return;
	}

	baseStat.resetAdd();
	resetComplexStats();
	

	// recalculate effects-given stats
	auto &eff = effectStat.effects;
	for(auto &ef : eff){
		ef.second->onRecalculateStat(this);
	}
	
	// recalculate passive-items-given stats
	auto &carry = invStat.carry;
	for(auto &item : carry){
		item.second->onRecalculateStat(this);
	}

	// recalculate passive-skill-given stats
	auto &skills = skillStat.skills;
	for(auto &sk : carry){
		sk.second->onRecalculateStat(this);
	}

	// recalculate local stats
	recalculateBaseStats();
	recalculateHealthStats();
	recalculateComplexStats();	

	// normalize in case of overflows
	normalizeComplexStats();	

	// server-side only (notify client owner)
	if(sv != NULL){ 
		nite::Packet notify;
		notify.setHeader(Game::PacketType::SV_UPDATE_ENTITY_ALL_STAT);
		notify.write(&id, sizeof(id));
		writeAllStatState(notify);
		sv->persSendAll(notify, 750, -1);
	}
		
}

void Game::EntityBase::loadAnim(){
	anim.load("data/anim/anim_humanoid.json");
	// server cannot load textures (headless doesnt' run opengl)
	if(sv == NULL){
		anim.anim.load(anim.source.path, anim.transparency);
	}
}

bool Game::EntityBase::damage(const Game::DamageInfo &dmg){
	if(this->healthStat.dead){
		return false;
	}
	Int32 def = 0, mdef = 0;
	auto item = invStat.slots[Game::EquipSlot::Chest];
	Game::ItemBase *armor = item.get() != NULL ? static_cast<Game::EquipItem*>(item.get()) : NULL; 
	auto efVal = Game::Element::isEffective(dmg.elmnt, armor == NULL ? Game::Element::Neutral : armor->elemnt); // entities are neutral by default
	Int32 dmgdone = dmg.amnt * efVal * (dmg.isCrit ? 2.25f : 1.0f); // damage cannot be negative at the end	   
	for(int i = 0; i < EquipSlot::TOTAL; ++i){
		auto item = invStat.slots[i];
		if(item.get() == NULL || i == EquipSlot::LeftAcc || i == EquipSlot::RightAcc) continue;
		auto equip = static_cast<Game::EquipItem*>(item.get());
		def += equip->def;
		mdef += equip->mdef;
		if(!dmg.truedmg){ // truedmg ignores damage buffs from weaps and items
			dmgdone = equip->onDamageRecv(dmgdone, dmg); 
		}
	}
	switch(dmg.dmgtype){
		case Game::DamageType::Magical: {
			dmgdone -= mdef;
		} break;
		case Game::DamageType::Physical: {
			dmgdone -= def;
		} break;
		case Game::DamageType::Ranged: {
			dmgdone = dmgdone * 0.90f - def; // ranged attacks are always 90% of the real value
		} break;				
	}
	auto &health = this->healthStat.health;
	auto orig = health;
	if(dmgdone > 0){
		if(dmgdone > health){
			health = 0;
		}else{
			health -= dmgdone;
		}
	}
	// server-side only
	if(orig != health && sv != NULL){ 
		nite::Packet notify;
		notify.setHeader(Game::PacketType::SV_UPDATE_ENTITY_HEALTH_STAT);
		notify.write(&id, sizeof(id));
		writeHealthStatState(notify);
		sv->persSendAll(notify, 750, -1);		
	}
	return dmgdone > 0;
}

void Game::EntityBase::writeAllStatState(nite::Packet &packet){
	packet.write(&healthStat, sizeof(healthStat));
	packet.write(&baseStat, sizeof(baseStat));
	packet.write(&complexStat, sizeof(complexStat));
}

void Game::EntityBase::readAllStatState(nite::Packet &packet){
	packet.read(&healthStat, sizeof(healthStat));
	packet.read(&baseStat, sizeof(baseStat));
	packet.read(&complexStat, sizeof(complexStat));	
}

void Game::EntityBase::writeHealthStatState(nite::Packet &packet){
	packet.write(&healthStat, sizeof(healthStat));
}

void Game::EntityBase::readHealthStatState(nite::Packet &packet){
	packet.read(&healthStat, sizeof(healthStat));
}

void Game::EntityBase::writeInitialState(nite::Packet &packet){
	writeAllStatState(packet);
}

void Game::EntityBase::readInitialState(nite::Packet &packet){
	readAllStatState(packet);
}