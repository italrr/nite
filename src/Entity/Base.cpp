#include "../Engine/Graphics.hpp"
#include "../Engine/Texture.hpp"

#include "../Core/Network.hpp"
#include "../Core/Server.hpp"

#include "Base.hpp"


void Game::EntityBase::entityMove(float angle, float mod, bool holdStance){ 
	if(!holdStance && (nite::toDegrees(angle) <= 45 || nite::toDegrees(angle) >= 315)){
		faceDirection = EntityFacing::Right;
	}
	if(!holdStance && (nite::toDegrees(angle) >= 135 && nite::toDegrees(angle) <= 225)){
		faceDirection = EntityFacing::Left;
	}
	// isWalking = true;
	// walkingTimeout = nite::getTicks();
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
	nite::print("Str "+str(baseStat.strStat)+"/"+ms+" | Agi "+str(baseStat.agiStat)+"/"+ms+
	" | Dex "+str(baseStat.dexStat)+"/"+ms+" | Endurance "+str(baseStat.enduStat)+"/"+ms+
	"| Luk "+str(baseStat.lukStat)+"/"+ms+" | Int "+str(baseStat.intStat)+"/"+ms+
	" | Char "+str(baseStat.charismaStat)+"/"+ms);
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
    static nite::Texture blank("data/sprite/empty.png");
    nite::setRenderTarget(nite::RenderTargetGame);
	nite::setColor(1.0f, 1.0f, 1.0f, 1.0f);
	nite::setDepth(nite::DepthMiddle);
    blank.draw(position.x, position.y, size.x, size.y, 0.5f, 0.5f, 0.0f);
}

void Game::EntityBase::updateStats(){

	auto effupd = effectStat.update();
	
	// recalculate local stats
	recalculateStats();

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

	// server-side only
	if(sv != NULL){ 
		nite::Packet notify;
		notify.setHeader(Game::PacketType::SV_UPDATE_ENTITY_ALL_STAT);
		notify.write(&id, sizeof(id));
		writeAllStatState(notify);
		sv->persSendAll(notify, 750, -1);
	}
		
}

bool Game::EntityBase::damage(const Game::DamageInfo &dmg){
	auto item = invStat.slots[Game::EquipSlot::Chest];
	Game::ItemBase *armor = item.get() != NULL ? static_cast<Game::EquipItem*>(item.get()) : NULL; 
	Int32 def = 0, mdef = 0;
	auto efVal = Game::Element::isEffective(dmg.elmnt, armor == NULL ? Game::Element::Neutral : armor->elemnt); // entities are neutral by default
	Int32 dmgdone = dmg.amnt * efVal * (dmg.isCrit ? 2.25f : 1.0f); // damage cannot be negative at the end	   
	for(int i = 0; i < EquipSlot::TOTAL; ++i){
		auto item = invStat.slots[i];
		if(item.get() == NULL || i == EquipSlot::LeftAcc || i == EquipSlot::RightAcc) continue;
		auto equip = static_cast<Game::EquipItem*>(item.get());
		def += equip->def;
		mdef += equip->mdef;
		dmgdone = equip->onDamageRecv(dmgdone, dmg);
	}
	switch(dmg.dmgtype){
		case Game::DamageType::Magical: {
			dmgdone -= mdef;
		} break;
		case Game::DamageType::Physical: {
			dmgdone -= def;
		} break;
		case Game::DamageType::Ranged: {
			dmgdone = dmgdone * 0.90f - def; 
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