#include "Entity.hpp"


Game::ComplexStat::ComplexStat() {
	maxCarry = 1000;
	walkRate = 0.25f;
	atk = 0;
	magicAtk = 0;
	def = 0;
	magicDef = 0; 
	critRate = 0;
	atkRate = 0;
	precsRate = 0;
	persuasionRate = 0;
	charmRate = 0;
	cooldownRedRate = 0.0f;
}

Game::BaseStat::BaseStat(){
	str = 0;
	agi = 0;
	dex = 0;
	endu = 0;
	luk = 0;
	intel = 0;
	charm = 0;
	resetAdd();	
}

void Game::BaseStat::resetAdd(){
	strAdd = 0;
	agiAdd = 0;
	dexAdd = 0;
	enduAdd = 0;
	lukAdd = 0;
	intelAdd = 0;
	charmAdd = 0;
}

void Game::EntityStat::resetComplexStats(){
	complexStat.maxCarry = 0.0f;
	complexStat.atk = 0.0f;
	complexStat.magicAtk = 0.0f;
	complexStat.def = 0.0f;
	complexStat.magicDef = 0.0f;
	complexStat.walkRate = 0.0f;
	complexStat.critRate = 0.0f;
	complexStat.precsRate = 0.0f;
	complexStat.atkRate = 0.0f;
	complexStat.persuasionRate = 0.0f;
	complexStat.charmRate = 0.0f;
	complexStat.cooldownRedRate = 0.0f;
	complexStat.luckRate = 0.0f;
	complexStat.fleeRate = 0.0f;
}

void Game::EntityStat::recalculateBaseStats(){
	baseStat.strAdd += baseStat.str;
	baseStat.agiAdd += baseStat.agi;
	baseStat.dexAdd += baseStat.dex;
	baseStat.enduAdd += baseStat.endu;
	baseStat.lukAdd += baseStat.luk;
	baseStat.intelAdd += baseStat.intel;
	baseStat.charmAdd += baseStat.charm;
}

void Game::EntityStat::recalculateHealthStats(){
	if(healthStat.dead){
		return;
	}	
	healthStat.maxHealth = 100 + nite::ceil((25.5f + baseStat.enduAdd * 1.5f)  * GAME_STAT_BASE_SCALE * healthStat.lv);
	healthStat.maxMana = nite::ceil((4.5f + baseStat.intelAdd * 0.5f) * GAME_STAT_BASE_SCALE * healthStat.lv);
	healthStat.maxStamina = nite::ceil((1.9f + baseStat.dexAdd * 0.35f + baseStat.enduAdd * 0.15f) * GAME_STAT_BASE_SCALE * healthStat.lv);
}

void Game::EntityStat::recalculateComplexStats(){
	if(healthStat.dead){
		return;
	}	
	complexStat.maxCarry += 1000 + nite::ceil((baseStat.strAdd * 150.0f + baseStat.enduAdd * 65.0f) * GAME_STAT_BASE_SCALE);
	complexStat.atk += 1.0f + nite::ceil((baseStat.strAdd * 5.2f + baseStat.enduAdd * 1.2f + healthStat.lv * 2.0f) * GAME_STAT_BASE_SCALE);
	complexStat.def += 1.0f + nite::ceil((baseStat.enduAdd * 5.8f + healthStat.lv * 1.5f) * GAME_STAT_BASE_SCALE);
	
	complexStat.magicAtk += 1.0f + nite::ceil((baseStat.intelAdd * 3.5f) * GAME_STAT_BASE_SCALE);	
	complexStat.magicDef += 1.0f + nite::ceil((baseStat.intelAdd * 3.5f + healthStat.lv * 2.5f) * GAME_STAT_BASE_SCALE);		

	complexStat.critRate += 0.0f + nite::ceil((0.25f * baseStat.lukAdd) * GAME_STAT_BASE_SCALE);
	complexStat.luckRate += 0.0f + nite::ceil((0.25f * baseStat.lukAdd) * GAME_STAT_BASE_SCALE); // Improve this


	complexStat.walkRate += nite::ceil((0.06f * baseStat.agiAdd) * GAME_STAT_BASE_SCALE); // TODO: take weight into account
	
	complexStat.precsRate += 1.0f + nite::ceil((2.5f * baseStat.dexAdd) * GAME_STAT_BASE_SCALE);
	complexStat.atkRate += (baseStat.agiAdd * (3.85f + healthStat.lv * 0.15f) + baseStat.dexAdd * (1.1f + healthStat.lv * 0.1f)) * GAME_STAT_BASE_SCALE;
	complexStat.persuasionRate += nite::ceil((0.45f * healthStat.lv + baseStat.charmAdd * 5.96f + baseStat.intelAdd * 2.25f) * GAME_STAT_BASE_SCALE);
	complexStat.charmRate += nite::ceil((0.5f * healthStat.lv + baseStat.charmAdd * 5.96f) * GAME_STAT_BASE_SCALE);
	
	
	complexStat.fleeRate += 1.0f + nite::ceil((2.5f * baseStat.agiAdd) * GAME_STAT_BASE_SCALE);
	complexStat.cooldownRedRate += 0.0f;
}

void Game::EntityStat::normalizeComplexStats(){
	if(complexStat.cooldownRedRate > 1.0f){
		complexStat.cooldownRedRate = 1.0f;
	}
}

Int32 Game::EntityStat::addBaseStat(int type, int amnt){
	if(healthStat.dead){
		return 0;
	}
	Int32 toAdd = 0;
	auto inc = [&](int &target){
		int toRest = baseStat.statPoints;
		// handle remaining points
		if(amnt <= toRest){
			toAdd +=  amnt;
			toRest -= amnt;
		}else{
			toAdd = toRest;
			toRest = 0;
		}
		// handle excess points
		if(toAdd + target > GAME_MAX_STAT){
			toAdd = GAME_MAX_STAT - target;
			toRest = baseStat.statPoints - toAdd;
		}
		baseStat.statPoints = toRest;
		target += toAdd;
	};
	switch(type){
		case BaseStatType::Strength:{
			inc(baseStat.str);
		} break;
		case BaseStatType::Agility:{
			inc(baseStat.agi);
		} break;
		case BaseStatType::Dexterity:{
			inc(baseStat.dex);
		} break;
		case BaseStatType::Endurance:{
			inc(baseStat.endu);
		} break;
		case BaseStatType::Luck:{
			inc(baseStat.luk);
		} break;
		case BaseStatType::Intelligence:{
			inc(baseStat.intel);
		} break;
		case BaseStatType::Charisma:{
			inc(baseStat.charm);
		} break;
	}
	recalculateStats();
	return toAdd;
}


void Game::EntityStat::recalculateStats(){
	baseStat.resetAdd();
	recalculateBaseStats();
	recalculateHealthStats();
	resetComplexStats();
	recalculateComplexStats();	
}

void Game::EntityStat::resetBaseStat(int type){
	if(healthStat.dead){
		return;
	}
	baseStat.statPoints = GAME_STAT_POINTS_PER_LV * this->healthStat.lv;
	switch(type){
		case BaseStatType::Strength:{
			baseStat.str = 0;
		} break;
		case BaseStatType::Agility:{
			baseStat.agi = 0;
		} break;
		case BaseStatType::Dexterity:{
			baseStat.dex = 0;
		} break;
		case BaseStatType::Endurance:{
			baseStat.endu = 0;
		} break;
		case BaseStatType::Luck:{
			baseStat.luk = 0;
		} break;
		case BaseStatType::Intelligence:{
			baseStat.intel = 0;
		} break;
		case BaseStatType::Charisma:{
			baseStat.charm = 0;
		} break;
	}
	recalculateStats();	
}

void Game::EntityStat::fullHeal(){
	healthStat.health = healthStat.maxHealth;
	healthStat.mana = healthStat.maxMana;
	healthStat.stamina = healthStat.maxStamina;
}

bool Game::EntityStat::lvUp(){
	if(healthStat.dead){
		return false;
	}	
	if(healthStat.lv >= GAME_MAX_LEVEL){
		return false;
	}
	++healthStat.lv;
	baseStat.statPoints += GAME_STAT_POINTS_PER_LV;
	healthStat.exp = 0;
	healthStat.nextExp = nite::ceil((healthStat.lv + 1) * GAME_STAT_BASE_SCALE * 250);
	recalculateStats();
	return true;
}

bool Game::EntityStat::lvUp(int lvs){
	if(healthStat.dead){
		return false;
	}	
	for(int i = 0; i < lvs; ++i){
		if(healthStat.lv >= GAME_MAX_LEVEL){
			break;
		}		
		++healthStat.lv;
		baseStat.statPoints += GAME_STAT_POINTS_PER_LV;
		healthStat.exp = 0;
		healthStat.nextExp = nite::ceil((healthStat.lv + 1) * GAME_STAT_BASE_SCALE * 250);
	}
	recalculateStats();
	return true;
}

void Game::EntityStat::setupStat(int lv){
	healthStat.lv = 0;
	healthStat.exp = 0;
	healthStat.dead = false;
	baseStat.statPoints = 12;
	while(healthStat.lv < lv && lvUp());
	
	fullHeal();
}

bool Game::EntityStat::heal(int hp, int mana, int stamina){
	if(healthStat.dead){
		return false; 
	}
		
	healthStat.health += hp;
	if(healthStat.health > healthStat.maxHealth){
		healthStat.health = healthStat.maxHealth;
	}

	healthStat.mana += mana;
	if(healthStat.mana > healthStat.maxMana){
		healthStat.mana = healthStat.maxMana;
	}

	healthStat.stamina += stamina;
	if(healthStat.stamina > healthStat.maxStamina){
		healthStat.stamina = healthStat.maxStamina;
	}		

	recalculateStats();
	return true;
}


Game::Entity::Entity(){
	walkSpeed = 1.0f;	
	nickname = "Entity";
	entityType = EntityType::UNDEFINED;
}

void Game::Entity::loadAnim(){
	if(!battleAnim.isLoaded()){
		battleAnim.load("data/texture/kekkers.png", nite::Color(1.0f, 1.0f, 1.0f, 1.0f));
	}
}

void Game::Entity::moveEntity(float x, float y){
	move(walkSpeed * x, walkSpeed * y);
}

void Game::Entity::printInfo(){
	String p = this->position.str();
	#define str nite::toStr
	String ms = str(GAME_MAX_STAT);
	nite::print("ID "+str(id)+" | Name: "+nickname+" | LV "+str(healthStat.lv)+" | Carry "+str(complexStat.maxCarry)+" | Position "+p);
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
	nite::print("LuckRate "+str(complexStat.luckRate));
	#undef str
}

bool Game::Entity::damage(Game::DamageInfo &info){
	if(info.target->healthStat.dead || info.owner->healthStat.dead){
		info.dmg = 0;
		return false;
	}

	bool targetLucky = nite::randomInt(11, 100 + nite::round(info.target->complexStat.critRate)) > 99.0f;
	bool ownerCritChance = nite::randomInt(11, 100 + nite::round(info.owner->complexStat.critRate)) > 99.0f;

	info.isCrit = ownerCritChance;
	
	float dmg = 1.0f + (float)info.owner->complexStat.atk * (info.isCrit ? 1.5f : 1.0f); // TODO: take weapon into account

	if(info.tryingBlock){
		int chance = nite::randomInt(targetLucky ? 20 : 10, targetLucky ? 35 : 25);
		float rate = (float)(100 - chance) / 100.0f;
		int ndmg = dmg * rate;
		info.blockDmg = dmg - ndmg;
		dmg = ndmg;
	}

	float difFleefRate = info.target->complexStat.fleeRate / info.owner->complexStat.precsRate;
	float missChanceTop = nite::round(100.0f * difFleefRate * (targetLucky ? 1.25f : 0.0f));
	int missChanceRoll = nite::randomInt(1, missChanceTop + (info.tryingDodge ? 10 : 0));
	bool missChance =  missChanceTop > 1 && missChanceRoll > 95;
	
	if(missChance){
		info.dodged = true;
		info.byLuck = targetLucky;
		info.dmg = 0;
		return false;
	}

	float tdef = info.target->complexStat.def; // TODO: take equipment into account, and element

	if(tdef > dmg){
		dmg = 0;
	}else{
		dmg -= tdef;
	}

	if(dmg >= info.target->healthStat.health){
		info.target->healthStat.dead = true;
		info.target->healthStat.health = 0;
	}else{
		info.target->healthStat.health -= dmg;
	}

	info.dmg = dmg;
	return true;
}