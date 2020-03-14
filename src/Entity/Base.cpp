#include "../Engine/Graphics.hpp"
#include "../Engine/Texture.hpp"
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
	nite::print("Killed entity id "+nite::toStr(id)+" '"+name+"'");
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

void Game::EntityBase::writeInitialState(nite::Packet &packet){
	packet.write(&healthStat, sizeof(healthStat));
	packet.write(&baseStat, sizeof(baseStat));
	packet.write(&complexStat, sizeof(complexStat));
}

void Game::EntityBase::readInitialState(nite::Packet &packet){
	packet.read(&healthStat, sizeof(healthStat));
	packet.read(&baseStat, sizeof(baseStat));
	packet.read(&complexStat, sizeof(complexStat));	
}